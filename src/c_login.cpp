#include "dd3d.h"
#include "utils.h"
#include "message.h"
#include "connection.h"
#include "crypt.h"
#include "m_user.h"
#include "m_service.h"
#include "config.h"

#include "c_login.h"

enum LoginResultEnumType {
	LOGIN_RESULT_OK,
	LOGIN_RESULT_FAILED,
	LOGIN_RESULT_AUTH_FAILED,
	LOGIN_RESULT_SESSION_ERROR,
	LOGIN_RESULT_EXPIRED,
	LOGIN_RESULT_BANNED,
	LOGIN_RESULT_MULTI,
	LOGIN_RESULT_CLIENT_VERSION_EXPIRED
};

LoginResultEnumType c_login_update_session(const char session_id[33], uint32_t uid, uint32_t client_id) {
	LoginResultEnumType rv = LOGIN_RESULT_OK;
	SessionInfo session_info;

	if (m_user_clear_timeout_sessions()) {
		if (m_user_get_session_by_uid(&session_info, uid)) {
			if (session_info.client_id == client_id) {
				m_user_erase_session_by_uid(uid);
			} else {
				rv = LOGIN_RESULT_MULTI;
			}
		}

		if (rv == LOGIN_RESULT_OK && m_user_get_session_by_id(&session_info, session_id)) {
			if (!m_user_update_session_by_id(session_id, client_id, uid))
				rv = LOGIN_RESULT_SESSION_ERROR;
		}
	} else
		rv = LOGIN_RESULT_SESSION_ERROR;
	
	return rv;
}

LoginResultEnumType c_login_login(LoginUserInfo* login_info, const char session_id[33], const char* email, const char* password, uint32_t client_id, uint32_t locale_id) {
	uint32_t now = static_cast<uint32_t>(time(NULL));
	LoginResultEnumType rv = LOGIN_RESULT_FAILED;
	if (m_user_get_login_info(login_info, email, password)) {
		//Test Mode: No expire date check
		if (ConfigTestMode == 1) {
			login_info->expire_time = 0;
			login_info->groupid = USER_GROUP_UNLIMITED;
		}

		switch (login_info->groupid) {
			case USER_GROUP_ADMIN:
				rv = LOGIN_RESULT_OK;
				break;
			case USER_GROUP_UNLIMITED:
				if (login_info->banned == 0)
					rv = LOGIN_RESULT_OK;
				else 
					rv = LOGIN_RESULT_BANNED;
				break;
			default:
				if (login_info->expire_time < now)
					rv = LOGIN_RESULT_EXPIRED;
				else
					rv = LOGIN_RESULT_OK;
				break;
		}
		if (rv == LOGIN_RESULT_OK)
			rv = c_login_update_session(session_id, login_info->uid, client_id);
	} else
		rv = LOGIN_RESULT_AUTH_FAILED;
	return rv;
}

//BETA login
bool c_login_beta_login(const char session_id[33], MessageFieldsData* data) {
	bool rv = false;
	data->fields_data[0][data->fields_size[0] - 1] = '\0';
	char* account = (char*)data->fields_data[0];
	data->fields_data[1][data->fields_size[1] - 1] = '\0';
	char* password = (char*)data->fields_data[1];
	uint32_t client_id = data->fields_size[2] == 4 ? *(uint32_t*)(data->fields_data[2]) : 0;
	uint32_t version_id = data->fields_size[3] == 4 ? *(uint32_t*)(data->fields_data[3]) : 0;
	uint8_t result;
	if (account && password && client_id && version_id) {
		uint32_t version_req = ConfigMinVersion ? ConfigMinVersion : m_service_get_lastest_release_build();
		if (version_req == 0 || version_id >= version_req) {
			LoginUserInfo info;
			result = c_login_login(&info, session_id, account, password, client_id, 0); 
			if (result == LOGIN_RESULT_OK) {
				uint32_t now = (uint32_t)time(NULL);
				uint32_t remain_time = (info.groupid == USER_GROUP_ADMIN || info.groupid == USER_GROUP_UNLIMITED) ? 0 : info.expire_time - now;
				connection_reply(&remain_time, 4);
			} else {
				connection_reply(&result, 1);
			}
			rv = true;
		} else {
			result = LOGIN_RESULT_CLIENT_VERSION_EXPIRED;
			connection_reply(&result, 1);
			rv = true;
		}
	}
	return rv;
}

//RELEASE login
bool c_login_release_login(const char session_id[33], MessageFieldsData* data) {
	bool rv = false;
	data->fields_data[0][data->fields_size[0] - 1] = '\0';
	char* account = (char*)data->fields_data[0];
	data->fields_data[1][data->fields_size[1] - 1] = '\0';
	char* password = (char*)data->fields_data[1];
	uint32_t client_id = data->fields_size[2] == 4 ? *(uint32_t*)(data->fields_data[2]) : 0;
	uint32_t version_id = data->fields_size[3] == 4 ? *(uint32_t*)(data->fields_data[3]) : 0;
	uint32_t locale_id = data->fields_size[4] == 4 ? *(uint32_t*)(data->fields_data[4]) : 0;
	uint8_t result;
	if (account && password && client_id && version_id) {
		if (!m_user_update_locale_by_id(session_id, locale_id))
			syslog(LOG_ERR, "[ERROR]Update user locale id failed.\n");
		uint32_t version_req = ConfigMinVersion ? ConfigMinVersion : m_service_get_lastest_release_build();
		if (version_req == 0 || version_id >= version_req) {
			LoginUserInfo info;
			result = c_login_login(&info, session_id, account, password, client_id, locale_id); 
			if (result == LOGIN_RESULT_OK) {
				uint32_t now = (uint32_t)time(NULL);
				uint32_t remain_time = (info.groupid == USER_GROUP_ADMIN || info.groupid == USER_GROUP_UNLIMITED) ? 0 : info.expire_time - now;
				connection_reply(&remain_time, 4);
			} else {
				connection_reply(&result, 1);
			}
			rv = true;
		} else {
			result = LOGIN_RESULT_CLIENT_VERSION_EXPIRED;
			connection_reply(&result, 1);
			rv = true;
		}
	}
	return rv;
}

bool c_login_handler(void* message, uint32_t message_size) {
	bool rv = false;
	//account + password + align >= 4
	if (message_size >= sizeof(MessageHeader) + (4 * 4) + 4 + 4 + 4) { 
		char session_id[33];
		session_id[32] = 0;
		MessageHeader* header = reinterpret_cast<MessageHeader*>(message);
		uint32_t encrypted_size = message_size - sizeof(MessageHeader);
		if (encrypted_size % 4 == 0) {
			//get session_id
			memcpy(session_id, header->session_id, 32);

			uint8_t key[16];
			if (m_user_get_session_key_by_id(session_id, key)) {
				utils_xxtea_decrypt(utils_poniter_calc<uint8_t*>(message, sizeof(MessageHeader)), encrypted_size, key);
				MessageFieldsData data;
				if (message_get_fields(&data, message, message_size)) {
					if (data.fieldc == 4 && ConfigEnableBetaLogin > 0) { //BETA protocol
						rv = c_login_beta_login(session_id, &data);
					} else if (data.fieldc == 5) { //RELEASE protocol
						rv = c_login_release_login(session_id, &data);
					}
					message_clear_fields(&data);
				}
			}
		}
	}
		
	return rv;
}
