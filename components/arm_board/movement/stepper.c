#include "stdint.h"
#include "result.h"
#include "stepper.h"
#include "tim.h"
#include "logging.h"


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
void User_TIMPeriodElapsedCallback(TIM_HandleTypeDef* htim, stepper_t* stepper);

result_t init_stepper(stepper_t* stepper, uint8_t id, uint8_t duty_cycle, TIM_HandleTypeDef* tim) {
    stepper->stepper_id = id;
    stepper->duty_cycle = duty_cycle;
    stepper->htim = tim;
    stepper->current_angle = 0;

    TIM_HandleTypeDef* htim = stepper->htim;

    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1); //Start the timer for PWM //NOTE: ONLY on CHANNEL_1

    //Register callback
    HAL_TIM_RegisterCallback(htim, HAL_TIM_PERIOD_ELAPSED_CB_ID, User_TIMPeriodElapsedCallback); //This links the PERIOD_ELAPSED_CB_ID (period elapsed callback) to user defined function
    HAL_TIM_Base_Start_IT(htim); //Start the timer in interrupt mode! 

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

void rotate_stepper(stepper_t* stepper, uint32_t target_angle_absolute) {

    uint32_t CW_angle = target_angle_absolute - stepper->current_angle; //relative clockwise turn
    uint32_t CCW_angle = 360 - CW_angle; //relative counterclockwise turn

    uint32_t target_angle_relative = (CW_angle < CCW_angle) ? CW_angle : CCW_angle;
    bool pin_val = (CW_angle < CCW_angle) ? 1 : 0; //set pin to 1 for clockwise, 0 for counterclockwise

    set_pin(DIR_PIN, pin_val); //set direction of turn

    amt_steps = target_angle_relative; //set the amount of steps to turn for the interrupt to happen
    stepper->current_angle = target_angle_absolute; //update the angle in the struct //TODO: ONLY set once movement completed!
    
    do_pwm(stepper);
    //!! Execution should not reach here, since do_pwm will go into infinite loop !!

}

void User_TIMPeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    LOGI(TAG, "Interrupt %d %u", pulse_ctr);
    pulse_ctr++;

    if (pulse_ctr >= amt_steps) { //If at the desired amt of pulses...
        LOGI(TAG, "Interrupt %d %u", pulse_ctr);
        while (1) {
            pulse_ctr = 0;
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0); //...turn PWM off FOR THIS TIMER
        }
    }
}

/** PLACEHOLDER
 * @brief Set the pin object
 * 
 * @param pinname 
 * @param what 
 */
void set_pin(int pinname, char what) {
    return;
}

