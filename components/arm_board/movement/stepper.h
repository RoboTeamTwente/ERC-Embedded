#include <stdbool.h>
#include "result.h"
#include "tim.h"
#include "string.h"

#define STEPPER_PWM_TIMER_TICK_HZ 1000000U

//1:1 recration of protobuf
typedef struct {
    uint8_t stepper_id;
    uint8_t duty_cycle; //Value of 1-100%
    uint8_t current_angle; //Value from 0-199 (in steps)
    TIM_HandleTypeDef* htim; //The timer that is used
    uint32_t step_frequency_hz;
    uint32_t* pwm_dma_buffer;
    size_t pwm_dma_buffer_len;
    bool pwm_dma_active;
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
void rotate_stepper(stepper_t* stepper, uint8_t target_angle_absolute);
