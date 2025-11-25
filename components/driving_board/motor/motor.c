#include "logging.h"
#include <stdint.h>
#include "result.h"
#include "motor.h"



static MotorInfoCallback_t info_callback = 0;



//init these in shared data for motor init just have whts different for each motor
result_t motor_init(motor_t *motors){//I want to put the result type from logging instead for the return

   for (int i = 0; i < NUM_MOTORS; i++) {
       motor_t *motor = &motors[i];
       motor->motor_id = i;
       motor-> distance_to_go = 0;
       motor->turning_radius = 0;
       motor->turning_angle = 0;
   }
   return RESULT_OK;
}//this logic would change if there was a thread running for each motor

result_t motor_update(motor_t *motors, int index,
                 float distance_to_go,
                 float turning_radius,
                 float turning_angle) {//must pass in a pointer to a motor struct


   if (index < 0 || index >= NUM_MOTORS) {
       //LOGI("Motor", "index out of bounds");
       return RESULT_ERR_INVALID_ARG;
   }
   motor_t *motor = &motors[index];
   motor->distance_to_go= distance_to_go;
   motor->turning_radius = turning_radius;
   motor->turning_angle = turning_angle;
   return RESULT_OK;
}


//IMPORTANT: Actually we might not need a callback function for diagnostics since they are periodical but it might be useful for sending progress after a robot reaches a certain point.
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