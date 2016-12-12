#ifndef ERROR_REPORT_H_INCLUDED
#define ERROR_REPORT_H_INCLUDED

#include <syslog.h>

#ifdef DEBUG
void _syslog(uint32_t p, const char* format, ...);
#define syslog _syslog
#else
#endif

void log_init(void);
void log_cleanup(void);

#endif
