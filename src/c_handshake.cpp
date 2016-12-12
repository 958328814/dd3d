#include "dd3d.h"
#include "connection.h"
#include "crypt.h"
#include "m_user.h"
#include "log.h"

#include "c_handshake.h"

bool c_handshake_handler(void* message, uint32_t message_size) {
	bool rv = false;
	if (message_size == 4 + RSA_SIZE) {
		uint8_t key[RSA_SIZE];
		int key_size = crypt_rsa_private_decrypt(utils_poniter_calc<uint8_t*>(message, 4), RSA_SIZE, key);
		if (16 == key_size) {
			char reply_data[32 + RSA_SIZE];
			if (m_user_create_session(reply_data, key)) {
				utils_xxtea_encrypt((uint8_t*)reply_data, 32, key);
				key_size = crypt_rsa_private_encrypt(key, 16, utils_poniter_calc<uint8_t*>(reply_data, 32));
				if (key_size != -1) {
					connection_reply(reply_data, 32 + RSA_SIZE);
					return true;	
				}
			} else
				syslog(LOG_ERR, "[ERROR]create session failed.\n");
		} else {
			syslog(LOG_ERR, "[ERROR]handshake failed.\n");
		}
	}
	return rv;
}
