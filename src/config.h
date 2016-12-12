#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <libconfig.h>

void config_init();
void config_cleanup();
config_t* config_get();

#endif
