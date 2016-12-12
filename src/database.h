#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED

#include <mysql++/mysql++.h>
#include "log.h"

#define CATCH_DB_EXCEPTION(code) \
	try { \
		code \
	} catch (const mysqlpp::Exception& e) { \
		syslog(LOG_ERR, "[ERROR]%s\n", e.what()); \
	}

void database_init(void);
void database_cleanup(void);
mysqlpp::Connection* database_conn(void);
void database_ping(void);

#endif
