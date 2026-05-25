#include <stdbool.h>
#include "result.h"
#include "tim.h"
#include "string.h"

//!NOTE: this is assuming that all steppers are the same stepper and thus have the same specs
#define STEPPER_PWM_TIMER_TICK_HZ 1000000U //Amt of Herz we calculate with after setting the prescaler (should be 1Mhz)
#define STEP_PULSE_MIN_WIDTH_TICKS 3U // 3 us at the 1 MHz timer tick rate
#define RPM 100 //Rotations per minute
#define STEPS_PER_REV 200 //The amount of steps that makes it turn 360 degrees
#define DEGREES_PER_STEP (360/STEPS_PER_REV) //At 200, 1 step turns the motor 1.8 degrees

//1:1 recration of protobuf
typedef struct {
    uint8_t duty_cycle; //Value of 1-100%
    uint8_t current_angle; //Value from 0-199 (in steps)
    TIM_HandleTypeDef* htim; //The timer that is used
    uint32_t frequency_hz; //The frequency we want for the stepper (Hertz)
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


result_t init_stepper(stepper_t* stepper, uint8_t duty_cycle, TIM_HandleTypeDef* htim);

void do_pwm_dma(stepper_t* stepper, int amt_steps, uint32_t freq);

void rotate_stepper(stepper_t* stepper, uint8_t target_angle_absolute, uint32_t freq);