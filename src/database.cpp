#include "dd3d.h"
#include "config.h"

#include "database.h"

mysqlpp::Connection* Conn = NULL;

char *Server = NULL, *Db = NULL, *User = NULL, *Password = NULL;
void database_init(void) {
	const char *server, *db, *user, *password;
	if (CONFIG_TRUE == config_lookup_string(config_get(), "dd3d.mysql.server", &server) &&
		CONFIG_TRUE == config_lookup_string(config_get(), "dd3d.mysql.db", &db) &&
		CONFIG_TRUE == config_lookup_string(config_get(), "dd3d.mysql.user", &user) &&
		CONFIG_TRUE == config_lookup_string(config_get(), "dd3d.mysql.password", &password))
	{
		//save config
		int len = strlen(server);;
		Server = new char[len + 1];
		strncpy(Server, server, len + 1);

		len = strlen(db);
		Db = new char[len + 1];
		strncpy(Db, db, len + 1);

		len = strlen(user);
		User = new char[len + 1];
		strncpy(User, user, len + 1);

		len = strlen(password);
		Password = new char[len + 1];
		strncpy(Password, password, len + 1);

		try {
			Conn = new mysqlpp::Connection();
			Conn->connect(Db, Server, User, Password);
		} catch (const mysqlpp::Exception& e) {
			throw e.what();
		}

		CATCH_DB_EXCEPTION (
			mysqlpp::Query query(Conn);
			query << "SET NAMES 'utf8'";
			if (!query.exec())
				throw "Init database encoding failed.";
		)
	} else
		throw "Mysql config missing or incomplete.\n";
}

void database_cleanup(void) {
	delete Conn;
	delete [] Server;
	delete [] Db;
	delete [] User;
	delete [] Password;
}

mysqlpp::Connection* database_conn(void) {
	return Conn;
}

void database_ping(void) {
	bool connected = Conn->ping();
	while (!Terminate && !connected) {
		syslog(LOG_WARNING, "[WARNING]Database connection lost. Try to reconnect...\n");
		try {
			connected = Conn->connect(Db, Server, User, Password);				
		} catch (const mysqlpp::Exception& e) {
			syslog(LOG_ERR, "[ERROR]Reconnect database failed: %s\n", e.what());
		}
		if (!connected)
			sleep(5);
		else
			syslog(LOG_INFO, "[INFO]Database connection restored.\n");
	}
}
