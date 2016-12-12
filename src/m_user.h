#ifndef M_USER_H_INCLUDED
#define M_USER_H_INCLUDED

#include "database.h"
#include "utils.h"

const uint32_t SESSION_TIMEOUT = 300;

enum UserGroupEnumType {
	USER_GROUP_ADMIN = 9,
	USER_GROUP_UNLIMITED = 3
};

struct LoginUserInfo {
	mysqlpp::sql_mediumint_unsigned  	uid;
	mysqlpp::sql_int_unsigned 			expire_time;
	mysqlpp::sql_smallint 				groupid;
	mysqlpp::sql_int_unsigned  			banned;
};

struct SessionInfo {
	mysqlpp::sql_char					session_id;
	mysqlpp::sql_int_unsigned			client_id;
	mysqlpp::sql_int_unsigned			uid;
	mysqlpp::sql_int_unsigned 			alive_time;
	mysqlpp::sql_blob					key;
	mysqlpp::sql_int_unsigned			locale_id;
};

void m_user_init(void);
void m_user_cleanup(void);

bool m_user_get_login_info(LoginUserInfo* info, const char* email, const char* password);
bool m_user_get_session_key_by_id(const char session_id[33], uint8_t key[XXTEA_KEY_SIZE], bool check_logged_in = false);
bool m_user_get_session_by_id(SessionInfo* info,  const char session_id[33]); 
bool m_user_get_session_by_uid(SessionInfo* info, uint32_t uid);
bool m_user_create_session(char session_id[33], const uint8_t key[16]);
bool m_user_alive_session_by_uid(uint32_t uid);
bool m_user_alive_session_by_id(const char session_id[33]);
bool m_user_erase_session_by_id(const char session_id[33]);
bool m_user_erase_session_by_uid(uint32_t uid);
bool m_user_update_session_client_id_by_uid(uint32_t uid,  uint32_t client_id);
bool m_user_update_session_by_id(const char session_id[33], uint32_t client_id, uint32_t uid);
bool m_user_clear_timeout_sessions();
bool m_user_update_locale_by_id(const char session_id[33], uint32_t locale_id);

#endif
