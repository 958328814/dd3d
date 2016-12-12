#ifndef CONNECTION_H_INCLUDED
#define CONNECTION_H_INCLUDED

typedef bool (*MessageHandlerType)(void* message_raw, uint32_t message_length);
void connection_init(void);
void connection_cleanup(void);
void connection_process(void);
void connection_reply(void* message, uint32_t message_size);
void connection_register_handler(uint16_t type, MessageHandlerType handler);

#endif
