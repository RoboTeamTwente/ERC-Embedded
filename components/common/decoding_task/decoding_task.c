#include "decoding_task.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "components/common/envelope.pb.h"
#include "logging.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_message.h"
#include "portmacro.h"
#include "projdefs.h"
#include "result.h"
#include "stm/ethernet_udp.h"

const static char* TAG = "DECODING_TASK";

static PBEnvelope DecodingPacketArena[PB_ENVELOPE_NUMBER_OF_PACKET_TYPES];
static PBEnvelope DecodingEnvelopeCurrent;
static receive_frame DecodingPacketCurrent;

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

void PacketDispatcherTask(void* pvParameters) {
    packet_dispatcher_config_t* conf =
        (packet_dispatcher_config_t*)pvParameters;

    if (conf == NULL) {
        LOGE(TAG, "Dispatcher task arguments are null");
        vTaskDelete(NULL);
    }
    if (conf->input_queue == NULL) {
        LOGE(TAG, "Dispatcher task incoming queue is null");
        vTaskDelete(NULL);
    }
    if (conf->task_count < 1) {
        LOGE(
            TAG,
            "Dispatcher task requires at least one packet type to be processed "
            "(otherwise why the hell would you use it)");
        vTaskDelete(NULL);
    }
    if (conf->tasks == NULL) {
        LOGE(TAG, "Packet task arg list is null");
        vTaskDelete(NULL);
    }
    configASSERT(conf != NULL);
    configASSERT(conf->input_queue != NULL);
    configASSERT(conf->tasks != NULL);
    configASSERT(conf->task_count > 0U);
    bool status;
    pb_istream_t istream;
    for (;;) {
        if (xQueueReceive(conf->input_queue, &DecodingPacketCurrent,
                          portMAX_DELAY) != pdTRUE) {
            LOGE(TAG, "Dispatcher could not receive packet form queue");
            continue;
        }

        if (DecodingPacketCurrent.len > PBEnvelope_size ||
            DecodingPacketCurrent.len == 0 ||
            DecodingPacketCurrent.payload == NULL) {
            LOGE(TAG,
                 "Incoming packet is NULL or its size is 0 or its size is "
                 "more than PBEnvelope allows");
            continue;
        }
        istream = pb_istream_from_buffer(DecodingPacketCurrent.payload,
                                         DecodingPacketCurrent.len);
        status =
            pb_decode(&istream, PBEnvelope_fields, &DecodingEnvelopeCurrent);

        if (status) {
            LOGE(TAG, "Dispatcher could not decode incoming packet");
            continue;
        }
        for (int i = 0; i < conf->task_count; i++) {
            if (conf->tasks[i].packet_type !=
                DecodingEnvelopeCurrent.which_payload) {
                continue;
            }
            if (xQueueSend(conf->tasks[i].queue,
                           &DecodingEnvelopeCurrent.payload,
                           portMAX_DELAY) != pdTRUE) {
                LOGE(TAG,
                     "Could not enqueue packet of type: %d into its "
                     "queue",
                     DecodingEnvelopeCurrent.which_payload);
            }
        }
    }
}

result_t PacketHandlerStart(packet_handler_config_t* handler_conf) {
    if (handler_conf->handler == NULL || handler_conf->task_name == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    if (handler_conf->packet_type >= PB_ENVELOPE_NUMBER_OF_PACKET_TYPES) {
        return RESULT_ERR_INVALID_ARG;
    }

    handler_conf->queue =
        xQueueCreate(handler_conf->queue_length, handler_conf->item_size);
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

result_t PacketDispatcherStart(packet_dispatcher_config_t* dispatcher_config) {
    if (dispatcher_config == NULL) {
        LOGE(TAG, "Dispatcher config provided is null");
        return RESULT_ERR_INVALID_ARG;
    }

    if ((dispatcher_config->tasks == NULL) &&
        (dispatcher_config->task_count > 0U)) {
        LOGE(TAG,
             "No Packet handlers NULL or task count is not higher than 0.");
        return RESULT_ERR_INVALID_ARG;
    }

    if (dispatcher_config->input_queue == NULL) {
        LOGE(TAG, "Incoming packet queue is NULL");
        return RESULT_ERR_INVALID_ARG;
    }

    result_t res;
    for (size_t i = 0; i < dispatcher_config->task_count; i++) {
        res = PacketHandlerStart(&dispatcher_config->tasks[i]);
        if (res != RESULT_OK) {
            LOGE(TAG, "Packet handler %s could not be created because: %s",
                 dispatcher_config->tasks[i].task_name,
                 result_to_short_str(res));
        }
    }

    if (dispatcher_config->dispatcher_stack_depth <= 0) {
        dispatcher_config->dispatcher_stack_depth =
            PACKET_DISPATCHER_TASK_STACK_DEPTH;
    }

    BaseType_t status = xTaskCreate(
        PacketDispatcherTask, "PktDispatch",
        dispatcher_config->dispatcher_stack_depth, (void*)dispatcher_config,
        dispatcher_config->dispatcher_priority, NULL);

    if (status != pdPASS) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}
