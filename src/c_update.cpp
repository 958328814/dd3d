#include "dd3d.h"
#include "m_user.h"
#include "m_service.h"
#include "connection.h"
#include "message.h"

#include "c_update.h"

const mysqlpp::sql_text* c_update_get_desc(const ReleaseInfo* info, uint32_t locale_id) {
	const mysqlpp::sql_text* rv = NULL;
	if (info->desc_map.size()) {
		if (locale_id == 0 || !info->desc_map.count(locale_id))
			locale_id = DEFAULT_LOCALE_ID;
		release_desc_map_t::const_iterator it = info->desc_map.find(locale_id);
		if (it != info->desc_map.end()) 
			rv = &(it->second);
	}
	return rv;
}

bool c_update_handler_no_session(void* message, uint32_t message_size) {
	bool rv = false;
	const ReleaseInfo* info = m_service_get_lastest_release_info();
	const mysqlpp::sql_text* desc = c_update_get_desc(info, 0);
	if (NULL != desc) {
		MessageFieldsData data;
		message_fields_init(&data, 4);
		message_field_add(&data, (uint8_t*)(&info->build), sizeof(mysqlpp::sql_int_unsigned));
		message_field_add(&data, (uint8_t*)(info->link.c_str()), info->link.size());
		message_field_add(&data, (uint8_t*)(desc->c_str()), desc->size());
		message_field_add(&data, (uint8_t*)(&info->date), sizeof(mysqlpp::sql_int_unsigned));
		uint32_t msg_size = message_size_calc(&data);
		if (msg_size <= sizeof(MsgBuffer) && message_build(MESSAGE_TYPE_UPDATE, &data, MsgBuffer, sizeof(MsgBuffer))) {
			connection_reply((uint8_t*)MsgBuffer, msg_size);
			rv = true;
		} else
			syslog(LOG_ERR, "[ERROR]Build update message failed.\n");
		message_clear_fields(&data);
	} else
		syslog(LOG_ERR, "[ERROR]Get lastest build information failed.\n");
	return rv;
}

bool c_update_handler_localized(void* message, uint32_t message_size) {
	bool rv = false;

	char session_id[33];
	memcpy(session_id, utils_poniter_calc<uint8_t*>(message, 2), 32);
	session_id[32] = 0;
	SessionInfo session_info;
	if (m_user_get_session_by_id(&session_info, session_id)) {
		const ReleaseInfo* info = m_service_get_lastest_release_info();
		const mysqlpp::sql_text* desc = c_update_get_desc(info, session_info.locale_id);
		if (NULL != desc) {
			MessageFieldsData data;
			message_fields_init(&data, 4);
			message_field_add(&data, (uint8_t*)(&info->build), sizeof(mysqlpp::sql_int_unsigned));
			message_field_add(&data, (uint8_t*)(info->link.c_str()), info->link.size());
			message_field_add(&data, (uint8_t*)(desc->c_str()), desc->size());
			message_field_add(&data, (uint8_t*)(&info->date), sizeof(mysqlpp::sql_int_unsigned));
			uint32_t msg_size = message_size_calc(&data);
			if (msg_size <= sizeof(MsgBuffer) && message_build(MESSAGE_TYPE_UPDATE, &data, MsgBuffer, sizeof(MsgBuffer))) {
				connection_reply((uint8_t*)MsgBuffer, msg_size);
				rv = true;
			} else
				syslog(LOG_ERR, "[ERROR]Build update message failed.\n");
			message_clear_fields(&data);
		} else
			syslog(LOG_ERR, "[ERROR]Get lastest build information failed.\n");
	}
	return rv;
}

bool c_update_handler(void* message, uint32_t message_size) {
	if (message_size == 2)
		return c_update_handler_no_session(message, message_size);
	else if (message_size == 32 + 2)
		return c_update_handler_localized(message, message_size);
	else
		return false;
}


