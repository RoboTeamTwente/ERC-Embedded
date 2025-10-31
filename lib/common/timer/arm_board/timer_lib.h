
#ifndef TIMER_LIB
#define TIMER_LIB

#include "tim.h"

/**
 * @brief function pointer for the timer callback
 *
 */
typedef void (*tim_update_callback_t)();

/**
 * @brief Struct for timer callbacks with an associated timer;
 */
typedef struct {
  tim_update_callback_t callback;
  TIM_HandleTypeDef *htim;
} timer_callbacks_t;

/**
 * @brief Adds callback function fo a certain timer
 *
 * @param callback The callback function of type tim_update_callback_t
 * @param tim Pointer to the handler of the timer
 */
void TIM_add_callback(tim_update_callback_t callback, TIM_HandleTypeDef *htim);

/**
 * @brief updates an callback function for a certain timer.
 *        If the timer has no callback function yet, it adds the new function
 *
 * @param callback The callback function of type tim_update_callback_t
 * @param tim Pointer to the handler of the timer
 */
void TIM_add_callback(tim_update_callback_t callback, TIM_HandleTypeDef *htim);

#endif // !TIMER_LIB
