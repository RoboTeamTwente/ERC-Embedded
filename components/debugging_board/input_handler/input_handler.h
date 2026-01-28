#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#import "FreeRTOS.h"
#include "portmacro.h"
#import "queue.h"
#include "result.h"
#import "stdint.h"

#define INPUT_KEY_LIST(X)                                                      \
  X(UP)                                                                        \
  X(DOWN)                                                                      \
  X(LEFT)                                                                      \
  X(RIGHT)                                                                     \
  X(MID)                                                                       \
  X(SELECT)                                                                    \
  X(RESET)

typedef enum {
#define X(name) INPUT_KEY_##name,
  INPUT_KEY_LIST(X)
#undef X
} input_handler_key_t;

enum {
#define X(name) +1
  INPUT_HANDLER_KEY_COUNT = 0 INPUT_KEY_LIST(X)
#undef X
};

#define INPUT_HANDLER_KEY_MAS(key) (1u << (key))

typedef enum {
  INPUT_HANDLER_EVENT_KEY_PRESSED = 0,
  INPUT_HANDLER_EVENT_KEY_RELEASED,
  INPUT_HANDLER_EVENT_KEY_HELD,
} input_handler_event_type_t;

typedef struct {
  input_handler_key_t key;
  input_handler_event_type_t event_type;
  TickType_t timestamp;
} input_handler_event_t;

typedef struct {
  uint8_t debounce_samples; // Number of consecutive identical samples required
                            // to accept change
  uint8_t enable_repeat;    // Have repeated key events when held

  uint16_t repeat_initial_delay_ms; // Delay before first repeat event
  uint16_t repeat_interval_ms;      // Interval between repeat events
  uint8_t repeat_keys_mask;         // Bitmask of keys that should repeat

  QueueHandle_t event_queue; // Queue to post input events to

} input_handler_config_t;

typedef struct {
  input_handler_config_t config;
  uint8_t stable_mask; // Gets you the debounced state of all keys
  uint8_t last_raw_mask;
  uint8_t stable_count[INPUT_HANDLER_KEY_COUNT];
  TickType_t press_tick[INPUT_HANDLER_KEY_COUNT];
  TickType_t next_repeat_tick[INPUT_HANDLER_KEY_COUNT];
} input_handler_t;

result_t input_handler_init(input_handler_t *handler,
                            const input_handler_config_t *config);

result_t input_handler_process_sample(input_handler_t *handler,
                                      uint8_t raw_key_mask, TickType_t tick);

static inline uint8_t
input_handler_get_stable_mask(const input_handler_t *handler) {
  return handler->stable_mask;
}

static inline result_t input_handler_recv(const input_handler_t *handler,
                                          input_handler_event_t *event,
                                          TickType_t ticks_to_wait) {
  return xQueueReceive(handler->config.event_queue, (void *)event,
                       ticks_to_wait) == pdTRUE
             ? RESULT_OK
             : RESULT_ERR_TIMEOUT;
}

uint16_t input_handler_empty_event_queue(const input_handler_t *handler);

#endif // !INPUT_HANDLER_H
