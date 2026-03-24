#ifndef DECODING_TASK_H
#define DECODING_TASK_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "components/common/envelope.pb.h"
#include "pb.h"
#include "portmacro.h"
#include "queue.h"
#include "result.h"
#include "stm/ethernet_udp.h"

#define PACKET_HANDLER_TASK_STACK_DEPTH_DEFAULT ((configSTACK_DEPTH_TYPE)512U)
#define PACKET_DISPATCHER_TASK_STACK_DEPTH ((configSTACK_DEPTH_TYPE)1024U)

typedef result_t (*packet_handler_t)(void* buffer);

typedef struct {
    packet_handler_t handler;
    const char* task_name;
    pb_size_t packet_type;
    UBaseType_t task_priority;
    configSTACK_DEPTH_TYPE task_stack_depth;  // Optional value
    size_t item_size;
    UBaseType_t queue_length;
    uint8_t* queue_buffer;       // must hold at least item_size*item_size bytes
    StaticQueue_t queue_struct;  // Not filled by caller
    QueueHandle_t queue;         // Not filled by called
} packet_handler_config_t;

typedef struct {
    packet_handler_config_t* tasks;
    const size_t task_count;
    configSTACK_DEPTH_TYPE dispatcher_stack_depth;
    const UBaseType_t dispatcher_priority;
    QueueHandle_t input_queue;
} packet_dispatcher_config_t;

void PacketHandlerTask(void* pvParameters);

void PacketDispatcherTask(void* pvParameters);

result_t PacketDispatcherStart(packet_dispatcher_config_t* dispatcher_config);

#endif
