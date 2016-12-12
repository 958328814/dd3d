#include "dd3d.h"
#include <sys/types.h>
#include <dirent.h>
#include "syslog.h"
#include "config.h"
#include "utils.h"

#include "m_service.h"

const char* TABLE_RELEASE = "release";
const char* TABLE_RELEASE_DESC = "release_desc";
static ReleaseInfo LastestReleaseInfo; 

static mysqlpp::Connection* Conn;

void m_service_init(void) {
	LastestReleaseInfo.build = 0;
	Conn = database_conn();
	CATCH_DB_EXCEPTION (
		mysqlpp::Query query(Conn);
		query << "select `build`, `link`, `date`, `version_string` from `" << TABLE_RELEASE << "` order by build desc limit 1";
		
		mysqlpp::StoreQueryResult res = query.store();
		if (res.num_rows()) { 
			LastestReleaseInfo.build 			= res[0][0];
			LastestReleaseInfo.link 			= res[0][1].c_str();
			LastestReleaseInfo.date 			= res[0][2];
			LastestReleaseInfo.version_string 	= res[0][3].c_str();
		}
	)
	
	if (LastestReleaseInfo.build > 0) {
		mysqlpp::Query query(Conn);
		query << "select `locale_id`, `desc` from `" << TABLE_RELEASE_DESC << "` WHERE `build` = " << LastestReleaseInfo.build;
		
		mysqlpp::StoreQueryResult res = query.store();
		if (res.num_rows()) { 
			for (uint32_t i = 0; i < res.num_rows(); ++i) {
				mysqlpp::sql_int_unsigned locale_id = mysqlpp::sql_int_unsigned(res[i][0]);
				mysqlpp::sql_text& desc_item = LastestReleaseInfo.desc_map[locale_id];
				desc_item.assign(res[i][1]);
			}
		}
	}
	
}

void m_service_cleanup(void) {
	Conn = NULL;
}

uint32_t m_service_get_lastest_release_build() {
	return LastestReleaseInfo.build;
}

const ReleaseInfo* m_service_get_lastest_release_info() {
	return &LastestReleaseInfo;
}

