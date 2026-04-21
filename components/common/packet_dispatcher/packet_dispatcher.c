#include "packet_dispatcher.h"

#include <stdint.h>
#include <string.h>

#include "components/common/envelope.pb.h"
#include "logging.h"
#include "pb.h"
#include "pb_decode.h"
#include "portmacro.h"
#include "projdefs.h"
#include "result.h"
#include "stm/ethernet_udp.h"

const static char* TAG = "DISPATCHER";

static PBEnvelope DecodingEnvelopeCurrent;
static packet_handler_config_t* PacketHandlers;
static size_t PacketHandlerCount;

void PacketHandlerTask(void* pvParameters) {
    packet_handler_config_t* conf = (packet_handler_config_t*)pvParameters;
    result_t res;
    if (conf == NULL) {
        LOGE(TAG, "Provided arguments for packet task are null");
        vTaskDelete(NULL);
    }
    if (conf->task_name == NULL) {
        LOGE(TAG, "Provided arguments for packet task has NULL name");
        vTaskDelete(NULL);
    }
    if (conf->handler == NULL) {
        LOGE(TAG, "Handler for %s packet task is NULL", conf->task_name);
        vTaskDelete(NULL);
    }
    if (conf->queue == NULL) {
        LOGE(TAG, "Queue for %s packet task is NULL", conf->task_name);
        vTaskDelete(NULL);
    }
    uint8_t* packet_buffer = malloc(conf->item_size);
    if (packet_buffer == NULL) {
        vTaskDelete(NULL);
    }

    for (;;) {
        if (xQueueReceive(conf->queue, packet_buffer, portMAX_DELAY) !=
            pdTRUE) {
            LOGE(TAG, "Handler: %s, could not receive packet from queue",
                 conf->task_name);
            continue;
        }
        res = conf->handler(packet_buffer);
        LOGI(TAG, "Received packet");
        if (res != RESULT_OK) {
            LOGW(TAG, "Handler: %s, had error: %s", conf->task_name,
                 result_to_short_str(res));
        }
    }
}

void DispatchPacket(receive_frame_t* incoming_packet) {
    if (incoming_packet->len > PBEnvelope_size || incoming_packet->len == 0 ||
        incoming_packet->payload == NULL) {
        LOGE(TAG,
             "Incoming packet is NULL or its size is 0 or its size is "
             "more than PBEnvelope allows");
        return;
    }

    pb_istream_t istream =
        pb_istream_from_buffer(incoming_packet->payload, incoming_packet->len);
    bool status =
        pb_decode(&istream, PBEnvelope_fields, &DecodingEnvelopeCurrent);

    if (!status) {
        LOGE(TAG, "Dispatcher could not decode incoming packet");
        return;
    }
    for (int i = 0; i < PacketHandlerCount; i++) {
        if (PacketHandlers[i].packet_type !=
            DecodingEnvelopeCurrent.which_payload) {
            continue;
        }
        if (xQueueSend(PacketHandlers[i].queue,
                       &DecodingEnvelopeCurrent.payload,
                       portMAX_DELAY) != pdTRUE) {
            LOGE(TAG,
                 "Could not enqueue packet of type: %d into its "
                 "queue",
                 DecodingEnvelopeCurrent.which_payload);
        }
        return;
    }
    LOGW(TAG, "Received packet of type %d, could not be processsed",
         DecodingEnvelopeCurrent.which_payload);
}

result_t PacketHandlerStart(packet_handler_config_t* handler_conf) {
    if (handler_conf->handler == NULL || handler_conf->task_name == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    handler_conf->queue = xQueueCreateStatic(
        handler_conf->queue_length, handler_conf->item_size,
        handler_conf->queue_buffer, &handler_conf->queue_struct);
    if (handler_conf->queue == NULL) {
        return RESULT_FAIL;
    }
    if (handler_conf->task_stack_depth <= 0) {
        handler_conf->task_stack_depth =
            PACKET_HANDLER_TASK_STACK_DEPTH_DEFAULT;
    }
    BaseType_t status =
        xTaskCreate(PacketHandlerTask, handler_conf->task_name,
                    handler_conf->task_stack_depth, (void*)handler_conf,
                    handler_conf->task_priority, NULL);
    if (status != pdPASS) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}

result_t PacketDispatcherInit(packet_handler_config_t* handlers,
                              size_t handler_count) {
    if (handlers == NULL || handler_count <= 0) {
        LOGE(TAG,
             "Provided handlers are NULL or handler_count is not a positive "
             "number.");
        return RESULT_ERR_INVALID_ARG;
    }

    PacketHandlers = handlers;
    PacketHandlerCount = handler_count;
    result_t res;
    for (size_t i = 0; i < handler_count; i++) {
        res = PacketHandlerStart(&handlers[i]);
        if (res != RESULT_OK) {
            LOGE(TAG, "Packet handler %s could not be created because: %s",
                 handlers[i].task_name, result_to_short_str(res));
            return RESULT_FAIL;
        }
    }

    return RESULT_OK;
}
