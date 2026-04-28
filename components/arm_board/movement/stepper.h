#include <stdbool.h>
#include "result.h"

//1:1 recration of protobuf
typedef struct {
    uint32_t current_angle;
    uint32_t duty_cycle; //Value of 1-100%
    uint8_t timer_number; //Value from 1-17; The timer that gets used for the interrupt
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
 * @param target_angle_absolute
 * @return uint32_t 
 */
result_t rotate_stepper(uint32_t target_angle_absolute);