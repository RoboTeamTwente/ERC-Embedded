#include "input_handler.h"
#include "logging.h"
#include <stdint.h>
#include <string.h>

const static char *TAG = "input_handler";

static result_t input_handler_send_event_(const input_handler_t *handler,
                                          input_handler_key_t key,
                                          input_handler_event_type_t event_type,
                                          TickType_t timestamp) {
  if (handler->config.event_queue != NULL) {
    input_handler_event_t event = {
        .key = key,
        .event_type = event_type,
        .timestamp = timestamp,
    };
    xQueueSend(handler->config.event_queue, &event, 0);
    return RESULT_OK;
  }
  LOGE(TAG, "Queue is NULL, event not sent");
  return RESULT_ERR_INVALID_STATE;
}

static inline BaseType_t tick_reached_(TickType_t now, TickType_t deadline) {
  return (BaseType_t)((int32_t)(now - deadline) >= 0);
}

result_t input_handler_init(input_handler_t *handler,
                            const input_handler_config_t *config) {
  if (handler == NULL || config == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  memset(handler, 0, sizeof(input_handler_t));
  handler->config = *config;
  if (handler->config.debounce_samples == 0) {
    handler->config.debounce_samples = 1;
  }
  return RESULT_OK;
}

result_t input_handler_process_sample(input_handler_t *handler,
                                      uint8_t raw_key_mask, TickType_t tick) {

  const TickType_t start_delay_ticks =
      pdMS_TO_TICKS(handler->config.repeat_initial_delay_ms);
  const TickType_t interval_ticks =
      pdMS_TO_TICKS(handler->config.repeat_interval_ms);

  for (uint8_t key = 0u; key < INPUT_HANDLER_KEY_COUNT; key++) {
    const uint8_t bit = INPUT_HANDLER_KEY_MAS(key);
    const uint8_t raw_pressed = (raw_key_mask & bit) != 0u;
    const uint8_t stable_pressed = (handler->stable_mask & bit) != 0u;
    if (raw_pressed == stable_pressed) {
      // No change, reset counter
      handler->stable_count[key] = 0u;
    } else {
      if (handler->stable_count[key] < UINT8_MAX) {
        handler->stable_count[key]++;
      }
      uint8_t stable_count = handler->stable_count[key];

      if (stable_count >= handler->config.debounce_samples) {
        // State change confirmed
        if (raw_pressed) {
          // Key pressed
          handler->stable_mask |= bit;
          handler->press_tick[key] = tick;
          handler->next_repeat_tick[key] = tick + start_delay_ticks;
          TRY(input_handler_send_event_(handler, (input_handler_key_t)key,
                                        INPUT_HANDLER_EVENT_KEY_PRESSED, tick));
        } else {
          // Key released
          handler->stable_mask &= ~bit;
          handler->next_repeat_tick[key] = 0u;
          TRY(input_handler_send_event_(handler, (input_handler_key_t)key,
                                        INPUT_HANDLER_EVENT_KEY_RELEASED,
                                        tick));
        }
        handler->stable_count[key] = 0u;
      }
    }
    // Handle key repeat
    if (!handler->config.enable_repeat) {
      continue;
    }
    if (!(handler->config.repeat_keys_mask & bit)) {
      continue;
    }
    if (!(handler->stable_mask & bit)) {
      continue;
    }
    const TickType_t next = handler->next_repeat_tick[key];
    if (!next || !tick_reached_(tick, next)) {
      continue;
    }
    input_handler_send_event_(handler, (input_handler_key_t)key,
                              INPUT_HANDLER_EVENT_KEY_HELD, tick);
    if (interval_ticks > 0) {
      handler->next_repeat_tick[key] = tick + interval_ticks;
    } else {
      handler->next_repeat_tick[key] = 0u;
    }
  }
  handler->last_raw_mask = raw_key_mask;
  return RESULT_OK;
}

uint16_t input_handler_empty_event_queue(const input_handler_t *handler) {
  uint16_t count = 0u;
  input_handler_event_t event;
  while (xQueueReceive(handler->config.event_queue, &event, 0) == pdTRUE) {
    count++;
  }
  return count;
}
