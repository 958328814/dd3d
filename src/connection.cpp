#include "dd3d.h"
#include <zmq.hpp>
#include "message.h"
#include "log.h"
#include "utils.h"
#include "config.h"

#include "connection.h"
#include "database.h" //TODO remove dependence

const uint32_t MAX_HANDLER = 0xFF;

MessageHandlerType Handlers[MAX_HANDLER] = {0};
zmq::context_t* ZMQContext;
zmq::socket_t* ZMQSocket;
char* BindString = NULL;

void connection_init_zmq() {
	ZMQContext = new zmq::context_t(3);
	zmq::context_t& ref_ctx = *ZMQContext;
	ZMQSocket = new zmq::socket_t(ref_ctx, ZMQ_REP);
	ZMQSocket->bind(BindString);
}

void connection_cleanup_zmq() {
	delete ZMQSocket;
	delete ZMQContext;
}

void connection_init(void) {
	const char* bind_string;
	if (CONFIG_TRUE ==  config_lookup_string(config_get(), "dd3d.zeromq.bind", &bind_string)) {
		int len = strlen(bind_string);
		BindString = new char[len + 1];
		strncpy(BindString, bind_string, len + 1);

		connection_init_zmq();

	} else {
		throw("Connection bind string is missing.\n");
	}
}

void connection_register_handler(uint16_t type, MessageHandlerType handler) {
	Handlers[type] = handler;
}

void connection_reply(void* message, uint32_t message_size) {
	zmq::message_t msg(message_size);
	memcpy(msg.data(), message, message_size);
	ZMQSocket->send(msg);
}

void connection_process(void) {
	bool processed = false;
	while (!Terminate) {
		try {
			processed = false;
			zmq::message_t msg_recv;
			ZMQSocket->recv(&msg_recv);
			void* data = reinterpret_cast<uint8_t*>(msg_recv.data());
			uint32_t size = msg_recv.size();
			if (size > 0) {
				uint16_t type = *reinterpret_cast<uint16_t*>(data);
				if (type < MAX_HANDLER && Handlers[type]) {
					//Ping database
					database_ping();
					if (Handlers[type](data, size)) {
						processed = true;
					}
				}
				else {
					utils_bin_dump(data, size);
					syslog(LOG_ERR, "[ERROR]Unknown message type 0x%X discarded.\n", type);
				}
			}
			if (!processed) {
				syslog(LOG_ERR, "[INFO]Message can't be processed.\n");
				if (size > 0)
					utils_bin_dump(data, size);
				zmq::message_t msg_null(2);
				memset(msg_null.data(), 0, 2);
				ZMQSocket->send(msg_null);
			}
		} catch (zmq::error_t e) {
			if (!Terminate) {
				syslog(LOG_ERR, "[ERROR]%s\n", e.what());
				syslog(LOG_INFO, "[INFO]Restart socket.\n");

				connection_cleanup_zmq();
				connection_init_zmq();
			}
		}
	}

}

void connection_cleanup(void) {
	connection_cleanup_zmq();
}
