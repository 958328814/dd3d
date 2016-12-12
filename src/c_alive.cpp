#include "dd3d.h"
#include "m_user.h"
#include "connection.h"

#include "c_alive.h"

bool c_alive_handler(void* message, uint32_t message_size) {
	bool rv = false;

	if (message_size == 2 + 32) { 
		char session_id[33];
		memcpy(session_id, utils_poniter_calc<uint8_t*>(message, 2), 32);
		session_id[32] = 0;
		
		uint16_t reply = m_user_alive_session_by_id(session_id) ? 1 : 0;
		connection_reply((uint8_t*)&reply, 2);

		rv = true;
	}
	return rv;
}


