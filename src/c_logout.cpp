#include "dd3d.h"
#include "utils.h"
#include "message.h"
#include "connection.h"
#include "crypt.h"
#include "m_user.h"

#include "c_logout.h"

bool c_logout_handler(void* message, uint32_t message_size) {
	bool rv = false;
	if (message_size == 2 + 32) { 
		char session_id[33];
		session_id[32] = 0;
		memcpy(session_id, utils_poniter_calc<void*>(message, 2), 32); 
		if (m_user_erase_session_by_id(session_id)) {
			uint8_t null_result = 0;
			connection_reply(&null_result, 1);
			rv = true;
		}
	}
		
	return rv;
}
