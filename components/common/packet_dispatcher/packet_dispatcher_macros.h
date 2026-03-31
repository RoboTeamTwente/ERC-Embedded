#ifndef PACKET_DISPATCHER_MACROS_H
#define PACKET_DISPATCHER_MACROS_H
#define PACKET_HANDLER_DEFAULT_PRIORITY (tskIDLE_PRIORITY + 2U)
#define PACKET_HANDLER_DEFAULT_QUEUE_LENGTH (5U)
#define PACKET_HANDLER_DEFAULT_STACK_DEPTH (0U)

#define PACKET_HANDLER_CONFIG_STATIC(name, packet_tag, payload_member,         \
                                     handler_fn)                               \
    static uint8_t                                                             \
        name##_queue_buffer[PACKET_HANDLER_DEFAULT_QUEUE_LENGTH *              \
                            sizeof(((PBEnvelope*)0)->payload.payload_member)]; \
    static packet_handler_config_t name = {                                    \
        .handler = (handler_fn),                                               \
        .task_name = #name,                                                    \
        .packet_type = (packet_tag),                                           \
        .task_priority = PACKET_HANDLER_DEFAULT_PRIORITY,                      \
        .task_stack_depth = PACKET_HANDLER_DEFAULT_STACK_DEPTH,                \
        .item_size = sizeof(((PBEnvelope*)0)->payload.payload_member),         \
        .queue_length = PACKET_HANDLER_DEFAULT_QUEUE_LENGTH,                   \
        .queue_buffer = name##_queue_buffer,                                   \
        .queue_struct = {0},                                                   \
        .queue = NULL,                                                         \
    }

#define PACKET_HANDLER_CONFIG_STATIC_EX(name, packet_tag, payload_member,      \
                                        handler_fn, priority_, stack_depth_,   \
                                        queue_length_)                         \
    static uint8_t                                                             \
        name##_queue_buffer[(queue_length_) *                                  \
                            sizeof(((PBEnvelope*)0)->payload.payload_member)]; \
    static packet_handler_config_t name = {                                    \
        .handler = (handler_fn),                                               \
        .task_name = #name,                                                    \
        .packet_type = (packet_tag),                                           \
        .task_priority = (priority_),                                          \
        .task_stack_depth = (stack_depth_),                                    \
        .item_size = sizeof(((PBEnvelope*)0)->payload.payload_member),         \
        .queue_length = (queue_length_),                                       \
        .queue_buffer = name##_queue_buffer,                                   \
        .queue_struct = {0},                                                   \
        .queue = NULL,                                                         \
    }

#endif
