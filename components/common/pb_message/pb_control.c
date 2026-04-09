#include "pb_control.h"
#include "result.h"
#include <stdbool.h>
#include "pb_message.h"

#ifdef UNIT_TEST
void pb_control_test_reset(void) {
  memset(DispatchTableEntries, 0, sizeof(DispatchTableEntries));
  memset(DispatchTableHandlers, 0, sizeof(DispatchTableHandlers));
  DispatchTableSize = 0;
}
#endif

result_t pb_control_initialize(const pb_size_t *processable_types,
                               const packet_handler_t *handlers,
                               uint16_t num_packet_types) {
  if (num_packet_types > MAX_PROCESSABLE_PACKET_TYPES) {
        return RESULT_ERR_INVALID_ARG;

  }

  if ((num_packet_types > 0U) &&
        (processable_types == NULL || handlers == NULL)) {
        return RESULT_ERR_INVALID_ARG;
  }
  memset(DispatchTableEntries, 0, sizeof(DispatchTableEntries));
  memset(DispatchTableHandlers, 0, sizeof(DispatchTableHandlers));
  memcpy(DispatchTableEntries, processable_types, sizeof(pb_size_t)*num_packet_types);
  memcpy(DispatchTableHandlers, handlers, sizeof(packet_handler_t)*num_packet_types);
  DispatchTableSize = num_packet_types;

  return RESULT_OK;
}


static PBEnvelope CurrentEnvelope;

result_t pb_control_process_incoming_packet(const void* packet_buffer, size_t size) {
  if (packet_buffer == NULL || size > PBEnvelope_size ) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (!pb_control_is_initialized()) {
    return RESULT_ERR_NOT_INITIALIZED;
  }
  
  TRY(pb_message_decode_into(packet_buffer, size, PBEnvelope_fields, PBEnvelope_size, &CurrentEnvelope));
  for (int i=0; i< DispatchTableSize; i++) {
    if (CurrentEnvelope.which_payload == DispatchTableEntries[i]) {
      return DispatchTableHandlers[i](&CurrentEnvelope.payload);
    }
  }
  return RESULT_ERR_INVALID_PACKET;
}

bool pb_control_is_initialized(void) { return DispatchTableSize > 0; }
