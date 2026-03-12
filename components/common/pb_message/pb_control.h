#include "result.h"
#include <stdbool.h>

static char *TAG = "PB_CONTROL";

typedef result_t (*packet_handler_t)(const uint8_t *packet_data,
                                     size_t packet_size);

#define MAX_PROCESSABLE_PACKET_TYPES 64

typedef struct {
  uint16_t type;
  packet_handler_t handler;
} handler_entry_t;

static handler_entry_t DispatchTable[MAX_PROCESSABLE_PACKET_TYPES];
static uint16_t DispachTableSize = 0;

/**
 * @brief Initialize the packet dispatch table for protobuf control flow.
 *
 * @param[in] packet_types      Array of packet type IDs (length @p
 * num_packet_types).
 * @param[in] handlers          Array of handler callbacks corresponding 1:1
 * with
 *                              @p packet_types (length @p num_packet_types).
 * @param[in] num_packet_types  Number of entries in @p packet_types and @p
 * handlers.
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if:
 *         - @p num_packet_types exceeds MAX_PROCESSABLE_PACKET_TYPES, or
 *         - @p packet_types / @p handlers is NULL while @p num_packet_types >
 * 0, or
 *         - any handler entry is NULL (recommended validation).
 */
result_t pb_contorl_initialize(uint16_t *packet_types,
                               packet_handler_t *handlers,
                               uint16_t num_packet_types);

/**
 * @brief Process an incoming packet and dispatch to the registered handler.
 *
 * @param[in] packet_data  Pointer to the received packet bytes.
 * @param[in] packet_size  Total size of @p packet_data in bytes.
 *
 * @return RESULT_OK if the handler executes successfully.
 * @return RESULT_ERR_INVALID_ARG if @p packet_data is NULL or @p packet_size
 * < 3.
 * @return RESULT_ERR_NOT_FOUND if the packet type is not registered.
 * @return RESULT_FAIL if a matching entry exists but its handler is NULL
 *         (recommended to guard against).
 *
 * @warning The dispatch table is global shared state. Ensure it is initialized
 *          before calling this function, and avoid concurrent modification.
 */
result_t pb_control_process_incoming_packet(const uint8_t *packet_data,
                                            size_t packet_size);

bool pb_control_is_initialized(void);
