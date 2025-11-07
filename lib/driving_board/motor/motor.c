#include "logging.h"
#include <stdint.h>
#include "motor.h"


static MotorInfoCallback_t info_callback = 0;
motor_t motors[NUM_MOTORS];

void motor_init(void){//I want to put the error type from logging instead

    for (int i = 0; i < NUM_MOTORS; i++) {
        motors[i].voltage = 0.0f;
        motors[i].angle_of_body_frame = 0.0f;
        motors[i].angular_momentum = 0.0f;
        motors[i].current = 0.0f;
        motors[i].rpm = 0.0f;
        motors[i].direction_vector_x = 0.0f;
        motors[i].direction_vector_y = 0.0f;
    }
}//this logic would change if there was a thread running for each motor

void motor_update(int index,
                  float voltage,
                  float angle_of_body_frame,
                  float angular_momentum,
                  float current,
                  float rpm,
                  float direction_vector_x,
                  float direction_vector_y) {

    if (index < 0 || index >= NUM_MOTORS) {
        //LOGI("Motor", "index out of bounds");
        return; 
    }

    motors[index].voltage = voltage;
    motors[index].angle_of_body_frame = angle_of_body_frame;
    motors[index].angular_momentum = angular_momentum;
    motors[index].current = current;
    motors[index].rpm = rpm;
    motors[index].direction_vector_x = direction_vector_x;
    motors[index].direction_vector_y = direction_vector_y;
}


//TODO: callback function here for control to call for diagnostics.

//currently stub implementatiton
void Motor_RegisterCallback(MotorInfoCallback_t cb) {//called in main
    info_callback = cb;
}

void Motor_UpdateDiagnostics(const DrivingBoardMotorInformation_t *data) {
    if (info_callback) {
        info_callback(data);
    }
}

void MotorInfoHandler(const DrivingBoardMotorInformation_t *data) {
    //I would start the parsing&sending process of diagnostic data here
}


//in main I would call Motor_RegisterCallback(MotorInfoHandler);



//Motor_UpdateDiagnostics(&motor_info); would be called by control
//motor_info is the info struct with 4 values defined in h file