#include "dd3d.h"
#include <stdarg.h>

#include "log.h"

#ifdef DEBUG
void syslog(uint32_t p, const char* format, ...) {
	char buffer[BUFSIZ];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, BUFSIZ - 1, format, args);
	va_end(args);
	printf(buffer);
}
#endif

void log_init(void) {
#ifndef DEBUG
	openlog("dd3d", LOG_CONS | LOG_PID, LOG_USER);
#endif
}

void log_cleanup(void) {
#ifndef DEBUG
	closelog();
#endif
}
