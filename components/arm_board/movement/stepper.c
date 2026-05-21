#include "stdint.h"
#include <math.h>
#include <stdlib.h>
#include "result.h"
#include "stepper.h"
#include "tim.h"
#include "logging.h"
#include "dma.h"

// FreeRTOS
#include "cmsis_os.h"

//Alledgedly, the steppers will be using CL57T drivers!
/*  CL57T driver
    Compatible with NEMA23 motor
    DEFAULT microstep resolution: 1600
    DEFAULT direction: counter clockwise
    ((I *think* that that means that 1 is CW and 0 is CCW))
    DEFAULT pulse mode: single pulse

    Pulse width should always be greater than 2.5 microsecs!!
*/

//Placeholder pins
#define PLS_PIN 2 //Pulse signal
#define DIR_PIN 3 //Direction signal
#define ENA_PIN 4 //Enable signal
#define ALM_PIN 5 //Alarm signal (OUT)

#define DEGREES_PER_STEP 1.8 //One step turns the motor 1.8 degrees
#define STEPS_PER_REV (360/DEGREES_PER_STEP) //At 1.8, this is 200

#define RPM 100 //Rotations per minute
#define STEP_PULSE_MIN_WIDTH_TICKS 3U // 3 us at the 1 MHz timer tick rate

#define TAG "STEPPER"

/* Class variables */
int64_t pulse_ctr = 0;
int64_t amt_steps = 100;

void User_TIMPeriodElapsedCallback(TIM_HandleTypeDef* htim);
static uint32_t get_step_frequency_hz(void);
static uint32_t get_pwm_period_ticks(uint32_t frequency_hz);
static uint32_t get_pwm_compare_ticks(uint32_t period_ticks, uint8_t duty_cycle);
void set_pin(int pinname, char what);

static stepper_t* active_stepper = NULL;

result_t init_stepper(stepper_t* stepper, uint8_t id, uint8_t duty_cycle, TIM_HandleTypeDef* tim) {
    stepper->stepper_id = id;
    stepper->duty_cycle = duty_cycle;
    stepper->htim = tim;
    stepper->current_angle = 0;
    stepper->step_frequency_hz = get_step_frequency_hz();
    stepper->pwm_dma_buffer = NULL;
    stepper->pwm_dma_buffer_len = 0;
    stepper->pwm_dma_active = false;
    active_stepper = stepper;

    TIM_HandleTypeDef* htim = stepper->htim;

    uint32_t period_ticks = get_pwm_period_ticks(stepper->step_frequency_hz);
    __HAL_TIM_SET_AUTORELOAD(htim, period_ticks - 1U);
    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, get_pwm_compare_ticks(period_ticks, duty_cycle));
    HAL_TIM_PWM_Stop(htim, TIM_CHANNEL_1);

    return RESULT_OK;
}

void do_pwm(stepper_t* stepper) {

    TIM_HandleTypeDef* htim = stepper->htim;

    uint32_t ARR_val = (uint32_t) htim->Instance->ARR + 1;
    float DC = ((float) stepper->duty_cycle) / 100;
    uint32_t CH1_Duty_Cycle = DC * ARR_val;

    while (1) {
        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, CH1_Duty_Cycle);
    }
}

void do_pwm_dma(stepper_t* stepper, int amt_steps) {

    TIM_HandleTypeDef* htim = stepper->htim;
    uint32_t period_ticks = get_pwm_period_ticks(stepper->step_frequency_hz);
    uint32_t my_Duty_Cycle = get_pwm_compare_ticks(period_ticks, stepper->duty_cycle);

    // FIX 1: sync timer ARR to match current step_frequency_hz before starting DMA.
    // Without this, changing step_frequency_hz after init has no effect on the actual
    // timer frequency — the old ARR value from init_stepper would still be used.
    __HAL_TIM_SET_AUTORELOAD(htim, period_ticks - 1U);

    if (stepper->pwm_dma_active && stepper->pwm_dma_buffer != NULL) {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
        free(stepper->pwm_dma_buffer);
        stepper->pwm_dma_buffer = NULL;
        stepper->pwm_dma_buffer_len = 0;
        stepper->pwm_dma_active = false;
    }

    // FIX 3: removed the +1 from data_arr_size.
    // The old +1 added a trailing zero entry that DMA would write to CCR, causing a
    // spurious extra pulse and a second PulseFinishedCallback fire. The callback
    // already handles stopping DMA cleanly, so no sentinel zero is needed.
    int data_arr_size = amt_steps;
    uint32_t* data_arr_ptr = (uint32_t*) calloc(data_arr_size, sizeof(uint32_t));

    if (data_arr_ptr == NULL) {
        return;
    }

    for (int i = 0; i < amt_steps; i++) {
        data_arr_ptr[i] = my_Duty_Cycle;
    }

    stepper->pwm_dma_buffer = data_arr_ptr;
    stepper->pwm_dma_buffer_len = (size_t) data_arr_size;
    stepper->pwm_dma_active = true;

    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, data_arr_ptr[0]);
    if (HAL_TIM_PWM_Start_DMA(htim, TIM_CHANNEL_1, data_arr_ptr, data_arr_size) != HAL_OK) {
        free(data_arr_ptr);
        stepper->pwm_dma_buffer = NULL;
        stepper->pwm_dma_buffer_len = 0;
        stepper->pwm_dma_active = false;
    }
}

void rotate_stepper(stepper_t* stepper, uint32_t amt_steps_absolute) {

    uint32_t current_angle = stepper->current_angle % (uint32_t)STEPS_PER_REV;
    uint32_t target_angle  = amt_steps_absolute    % (uint32_t)STEPS_PER_REV;
    uint32_t CW_angle      = (target_angle + (uint32_t)STEPS_PER_REV - current_angle) % (uint32_t)STEPS_PER_REV;
    uint32_t CCW_angle     = ((uint32_t)STEPS_PER_REV - CW_angle) % (uint32_t)STEPS_PER_REV;
    uint32_t amt_steps_relative = (CW_angle < CCW_angle) ? CW_angle : CCW_angle;

    bool pin_val = (CW_angle < CCW_angle) ? 1 : 0;
    set_pin(DIR_PIN, pin_val);

    do_pwm_dma(stepper, amt_steps_relative);

    // FIX 2: wait for DMA to finish before updating current_angle.
    // Previously current_angle was updated immediately after do_pwm_dma() returned,
    // but do_pwm_dma() is non-blocking — the motor physically hasn't moved yet.
    // A back-to-back rotate_stepper() call would compute the wrong relative angle.
    // osDelay(1) yields the CPU to other FreeRTOS tasks while waiting.
    while (stepper->pwm_dma_active) {
        osDelay(1);
    }

    stepper->current_angle = target_angle;
}

/** PLACEHOLDER
 * @brief Set the pin object
 */
void set_pin(int pinname, char what) {
    return;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef* htim) {
    stepper_t* stepper = active_stepper;

    if (stepper == NULL || stepper->htim != htim) {
        return;
    }

    HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);

    if (stepper->pwm_dma_buffer != NULL) {
        free(stepper->pwm_dma_buffer);
        stepper->pwm_dma_buffer = NULL;
    }

    stepper->pwm_dma_buffer_len = 0;
    stepper->pwm_dma_active = false;
}

static uint32_t get_step_frequency_hz(void) {
    float steps_per_second = ((float) RPM * (float) STEPS_PER_REV) / 60.0f;
    if (steps_per_second < 1.0f) {
        return 1U;
    }

    return (uint32_t) steps_per_second;
}

static uint32_t get_pwm_period_ticks(uint32_t frequency_hz) {
    if (frequency_hz == 0U) {
        frequency_hz = 1U;
    }

    uint32_t period_ticks = STEPPER_PWM_TIMER_TICK_HZ / frequency_hz;
    if (period_ticks < 2U) {
        period_ticks = 2U;
    }

    return period_ticks;
}

static uint32_t get_pwm_compare_ticks(uint32_t period_ticks, uint8_t duty_cycle) {
    uint32_t compare_ticks = (period_ticks * duty_cycle) / 100U;

    if (compare_ticks < STEP_PULSE_MIN_WIDTH_TICKS) {
        compare_ticks = STEP_PULSE_MIN_WIDTH_TICKS;
    }

    if (compare_ticks >= period_ticks) {
        compare_ticks = period_ticks - 1U;
    }

    return compare_ticks;
}