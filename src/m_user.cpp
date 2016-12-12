#include "dd3d.h"
#include "log.h"

#include <time.h>

#include "m_user.h"
#include "utils.h"

const char* TABLE_USER 		= "vip_user";
const char* TABLE_SESSION 	= "vip_session";

static mysqlpp::Connection* Conn;

void m_user_init(void) {
	Conn = database_conn();
}

void m_user_cleanup(void) {
	Conn = NULL;
}

bool m_user_get_login_info(LoginUserInfo* info, const char* email, const char* password) {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		mysqlpp::Query query(Conn);
		query << "SELECT `uid`, `expire_time`, `groupid`, `banned` FROM `" << TABLE_USER << "` WHERE (`email` = " << 
			mysqlpp::quote << email << " OR `username` = " << mysqlpp::quote << email << ") AND (`password` = " << 
			mysqlpp::quote << password << " OR `password` = MD5(" << mysqlpp::quote << password << ")) LIMIT 1";
		
		mysqlpp::StoreQueryResult res = query.store();
		if (res.num_rows()) {
			info->uid 		= res[0][0];
			info->expire_time 	= res[0][1];
			info->groupid 		= res[0][2];
			info->banned 		= res[0][3];
			rv = true;
		}
	)
	return rv;
}

bool m_user_get_session_by_id(SessionInfo* info,  const char session_id[33]) {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		mysqlpp::Query query(Conn);
		query << "SELECT `client_id`, `uid`, `alive_time`, `key`, `locale_id` FROM `" << TABLE_SESSION << "`"
			" WHERE `session_id` = " << mysqlpp::quote << session_id << " LIMIT 1";
		
		mysqlpp::StoreQueryResult res = query.store();
		if (res.num_rows()) {
			info->session_id	= session_id;
			info->client_id 	= res[0][0];
			info->uid 			= res[0][1];
			info->alive_time 	= res[0][2];
			info->key			= res[0][3];
			info->locale_id 	= res[0][4];
			rv = true;
		}
	)
	return rv;
}

bool m_user_get_session_key_by_id(const char session_id[33], uint8_t key[XXTEA_KEY_SIZE], bool check_logged_in) {
	bool rv = false;
	SessionInfo info;
	if (m_user_get_session_by_id(&info, session_id)) {
		if (!check_logged_in || (check_logged_in && info.uid > 0)) {
			memcpy(key, info.key.data(), XXTEA_KEY_SIZE);
			rv = true;
		}
	}
	return rv;
}

bool m_user_erase_session_by_id(const char session_id[33]) {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		mysqlpp::Query query(Conn);
		query << "DELETE FROM `" << TABLE_SESSION << "` WHERE `session_id` = "  << mysqlpp::quote << session_id; 
		if (query.exec())
			rv = true;
	)
	return rv;
}

bool m_user_erase_session_by_uid(uint32_t uid) {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		mysqlpp::Query query(Conn);
		query << "DELETE FROM `" << TABLE_SESSION << "` WHERE `uid` = " << uid;
		if (query.exec())
			rv = true;
	)
	return rv;
}

bool m_user_get_session_by_uid(SessionInfo* info, uint32_t uid) {
	bool rv = false;
	try {
		mysqlpp::Query query(Conn);
		query << "SELECT `session_id`, `client_id`, `alive_time`, `key`, `locale_id` FROM `" << TABLE_SESSION << "`"
			" WHERE `uid` = " << uid << " LIMIT 1";
		
		mysqlpp::StoreQueryResult res = query.store();
		if (res.num_rows()) {
			info->session_id	= res[0][0].c_str();
			info->client_id 	= res[0][1];
			info->uid 			= uid;
			info->alive_time 	= res[0][2]; 
			info->key		 	= res[0][3]; 
			info->locale_id 	= res[0][4];
			rv = true;
		}
	} catch (const mysqlpp::Exception& e) {
		printf("exception: %s\n", e.what());
	}
	return rv;
}

bool m_user_clear_timeout_sessions() {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		uint32_t now = time(NULL);
		mysqlpp::Query query(Conn);
		query << "DELETE FROM `" << TABLE_SESSION << "` WHERE `alive_time` < " << (now - SESSION_TIMEOUT);
		if (query.exec())
			rv = true;
	)
	return rv;
}

bool m_user_create_session(char session_id[33], const uint8_t key[16]) {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		uint32_t now = time(NULL);
		mysqlpp::Query query(Conn);
		uint8_t uuid[16];
		utils_get_uuid(uuid);
		utils_bin2hex(uuid, 16, session_id, 33);
		std::string key_str;
		key_str.assign((char*)key, 16);
		query << "INSERT INTO `" << TABLE_SESSION << "` VALUES(" 
			<< mysqlpp::quote << session_id << "," << 0 << "," << 0 << "," << now << ",\"" << mysqlpp::escape << key_str << "\"" << "," << 0 << ")";
		if (query.exec() && query.affected_rows() == 1)
			rv = true;
	)
	return rv;
}

bool m_user_update_session_by_id(const char session_id[33], uint32_t client_id, uint32_t uid) {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		uint32_t now = time(NULL);
		mysqlpp::Query query(Conn);
		query << "UPDATE `" << TABLE_SESSION << "` SET `client_id` = " << client_id
			<< ", uid = " << uid << ", alive_time = " << now << " WHERE `session_id` = " << mysqlpp::quote << session_id;
		if (query.exec() && query.affected_rows() == 1)
			rv = true;
	)
	return rv;
}

bool m_user_update_session_client_id_by_uid(uint32_t uid, uint32_t client_id) {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		uint32_t now = time(NULL);
		mysqlpp::Query query(Conn);
		query << "UPDATE `" << TABLE_SESSION << "` SET `client_id` = " << client_id  
			<< ", alive_time = " << now << " WHERE `uid` = " 
			<< uid << " LIMIT 1";
		if (query.exec() && query.affected_rows() == 1)
			rv = true;
	)
	return rv;
}

bool m_user_alive_session_by_id(const char session_id[33]) {
	bool rv = false;
	uint32_t now = time(NULL);
	CATCH_DB_EXCEPTION (
		mysqlpp::Query query(Conn);
		query << "UPDATE `" << TABLE_SESSION << "` SET `alive_time` = " << now << " WHERE `session_id` = " 
			<< mysqlpp::quote << session_id << " LIMIT 1";
		if (query.exec() && query.affected_rows() == 1)
			rv = true;
	)
	return rv;
}

bool m_user_alive_session_by_uid(uint32_t uid) {
	bool rv = false;
	uint32_t now = time(NULL);
	CATCH_DB_EXCEPTION (
		mysqlpp::Query query(Conn);
		query << "UPDATE `" << TABLE_SESSION << "` SET `alive_time` = " << now << " WHERE `uid` = " 
			<< uid << " LIMIT 1";
		if (query.exec() && query.affected_rows() == 1)
			rv = true;
	)
	return rv;
}

bool m_user_update_locale_by_id(const char session_id[33], uint32_t locale_id) {
	bool rv = false;
	CATCH_DB_EXCEPTION (
		mysqlpp::Query query(Conn);
		query << "UPDATE `" << TABLE_SESSION << "` SET `locale_id` = " << locale_id << " WHERE `session_id` = " 
			<< mysqlpp::quote << session_id << " LIMIT 1";
		if (query.exec() && query.affected_rows() == 1)
			rv = true;
	)
	return rv;
}
