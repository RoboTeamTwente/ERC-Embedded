#include <stdbool.h>
#include "result.h"

//1:1 recration of protobuf
typedef struct {
    uint32_t current_angle;
    uint32_t pwm;
} stepper_t;

//1:1 recration of protobuf
typedef struct {
    float control_wrist_rotation;         
    float control_gripper_pitch;          
    float control_base;
    //Enable signals                
    float stepper_top_ENA;         
    float stepper_bottom_ENA;
    //Revolution           
    float stepper_top_REV;     
    float stepper_bottom_REV;             
} control_signals_t;

stepper_t stepper;

result_t init_stepper();

result_t control_signals_handler (control_signals_t signals);

/**
 * @brief turning the stepper using half drive mode
 * 
 * @param angle amount to turn
 * @param direction direction to turn
 * @return uint32_t seconds it takes to turn angle
 */
uint32_t stepper_make_steps(uint32_t angle);

/**
 * @brief calculates the rotation for the stepper and uses stepper_make_steps to turn
 * 
 * @param target_angle 
 * @return uint32_t 
 */
void rotate_stepper(uint32_t target_angle);

void do_pwm();

//placeholder 
void delayMicroseconds (uint32_t ms);

void delay_by_rpm();

//NOTE: for sequence half drive
/**
 * @brief returns if the PWM should be on or off for 2 pins
 * Placeholder bc it can only be implemented with the actual motors
 * 
 * @param step 
 */
void sequence_placeholder(int step);

//NOTE: for sequence half drive
/**
 * @brief returns if the PWM pins should be on/off 
 * 
 * @param step 
 */
void sequence(int step);

//NOTE: temporary method, will be replaced by HAL code
void set_pin(int pinname, char what);