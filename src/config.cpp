#include "dd3d.h"

#include "config.h"

#ifdef DEBUG
const char* CONFIG_PATH = "dd3d.conf";
#elif defined BETA
const char* CONFIG_PATH = "/etc/dd3d_beta/dd3d.conf";
#else
const char* CONFIG_PATH = "/etc/dd3d/dd3d.conf";
#endif

config_t Config;

void config_init() {
	config_init(&Config);
	if (CONFIG_FALSE == config_read_file(&Config, CONFIG_PATH)) {
		snprintf(Buffer, BUFSIZ, "Initialize config failed: %s (%s:%d)\n", 
			config_error_text(&Config),
			config_error_file(&Config),
			config_error_line(&Config)
		);
		throw(Buffer);
	}
}

void config_cleanup() {
	config_destroy(&Config);
}

config_t* config_get() {
	return &Config;
}
