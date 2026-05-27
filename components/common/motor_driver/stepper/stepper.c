
#include "stepper.h"

#include <math.h>
#include <stdlib.h>

#include "dma.h"
#include "logging.h"
#include "result.h"
#include "stdint.h"
#include "tim.h"

// FreeRTOS
#include "cmsis_os.h"

// Alledgedly, the steppers will be using CL57T drivers!
/*  CL57T driver
    Compatible with NEMA23 motor
    DEFAULT microstep resolution: 1600
    DEFAULT direction: counter clockwise
    ((I *think* that that means that 1 is CW and 0 is CCW))
    DEFAULT pulse mode: single pulse

    Pulse width should always be greater than 2.5 microsecs!!
*/

#define TAG "STEPPER"

void User_TIMPeriodElapsedCallback(TIM_HandleTypeDef* htim);
static uint32_t calc_ARR_ticks(uint32_t frequency_hz);
static uint32_t calc_CCR_ticks(uint32_t ARR_ticks, uint8_t duty_cycle);
void set_pin(int pinname, char what);

result_t init_stepper(stepper_t* stepper, uint8_t duty_cycle, TIM_HandleTypeDef* htim, pin_t dir_pin, pin_t ena_pin) {
    stepper->duty_cycle = duty_cycle;
    stepper->htim = htim;
    stepper->dir_pin = dir_pin;
    stepper->ena_pin = ena_pin;

    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);

    LOGI(TAG, "Stepper %u initialized", stepper->htim);
    return RESULT_OK;
}

void do_pwm_dma(stepper_t* stepper, int amt_steps, uint32_t freq) {
    // set the stepper freq to the new frequency
    stepper->frequency_hz = freq;

    TIM_HandleTypeDef* htim = stepper->htim;
    uint32_t ARR_ticks = calc_ARR_ticks(stepper->frequency_hz);
    uint32_t my_duty_cycle = calc_CCR_ticks(ARR_ticks, stepper->duty_cycle);

    __HAL_TIM_SET_AUTORELOAD(htim, ARR_ticks - 1U);

    // the size of the data arr will be one larger than the amt of steps
    // This is for a trailing 0 value, which will set pwm to 0 after finishing
    // transfer
    int data_arr_size = amt_steps + 1;
    uint32_t* data_arr_ptr = (uint32_t*)calloc(data_arr_size, sizeof(uint32_t));

    if (data_arr_ptr == NULL) {
        //! TODO: ERROR HANDLE;
        return;
    }

    // Fill the array with the desired duty cycle
    for (int i = 0; i < amt_steps; i++) {
        data_arr_ptr[i] = my_duty_cycle;
    }

    //! TODO: error handling
    HAL_StatusTypeDef res =
        HAL_TIM_PWM_Start_DMA(htim, TIM_CHANNEL_1, data_arr_ptr, data_arr_size);

    // Block until DMA transfer complete
    //! NOTE: CC1 means on channel 1!!!!!!!!!1
    while (htim->hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
        osDelay(1);  // Delay for thread switching
    }
    //TODO: WHAT if never gets out?

    free(data_arr_ptr);
}

void rotate_stepper(stepper_t* stepper, uint8_t amt_steps_absolute,
                    uint32_t freq) {
    /* Calculate shortest the relative angle */
    //!NOTE: the "angles" are in amounts of steps and they are absolute
    int32_t relative_angle = amt_steps_absolute - stepper->current_angle; //relative clockwise turn
    bool dir_pin_val = (relative_angle >= 0) ? GPIO_PIN_SET : GPIO_PIN_RESET; //set pin to 1 for clockwise, 0 for counterclockwise

    pin_t dir = stepper->dir_pin;
    HAL_GPIO_WritePin(dir.GPIOx, dir.GPIO_PIN_no, dir_pin_val); //set pin to 1 for clockwise, 0 for counterclockwise

    pin_t ena = stepper->ena_pin;
    HAL_GPIO_WritePin(ena.GPIOx, ena.GPIO_PIN_no, GPIO_PIN_SET); //enable stepper

    //DO ACTUAL MOVEMENT
    do_pwm_dma(stepper, abs(relative_angle), freq);

    //TODO: should this be here? and if so where?
    HAL_GPIO_WritePin(ena.GPIOx, ena.GPIO_PIN_no, GPIO_PIN_RESET); //disable stepper?

    stepper->current_angle = amt_steps_absolute;
}

/** PLACEHOLDER
 * @brief Set the pin object
 */
void set_pin(int pinname, char what) { return; }

/**
 * @brief Calculates the amount of ticks to put in the AutoReload register based
 * on the frequency in Hz that we want
 *
 * @param frequency_hz frequency in Hz
 * @return uint32_t amt of ticks to put in the ARR
 */
static uint32_t calc_ARR_ticks(uint32_t frequency_hz) {
    // Minimum divide by 1
    if (frequency_hz < 1U) {
        frequency_hz = 1U;
    }

    uint32_t ARR_ticks = STEPPER_PWM_TIMER_TICK_HZ / frequency_hz;

    // ARR is lower bounded by 2 (effectively 1 when done -1)
    if (ARR_ticks < 2U) {
        ARR_ticks = 2U;
    }

    // ARR is upper bounded by 65 535, freq is lower bouded by 16Hz
    if (ARR_ticks > 65535) {
        ARR_ticks = 65535;
    }

    return ARR_ticks;
}

/**
 * @brief Calculates the amount of ticks to put in CaptureCompare register based
 * on the ARR value and the wished for duty cycle
 *
 * @param ARR_ticks current value in ARR
 * @param duty_cycle desired duty cycle: value from 1-100
 * @return compare_ticks, the amt of ticks for in CCR
 */
static uint32_t calc_CCR_ticks(uint32_t ARR_ticks, uint8_t duty_cycle) {
    // Calculate the value to go in the CCR register based on
    uint32_t compare_ticks = (ARR_ticks * duty_cycle) / 100U;

    // Lower bounded by STEP_PULSE_MIN_WIDTH_TICKS
    if (compare_ticks < STEP_PULSE_MIN_WIDTH_TICKS) {
        compare_ticks = STEP_PULSE_MIN_WIDTH_TICKS;
    }

    // Upper bounded by ARR_ticks
    if (compare_ticks >= ARR_ticks) {
        compare_ticks = ARR_ticks - 1U;
    }

    return compare_ticks;
}