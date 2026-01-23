#include "logging.h"
#include <stdint.h>
#include "result.h"
#include "motor.h"


static MotorInfoCallback_t info_callback = 0;

static char *TAG = "MOTOR";
//init these in shared data for motor init just have whts different for each motor
result_t driving_system_init(driving_system_t *driving_system){//I want to put the result type from logging instead for the return  
   driving_system-> distance_to_go = 0;
   driving_system->turning_radius = 0;
   driving_system->turning_angle = 0;
   driving_system->state = 0;
   return RESULT_OK;
}//this logic would change if there was a thread running for each motor

result_t motor_speed_init(motor_speed_t *motors){//I want to put the result type from logging instead for the return

   for (int i = 0; i < NUM_MOTORS_SPEED; i++) {
       motor_speed_t *motor = &motors[i];
       motor->motor_id = i;
       motor-> actspeed = 0;
       motor->control_var = 0;
   }
   return RESULT_OK;
}

result_t motor_steering_init(motor_steering_t *motors){//I want to put the result type from logging instead for the return

   for (int i = 0; i < NUM_MOTORS_STEERING; i++) {
       motor_steering_t *motor = &motors[i];
       motor->motor_id = i;
       motor->  actangle= 0;
       motor->desang = 0;
       motor->pwnenable = 0;
       motor->pwmrev = 0;
   }
   return RESULT_OK;
}

result_t driving_system_update(driving_system_t *driving_system,
                 float distance_to_go,
                 float turning_radius,
                 float turning_angle, float state) {//must pass in a pointer to a motor struct

   driving_system->distance_to_go= distance_to_go;
   driving_system->turning_radius = turning_radius;
   driving_system->turning_angle = turning_angle;
   driving_system->state = state;
   return RESULT_OK;
}

result_t motor_steering_update(motor_steering_t *motors, int index,
                 float actangle,
                 float desang,
                 float pwnenable, float pwmrev) {//must pass in a pointer to a motor struct


   if (index < 0 || index >= NUM_MOTORS_SPEED) {
       LOGE(TAG, "Index: %d", index);
       return RESULT_ERR_INVALID_ARG;
   }
   motor_steering_t *motor = &motors[index];
   motor->actangle= actangle;
   motor->desang = desang;
   motor->pwnenable = pwnenable;
   motor->pwmrev = pwmrev;
   return RESULT_OK;
}


result_t motor_speed_update(motor_speed_t *motors, int index,
                 float actspeed,
                 float controlvar
                 ) {//must pass in a pointer to a motor struct


   if (index < 0 || index >= NUM_MOTORS_SPEED) {
       LOGE(TAG, "Index: %d", index);
       return RESULT_ERR_INVALID_ARG;
   }
   motor_speed_t *motor = &motors[index];
   motor->actspeed= actspeed;
   motor->control_var = controlvar;
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