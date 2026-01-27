#include <stdbool.h>

typedef struct {
    float current_x_pos;
    float current_y_pos;
    float current_z_pos;
    float rotation;
    bool enable;
    bool dir;
    float pwm;
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