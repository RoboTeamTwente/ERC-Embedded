//logic fr getting the type of message from envelope

//will use that type in the future right now it assumes I got drivingboard msg

/**
 * #include "logging.h"
#include "string.h"
#include "parser.h"
#include "diagnostics.pb.h"
#include "motor.pb.h"
#include "pb_message.h"
#include <stdint.h>
#include "logging.h"


static char *TAG = "MAIN";

 */


//(result_t pb_message_encode(const void *src_struct, const pb_field_t fields[],uint8_t **out_data, size_t *out_length);

//you can just extern float values since we wont free that memory and it will keep getting updated
/**
 * result_t DBMMsgEncode(float distance_to_go, float turning_angle, float turning_radius, uint8_t **out_data, size_t *out_length){//pointer passing for the result encoding_out
 if (turning_angle > 360){
   LOGE(TAG, "Angle value redundant: %f", turning_angle);
   return RESULT_ERR_INVALID_ARG;
 }
 if (turning_radius < 0){
   LOGE(TAG, "turning radius can't be negative: %f", turning_radius);
   return RESULT_ERR_INVALID_ARG;
 }
 if (distance_to_go < 0){
   LOGE(TAG, "distance to go can't be negative: %f", distance_to_go);
   return RESULT_ERR_INVALID_ARG;
 }
  DrivingBoardMotorMsg message = DrivingBoardMotorMsg_init_zero;
   message.distance_to_go = distance_to_go;
   message.turning_angle = turning_angle;
   message.turning_radius = turning_radius;
   result_t res= pb_message_encode((void *)&message, DrivingBoardMotorMsg_fields, out_data, out_length);


   if (res != RESULT_OK) {
     LOGE(TAG, "Encoding error: %d", res);
     return RESULT_FAIL;
 }
   else {
     LOGE(TAG, "message encoded successfully: %d", res);
     return RESULT_OK;
 }
}
 */
/**
 * void copy_motor_to_pb(MotorInformation *dst, const MotorDiagnostic *src)//gets the each motor diagnostic struct and puts it in motor information pb
{
    dst->state = (MotorInformation_State)src->state;//typecast motor info state
    dst->motor_id = src->motor_id;
    dst->rpm = src->rpm;
    dst->voltage = src->voltage;
    dst->encoder_angle = src->encoder_angle;
}

result_t DBMDiagnosticsEncode(const DiagnosticsData *diag, uint8_t **out_data, size_t *out_length){
  DrivingBoardDiagnostics msg = DrivingBoardDiagnostics_init_zero;
  msg.state = (DrivingBoardDiagnostics_State)diag->board_state; //typecast boardstate enum

  MotorInformation *pb_motors[] = {
    &msg.front_left_motor,
    &msg.middle_left_motor,
    &msg.back_left_motor,
    &msg.front_right_motor,
    &msg.middle_right_motor,
    &msg.back_right_motor,
    &msg.steering_front_left_motor,
    &msg.steering_back_left_motor,
    &msg.steering_front_right_motor,
    &msg.steering_back_right_motor
  };
  for (int i = 0; i < 10; i++) {
    copy_motor_to_pb(pb_motors[i], &diag->motors[i]);//writes all 10 motors into motorinfo pbs that are inside diagnostics pb
  }
  res = pb_message_encode(&msg, DrivingBoardDiagnostics_fields, out_data, out_length);
  if res != RESULT_OK{
    return RESULT_FAIL;
  }
  return RESULT_OK;
}


 */