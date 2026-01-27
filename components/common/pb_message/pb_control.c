#include "pb_control.h"
#include "result.h"
#include <stdbool.h>

#ifdef UNIT_TEST
void pb_control_test_reset(void) {
  memset(DispatchTable, 0, sizeof(DispatchTable));
  DispachTableSize = 0;
}
#endif

result_t pb_control_initialize(uint16_t *packet_types,
                               packet_handler_t *handlers,
                               uint16_t num_packet_types) {
  if (num_packet_types > MAX_PROCESSABLE_PACKET_TYPES) {
    return RESULT_ERR_INVALID_ARG;
  }

  if (num_packet_types > 0U && (packet_types == NULL || handlers == NULL)) {
    return RESULT_ERR_INVALID_ARG;
  }

  for (uint16_t i = 0U; i < num_packet_types; i++) {
    if (handlers[i] == NULL) {
      return RESULT_ERR_INVALID_ARG;
    }

    DispatchTable[i].type = packet_types[i];
    DispatchTable[i].handler = handlers[i];
  }

  /* Selection sort by type (ascending), in-place to allow for bin search
   * later*/
  for (uint16_t i = 0U; i < num_packet_types; i++) {
    uint16_t min_index = i;

    for (uint16_t j = (uint16_t)(i + 1U); j < num_packet_types; j++) {
      if (DispatchTable[j].type < DispatchTable[min_index].type) {
        min_index = j;
      }
    }
    if (i > 0 && DispatchTable[min_index].type == DispatchTable[i - 1U].type) {
      /* Duplicate packet type */
      return RESULT_ERR_INVALID_ARG;
    }

    if (min_index == i) {
      continue;
    }

    handler_entry_t tmp = DispatchTable[i];
    DispatchTable[i] = DispatchTable[min_index];
    DispatchTable[min_index] = tmp;
  }

  DispachTableSize = num_packet_types;
  return RESULT_OK;
}

result_t pb_control_process_incoming_packet(const uint8_t *packet_data,
                                            size_t packet_size) {
  if (packet_data == NULL || packet_size < 3) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (DispachTableSize == 0U) {
    return RESULT_ERR_NOT_INITIALIZED;
  }

  uint16_t packet_type = (packet_data[0] << 8) | packet_data[1];
  uint16_t lhs = 0U;
  uint16_t rhs = DispachTableSize;
  uint16_t mid;
  while (lhs < rhs) {
    mid = (uint16_t)(lhs + ((uint16_t)(rhs - lhs) >> 1));
    if (DispatchTable[mid].type == packet_type) {
      if (DispatchTable[mid].handler == NULL) {
        return RESULT_FAIL;
      }
      return DispatchTable[mid].handler(packet_data + 2, packet_size - 2);
    } else if (DispatchTable[mid].type < packet_type) {
      lhs = (uint16_t)(mid + 1U);
    } else {
      rhs = mid;
    }
  }
  return RESULT_ERR_NOT_FOUND;
}

bool pb_control_is_initialized(void) { return DispachTableSize > 0; }
