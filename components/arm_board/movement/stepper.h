#include <stdbool.h>
#include "result.h"
#include "tim.h"
#include "string.h"

//1:1 recration of protobuf
typedef struct {
    uint8_t stepper_id;
    uint8_t duty_cycle; //Value of 1-100%
    uint16_t current_angle; //Value from 0-359
    TIM_HandleTypeDef* htim; //The timer that is used
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


result_t init_stepper(stepper_t* stepper, uint8_t id, uint8_t duty_cycle, TIM_HandleTypeDef* tim);
void rotate_stepper(stepper_t* stepper, uint32_t target_angle_absolute);