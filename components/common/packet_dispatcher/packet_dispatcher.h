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

/**
 * @brief Packet handler callback type.
 *
 * This function is invoked by the dispatcher when a packet of the corresponding
 * type is received. The provided buffer contains the decoded payload for the
 * packet type associated with the handler.
 * @note The provided buffer will be of the decoded protobuffer that is stored
 * in the PBEnvelope, not the PBEnvelope itself
 *
 * @param buffer Pointer to the decoded packet payload. The actual type depends
 *               on the packet_type this handler was registered for.
 *
 * @return result_t Result of the handler execution.
 */
typedef result_t (*packet_handler_t)(void* buffer);

/**
 * @brief Configuration for a packet handler task.
 *
 * Each entry describes a single packet type, its associated handler, and the
 * RTOS resources required to process incoming packets of that type.
 *
 * A FreeRTOS task and queue will be created per configuration entry during
 * dispatcher initialization.
 *
 * @note queue_buffer must be large enough to hold queue_length * item_size
 * bytes.
 * @note queue_struct and queue are initialized internally and must not be set
 *       by the caller.
 */
typedef struct {
    packet_handler_t handler; /**< Callback invoked for this packet type */
    const char* task_name;    /**< Name of the FreeRTOS task */
    pb_size_t packet_type;    /**< Nanopb oneof discriminator / packet type */

    UBaseType_t task_priority; /**< FreeRTOS task priority (optional) */
    configSTACK_DEPTH_TYPE task_stack_depth; /**< Task stack depth (optional) */

    size_t item_size; /**< Size of a single queue item (decoded payload) */
    UBaseType_t queue_length; /**< Number of items the queue can hold */

    uint8_t* queue_buffer;      /**< Backing buffer for static queue storage */
    StaticQueue_t queue_struct; /**< Internal static queue control structure */
    QueueHandle_t queue; /**< Handle to the created queue (set internally) */
} packet_handler_config_t;

/**
 * @brief Dispatch an incoming packet to the appropriate handler queue.
 *
 * Decodes the provided frame into a PBEnvelope, determines its packet type,
 * and enqueues the decoded payload into the corresponding handler queue.
 *
 * This function is typically called from a producer task or ISR-safe context
 * (if properly adapted) that receives raw frames.
 *
 * @param incoming_packet Pointer to the received frame containing encoded data.
 *
 * @note The function assumes that PacketDispatcherInit() has been successfully
 *       called and that handler queues are initialized.
 * @note If the queue for a given packet type is full, the packet may be dropped
 *       depending on the internal implementation.
 */
void DispatchPacket(receive_frame_t* incoming_packet);

/**
 * @brief Initialize the packet dispatcher and spawn handler tasks.
 *
 * Creates a FreeRTOS task and queue for each provided handler configuration.
 * Each task will block on its queue and invoke the associated handler when
 * new packets arrive.
 *
 * @param handlers Array of packet handler configurations.
 * @param handler_count Number of entries in the handlers array.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if input parameters are invalid.
 * @return RESULT_FAIL if task or queue creation fails.
 *
 * @note This function must be called before DispatchPacket().
 * @note The handlers array must remain valid for the lifetime of the system,
 *       as internal references to its elements may be used.
 */
result_t PacketDispatcherInit(packet_handler_config_t* handlers,
                              size_t handler_count);
#endif
