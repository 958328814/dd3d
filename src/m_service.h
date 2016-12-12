#ifndef M_SERVICE_H_INCLUDED
#define M_SERVICE_H_INCLUDED

#include "database.h"
#include "utils.h"

const uint32_t DEFAULT_LOCALE_ID = 1033;
typedef std::map<mysqlpp::sql_int_unsigned, mysqlpp::sql_text> release_desc_map_t;

struct ReleaseInfo {
	mysqlpp::sql_int_unsigned			build;
	mysqlpp::sql_varchar				link;
	mysqlpp::sql_int_unsigned 			date;
	mysqlpp::sql_varchar				version_string;
	release_desc_map_t 					desc_map;
};

void m_service_init(void);
void m_service_cleanup(void);
const ReleaseInfo* m_service_get_lastest_release_info();
uint32_t m_service_get_lastest_release_build();

#endif
