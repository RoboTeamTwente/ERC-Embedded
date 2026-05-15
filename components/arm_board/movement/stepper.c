#include "stdint.h"
#include <math.h>
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

#define TAG "STEPPER"

/* Class variables */
int64_t pulse_ctr = 0; //counts the amt of pulses that happened
int64_t amt_steps = 100; //amount of pulses that we want, DEFAULT VAL FOR TESTING

//Custom interrupt function definition
void User_TIMPeriodElapsedCallback(TIM_HandleTypeDef* htim);

result_t init_stepper(stepper_t* stepper, uint8_t id, uint8_t duty_cycle, TIM_HandleTypeDef* tim) {
    stepper->stepper_id = id;
    stepper->duty_cycle = duty_cycle;
    stepper->htim = tim;
    stepper->current_angle = 0;

    TIM_HandleTypeDef* htim = stepper->htim;

    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1); //Start the timer for PWM //NOTE: ONLY on CHANNEL_1

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

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

void do_pwm_slave(stepper_t* stepper) {

    // htim3.Instance->PSC = 0;

    int amt_pulses = 10;

    int tim2_arr = 65535;
    int tim3_arr = tim2_arr/amt_pulses;
    htim2.Instance->ARR = tim2_arr-1;
    htim3.Instance->ARR = tim3_arr-1;

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 100);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (int) tim3_arr*0.5); //50% duty cycle

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    
}

/**
 * @brief Generates a continuous PWM signal for testing purposes.
 * @note This is a simplified function to verify PWM output with an oscilloscope.
 * It sets a 50% duty cycle on the timer channel associated with the stepper.
 * The PWM signal is started in init_stepper.
 */
void test_generate_pwm(stepper_t* stepper) {
    TIM_HandleTypeDef* htim = stepper->htim;

    // Calculate the compare value for a 50% duty cycle.
    // The ARR (Auto-Reload Register) value determines the PWM period and is configured in CubeMX.
    uint32_t arr_val = __HAL_TIM_GET_AUTORELOAD(htim);
    uint32_t compare_val = arr_val / 2;

    // Set the duty cycle by writing to the Compare Register.
    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, compare_val);
    LOGI(TAG, "Continuous PWM test started. ARR: %lu, Compare (Duty): %lu", arr_val, compare_val);
}

extern DMA_HandleTypeDef hdma_tim2_ch1;

#define PWM_BUFFER_SIZE 100
uint32_t pwm_buffer[PWM_BUFFER_SIZE];

/**
 * @brief Tests PWM output using DMA to create a "breathing" LED effect.
 * @note This function assumes TIM2_CH1 is used for PWM and its corresponding
 *       DMA channel (DMA1 Stream 1 for TIM2_CH1 on H753) is configured for
 *       Memory-to-Peripheral transfers in Circular mode.
 *       The data width for both memory and peripheral should be set to Word (32-bit).
 *       This setup needs to be done in STM32CubeMX.
 * @param stepper A pointer to the stepper object containing the timer handle.
 */
void test_pwm_with_dma(stepper_t* stepper) {
    TIM_HandleTypeDef* htim = stepper->htim;
    uint32_t arr_val = __HAL_TIM_GET_AUTORELOAD(htim);

    // 1. Prepare a buffer of PWM duty cycle values (CCR values).
    // This will create a sine wave pattern for a "breathing" effect on the LED.
    for (int i = 0; i < PWM_BUFFER_SIZE; i++) {
        // Calculate sine wave value from 0 to 1
        float sine_val = (sinf(i * 2.0f * (float)M_PI / PWM_BUFFER_SIZE) + 1.0f) / 2.0f;
        // Scale it to the timer's period (ARR)
        pwm_buffer[i] = (uint32_t)(sine_val * arr_val);
    }

    // 2. Link the timer's DMA handle for Channel 1.
    // In CubeMX, you should associate the DMA stream with TIM2_CH1.
    // This line ensures the timer handle knows about its DMA stream handle.
    __HAL_LINKDMA(htim, hdma[TIM_DMA_ID_CC1], hdma_tim2_ch1);

    // 3. Start the PWM signal generation with DMA.
    // The HAL function will configure the DMA and start the transfer.
    // - htim: pointer to timer handle
    // - TIM_CHANNEL_1: the timer channel to use
    // - (uint32_t*)pwm_buffer: pointer to the source data buffer in memory
    // - PWM_BUFFER_SIZE: the number of values to transfer before wrapping around (in circular mode)
    if (HAL_TIM_PWM_Start_DMA(htim, TIM_CHANNEL_1, (uint32_t*)pwm_buffer, PWM_BUFFER_SIZE) != HAL_OK) {
        LOGE(TAG, "Failed to start PWM with DMA");
        Error_Handler();
    }

    LOGI(TAG, "PWM with DMA test started. LED on PA0 should be breathing.");
}


void rotate_stepper(stepper_t* stepper, uint32_t target_angle_absolute) {

    uint32_t CW_angle = target_angle_absolute - stepper->current_angle; //relative clockwise turn
    uint32_t CCW_angle = 360 - CW_angle; //relative counterclockwise turn

    uint32_t target_angle_relative = (CW_angle < CCW_angle) ? CW_angle : CCW_angle;
    bool pin_val = (CW_angle < CCW_angle) ? 1 : 0; //set pin to 1 for clockwise, 0 for counterclockwise

    set_pin(DIR_PIN, pin_val); //set direction of turn

    amt_steps = target_angle_relative; //set the amount of steps to turn for the interrupt to happen
    stepper->current_angle = target_angle_absolute; //update the angle in the struct //TODO: ONLY set once movement completed!
    
    // NOTE: The do_pwm_slave function below has issues. It uses global, uninitialized timers
    // and does not seem to correctly generate a fixed number of pulses.
    // For testing, it's better to generate a continuous PWM signal first to verify the hardware setup.
    // You can use the `test_generate_pwm` function for that.
    do_pwm_slave(stepper);
    //!! Execution should not reach here, since do_pwm will go into infinite loop !!

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
