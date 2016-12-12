#include "dd3d.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>

#include "config.h"
#include "connection.h"
#include "database.h"
#include "crypt.h"
#include "log.h"

#include "m_user.h"
#include "m_offsets.h"
#include "m_service.h"

#include "c_handshake.h"
#include "c_login.h"
#include "c_offsets.h"
#include "c_alive.h"
#include "c_update.h"
#include "c_logout.h"

#include "message.h"
#include "utils.h"

void dd3d_init(void);
void dd3d_serve(void);
void dd3d_cleanup(void);

int ConfigTestMode = 0;
int ConfigMinVersion = 0;
int ConfigEnableBetaLogin = 0;
char Buffer[BUFSIZ];
uint8_t MsgBuffer[1024000];
volatile bool Terminate = false;

void signal_handler(int sig) {
	switch(sig) {
		case SIGHUP:
			syslog(LOG_WARNING, "[INFO]Received SIGHUP signal.\n");
			break;
		case SIGINT:
		case SIGTERM:
			syslog(LOG_WARNING, "[INFO]Received SIGTERM signal.\n");
			Terminate = true;
			break;
		default:
			syslog(LOG_WARNING, "[INFO]Unhandled signal (0x%X) %s\n", sig, strsignal(sig));
			break;
	}
}

int main(int argc, char* argv[]) {

	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

#ifndef DEBUG
	pid_t pid, sid;

	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	umask(0);

	log_init();

	sid = setsid();
	if (sid < 0) {
		syslog(LOG_ERR, "[ERROR]Failed to setsid.\n");
		exit(EXIT_FAILURE);
	}

	if (chdir("/") < 0) {
		syslog(LOG_ERR, "[ERROR]Failed to chdir.\n");
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
#endif

	try {
		dd3d_init();
	} catch(const char* exception) {
		syslog(LOG_ERR, "[ERROR]%s", exception);
		exit(EXIT_FAILURE);
	}

	dd3d_serve();
	dd3d_cleanup();

#ifndef DEBUG
	log_cleanup();
#endif
	return EXIT_SUCCESS;
}

void dd3d_init(void) {
	config_init();
	config_lookup_bool(config_get(), "dd3d.test_mode", &ConfigTestMode);
	config_lookup_int(config_get(), "dd3d.min_version", &ConfigMinVersion);
	config_lookup_bool(config_get(), "dd3d.enable_beta_login", &ConfigEnableBetaLogin);

	//Foundation modules
	crypt_init();
	database_init();
	connection_init();

	//Models
	m_user_init();
	m_offsets_init();
	m_service_init();
}

void dd3d_serve(void) {
	connection_register_handler(MESSAGE_TYPE_HANDSHAKE, c_handshake_handler);
	connection_register_handler(MESSAGE_TYPE_LOGIN, c_login_handler);
	connection_register_handler(MESSAGE_TYPE_GET_OFFSETS, c_offsets_handler);
	connection_register_handler(MESSAGE_TYPE_ALIVE, c_alive_handler);
	connection_register_handler(MESSAGE_TYPE_UPDATE, c_update_handler);
	connection_register_handler(MESSAGE_TYPE_LOGOUT, c_logout_handler);

	syslog(LOG_INFO, "[INIT]Server started.\n");
	connection_process();
	syslog(LOG_INFO, "[INIT]Server terminated.\n");
}

void dd3d_cleanup(void) {
	//Models
	m_service_cleanup();
	m_offsets_cleanup();
	m_user_cleanup();

	//Foundation modules
	connection_cleanup();
	database_cleanup();
	crypt_cleanup();
	config_cleanup();
}
