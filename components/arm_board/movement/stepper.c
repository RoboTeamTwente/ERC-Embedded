#include "stdint.h"
#include "result.h"
#include "stepper.h"


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

//NOTE: placeholder values
#define PWM_FREQ 1000
#define STEPS_PER_REV 200 //In steps / rev
#define RPM 100 

//The angles from where to where the stepper is allowed to move
//For example, the wrist could probably not move fully where the arm is
const uint32_t START_ANGLE_CW = 315; //Place in the CW system where the stepper can only move forwards
const uint32_t START_ANGLE_CW = 270; //Place in the CW system where the stepper can only move backwards
const uint32_t START_ANGLE_CCW = START_ANGLE_CW % 360;
const uint32_t STOP_ANGLE_CCW = START_ANGLE_CW % 360;



const uint32_t WAVE_DRIVE[4][4] = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};

const uint32_t HALF_DRIVE[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

const uint32_t FULL_DRIVE[4][4] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1},
};


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

result_t init_stepper() {
    set_pin(ENA_PIN, "HIGH");
    stepper.current_angle = 0;
    stepper.dir = 1; 
    stepper.pwm = 0;
}

result_t control_signals_handler (control_signals_t signals) {

}

result_t stepper_make_steps(uint32_t angle, bool direction) {
    angle = angle % 360;
    uint32_t steps_for_angle = STEPS_PER_REV * (angle / 360);
    
    set_pin(DIR_PIN, direction); //1 for clockwise, 0 for counterclockwise
    
    int seq_len = len(HALF_DRIVE);
    for (int x = 0; x < steps_for_angle; x++) {
        int step_index;
        if (direction == 1){
            step_index = x % seq_len;
        } else if (direction == 0) { //if dir is counterclockwise, run through the sequence backwards
            step_index = (seq_len - (x % seq_len)) % seq_len;
        } else {
            //THROW ERROR 
        }
        sequence(step_index);
        delay_by_rpm();
    }
}

result_t rotate_stepper(uint32_t target_angle) { //Target angle is in CW system
    uint32_t current_angle = stepper.current_angle;
    uint32_t CW_angle = target_angle - current_angle;
    uint32_t CCW_angle = 360 - CW_angle;

    if (CW_angle < CCW_angle) {
        stepper_make_steps(CW_angle, 1);
    } else if (CCW_angle < CW_angle) {
        stepper_make_steps(CW_angle, 0);
    }
    stepper.current_angle = target_angle;
}

//placeholder 
void delayMicroseconds (uint32_t ms){
//   __HAL_TIM_SET_COUNTER(&htim1, 0);
//   while (__HAL_TIM_GET_COUNTER(&htim1) < ms);
    return;
}

void delay_by_rpm() {
    int ms_in_minute = 60000000;
    delayMicroseconds(ms_in_minute/STEPS_PER_REV/RPM);
}

//NOTE: for sequence half drive
void sequence(int step){
    set_pin("GPIO1", HALF_DRIVE[step][0]);
    set_pin("GPIO2", HALF_DRIVE[step][1]);
    set_pin("GPIO3", HALF_DRIVE[step][2]);
    set_pin("GPIO4", HALF_DRIVE[step][3]);
}

//NOTE: temporary method, will be replaced by HAL code
void set_pin(pinname, what) {
    return;
}
