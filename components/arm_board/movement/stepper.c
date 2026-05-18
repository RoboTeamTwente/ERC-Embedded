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


void do_pwm_dma(stepper_t* stepper, int amt_steps) {

    /*Calculate the value of the CCR*/
    TIM_HandleTypeDef* htim = stepper->htim;
    uint32_t ARR_val = htim->Instance->ARR + 1; 
    float DC_val = ((float) stepper->duty_cycle) / 100; //Percentage -> multiplier (50 -> 0.5)
	uint32_t my_Duty_Cycle = DC_val * ARR_val;

    //Dynamically allocate an arr of 0s for the amount of steps + 1
    //Why amt_steps + 1? --> We need a 0 to close off the array after the wished for amt of steps
    int data_arr_size = amt_steps + 1;
    uint32_t* data_arr_ptr = (uint32_t*) calloc(data_arr_size, sizeof(uint32_t));

    // Populate the array
    for (int i = 0; i < amt_steps; i++) {
        data_arr_ptr[i] = my_Duty_Cycle;
    }

    //!TODO: error handling
    HAL_TIM_PWM_Start_DMA(htim, TIM_CHANNEL_1, data_arr_ptr, data_arr_size);

    free(data_arr_ptr);

    
}

void rotate_stepper(stepper_t* stepper, uint32_t amt_steps_absolute) {

    /* Calculate shortest the relative angle */
    //!NOTE: the "angles" are in amounts of steps and they are absolute
    uint32_t CW_angle = amt_steps_absolute - stepper->current_angle; //relative clockwise turn
    uint32_t CCW_angle = STEPS_PER_REV - CW_angle; //relative counterclockwise turn
    uint32_t amt_steps_relative = (CW_angle < CCW_angle) ? CW_angle : CCW_angle; //pick the shortest

    //set pin to 1 for clockwise, 0 for counterclockwise
    bool pin_val = (CW_angle < CCW_angle) ? 1 : 0; 
    set_pin(DIR_PIN, pin_val);
    
    do_pwm_dma(stepper, amt_steps_relative);

    //Update the stepper struct to reflect actual pos once movement completed!
    stepper->current_angle = amt_steps_absolute;
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
