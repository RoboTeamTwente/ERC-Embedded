#include "stdint.h"
#include "result.h"
#include "stepper.h"

//NOTE: placeholder values
#define PWM_FREQ 1000
#define STEPS_PER_REV 200 //In steps / rev
#define RPM 100 


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

result_t Control_Signals_Handler (control_signals_t signals) {

}

result_t rotate_stepper(uint32_t angle, bool direction) {
    angle = angle % 360;
    uint32_t steps_for_angle = STEPS_PER_REV * (angle / 360);
    
    set_pin("ENA", ">threshold"); //Set enable pin volt. high enough to enable stepper
    set_pin("DIR", direction); //1 for clockwise, 0 for counterclockwise
    
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

void delay_by_rpm() {
    int ms_in_minute = 60000000;
    delay(ms_in_minute/STEPS_PER_REV/RPM);
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
