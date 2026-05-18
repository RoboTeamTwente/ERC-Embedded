#include "stdint.h"
#include <math.h>
#include <stdlib.h>
#include "result.h"
#include "stepper.h"
#include "tim.h"
#include "logging.h"
#include "dma.h"



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
int64_t pulse_ctr = 0; //counts the amt of pulses that happened
int64_t amt_steps = 100; //amount of pulses that we want, DEFAULT VAL FOR TESTING

//Custom interrupt function definition
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

    // //Register callback
    // HAL_TIM_RegisterCallback(htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, User_TIMPeriodElapsedCallback); //This links the PERIOD_ELAPSED_CB_ID (period elapsed callback) to user defined function
    // HAL_TIM_Base_Start_IT(htim); //Start the timer in interrupt mode! 

    return RESULT_OK;
}

void do_pwm(stepper_t* stepper) {

    TIM_HandleTypeDef* htim = stepper->htim;

    uint32_t ARR_val = (uint32_t) htim->Instance->ARR + 1; //NOTE: the ARR (counter register) value is set in CubeMX!
    float DC = ((float) stepper->duty_cycle) / 100; //Percentage -> multiplier
	uint32_t CH1_Duty_Cycle = DC * ARR_val;

    while (1) {
    	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, CH1_Duty_Cycle);
    }
}


void do_pwm_dma(stepper_t* stepper, int amt_steps) {

    /*Calculate the value of the CCR*/
    TIM_HandleTypeDef* htim = stepper->htim;
    uint32_t period_ticks = get_pwm_period_ticks(stepper->step_frequency_hz);
	uint32_t my_Duty_Cycle = get_pwm_compare_ticks(period_ticks, stepper->duty_cycle);

    if (stepper->pwm_dma_active && stepper->pwm_dma_buffer != NULL) {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
        free(stepper->pwm_dma_buffer);
        stepper->pwm_dma_buffer = NULL;
        stepper->pwm_dma_buffer_len = 0;
        stepper->pwm_dma_active = false;
    }

    //Dynamically allocate an arr of 0s for the amount of steps + 1
    //Why amt_steps + 1? --> We need a 0 to close off the array after the wished for amt of steps
    int data_arr_size = amt_steps + 1;
    uint32_t* data_arr_ptr = (uint32_t*) calloc(data_arr_size, sizeof(uint32_t));

    if (data_arr_ptr == NULL) {
        return;
    }

    // Populate the array
    for (int i = 0; i < amt_steps; i++) {
        data_arr_ptr[i] = my_Duty_Cycle;
    }

    //!TODO: error handling
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

    /* Calculate shortest the relative angle */
    //!NOTE: the "angles" are in amounts of steps and they are absolute
    uint32_t current_angle = stepper->current_angle % (uint32_t)STEPS_PER_REV;
    uint32_t target_angle = amt_steps_absolute % (uint32_t)STEPS_PER_REV;
    uint32_t CW_angle = (target_angle + (uint32_t)STEPS_PER_REV - current_angle) % (uint32_t)STEPS_PER_REV; //relative clockwise turn
    uint32_t CCW_angle = ((uint32_t)STEPS_PER_REV - CW_angle) % (uint32_t)STEPS_PER_REV; //relative counterclockwise turn
    uint32_t amt_steps_relative = (CW_angle < CCW_angle) ? CW_angle : CCW_angle; //pick the shortest

    //set pin to 1 for clockwise, 0 for counterclockwise
    bool pin_val = (CW_angle < CCW_angle) ? 1 : 0; 
    set_pin(DIR_PIN, pin_val);
    
    do_pwm_dma(stepper, amt_steps_relative);

    //Update the stepper struct to reflect actual pos once movement completed!
    stepper->current_angle = target_angle;
}

// void User_TIMPeriodElapsedCallback(TIM_HandleTypeDef* htim) {
//     LOGI(TAG, "Interrupt %d %u", pulse_ctr);
//     pulse_ctr++;

//     if (pulse_ctr >= amt_steps) { //If at the desired amt of pulses...
//         LOGI(TAG, "Interrupt %d %u", pulse_ctr);
//         while (1) {
//             pulse_ctr = 0;
//             __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0); //...turn PWM off FOR THIS TIMER
//         }
//     }
// }

/** PLACEHOLDER
 * @brief Set the pin object
 * 
 * @param pinname 
 * @param what 
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
