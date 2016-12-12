#include "dd3d.h"
#include "message.h"
#include "m_user.h"
#include "m_offsets.h"
#include "connection.h"

#include "c_offsets.h"

bool c_offsets_handler(void* message, uint32_t message_size) {
	bool rv = false;

	if (message_size == sizeof(MessageHeader) + (2 * 4) + 4 + 4) { 
		char session_id[33];
		session_id[32] = 0;
		MessageHeader* header = reinterpret_cast<MessageHeader*>(message);
		uint32_t encrypted_size = message_size - sizeof(MessageHeader);
		if (encrypted_size % 4 == 0) {
			//get session_id
			memcpy(session_id, header->session_id, 32);

			uint8_t key[16];
			if (m_user_get_session_key_by_id(session_id, key, true)) {
				utils_xxtea_decrypt(utils_poniter_calc<uint8_t*>(message, sizeof(MessageHeader)), encrypted_size, key);
				MessageFieldsData data;
				if (message_get_fields(&data, message, message_size)) {
					if (data.fieldc == 2 && data.fields_size[0] == 4 && data.fields_size[1] == 4) {
						uint32_t ver = *((uint32_t*)data.fields_data[0]);
						uint32_t client_key = *((uint32_t*)data.fields_data[1]);
						*(uint32_t*)key ^= client_key; //KEY CHANGED HERE
						
						syslog(LOG_INFO, "[OFFSETS] Session Id: %s\n", session_id);
						syslog(LOG_INFO, "[OFFSETS] Version: %u, Client Key: 0x%X\n", ver, client_key);
						const OffsetDataInfo* info = m_offsets_get_data(ver);
						if (info) {
							uint8_t* reply_data = new uint8_t[info->size];
							memcpy(reply_data, info->data, info->size);
							utils_xxtea_encrypt(reply_data, info->size, key);
							connection_reply(reply_data, info->size);
							delete reply_data;
							rv = true;
						}
					}
					message_clear_fields(&data);
				}
			}
		}
	}
	return rv;
}


