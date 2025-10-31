

#include "timer_lib.h"
#include "logging.h"
#include <stdlib.h>

#define TAG "TIMER"

timer_callbacks_t **timer_callbacks = NULL;
size_t timer_callbacks_len = 0;

void TIM_add_callback(tim_update_callback_t callback, TIM_HandleTypeDef *htim) {
  size_t new_len = (timer_callbacks_len + 1) * sizeof(timer_callbacks_t);
  timer_callbacks_t **temp = realloc(timer_callbacks, new_len);
  if (temp == NULL) {
    LOG(TAG, "Could not allocate memory");
    free(temp);
    return;
  }
  free(timer_callbacks);
  timer_callbacks = temp;
  timer_callbacks[timer_callbacks_len]->callback = callback;
  timer_callbacks[timer_callbacks_len]->htim = htim;

  timer_callbacks_len += 1;
}

void TIM_update_callback(tim_update_callback_t callback,
                         TIM_HandleTypeDef *htim) {
  for (int i = 0; i < sizeof(*timer_callbacks); i++) {
    if (timer_callbacks[i]->htim == htim) {
      timer_callbacks[i]->callback = callback;
      return;
    }
  }

  TIM_add_callback(callback, htim);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  for (int i = 0; i < timer_callbacks_len; i++) {
    if (timer_callbacks[i]->htim == htim) {
      timer_callbacks[i]->callback();
    }
  }
}
