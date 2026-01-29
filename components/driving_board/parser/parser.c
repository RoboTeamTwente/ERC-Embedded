/**
 * //logic fr getting the type of message from envelope

//will use that type in the future right now it assumes I got drivingboard msg
#include "logging.h"
#include "string.h"
#include "driving_board.pb.h"
#include "pb_message.h"
#include <stdint.h>
#include "logging.h"

static char *TAG = "MAIN";


result_t pb_message_encode(const void *src_struct, const pb_field_t fields[],
                           uint8_t **out_data, size_t *out_length);

//you can just extern float values since we wont free that memory and it will keep getting updated
result_t DBMMsgEncode(float distance_to_go, float turning_angle, float turning_radius, uint8_t* out_data){//pointer passing for the result encoding_out
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
   result_t res= pb_message_encode((void *)&message, DrivingBoardMotorMsg_fields, **out_data, *out_length);


   if (res != RESULT_OK) {
     LOGE(TAG, "Encoding error: %s", res);
     return RESULT_FAIL;
 }
   else {
     LOGE(TAG, "message encoded successfully: %s", res);
     return RESULT_OK;
 }
}
//rn DrivingBoardMotorMsg can come from test

result_t DBMPProgressEncode(float distance_left, uint8_t* out_data){//pointer passing for the result encoding_out

 if (distance_left < 0){
   LOGE(TAG, "distance left can't be negative: %f", distance_left);
   return RESULT_ERR_INVALID_ARG;
 }
  DrivingBoardMotorPeriodicProgress message = DrivingBoardMotorPeriodicProgress_init_zero;
   message.distance_left =distance_left;
   result_t res = pb_message_encode((void *)&message, DrivingBoardMotorPeriodicProgress_fields, **out_data, *out_length);

   if (res != RESULT_OK) {
     LOGE(TAG, "Encoding error: %s", res);
     return RESULT_FAIL;
 }
   else {
     LOGE(TAG, "message encoded successfully: %s", res);
     return RESULT_OK;
 }
}

result_t DBMDReachedNotificationEncode(float distance_reached, uint8_t* out_data){//pointer passing for the result encoding_out

 if (distance_reached != 0 && distance_reached != 1){
   LOGE(TAG, "distance reached cannot be a number except 0 or 1: %f", distance_reached);
   return RESULT_ERR_INVALID_ARG;
 }
  DrivingBoardMotorDistanceReachedNotification message = DrivingBoardMotorDistanceReachedNotification_init_zero;
   message.distance_reached = distance_reached;
   result_t res = pb_message_encode((void *)&message, DrivingBoardMotorDistanceReachedNotification_fields, **out_data, *out_length);
   if (res != RESULT_OK) {
     LOGE(TAG, "Encoding error: %s", res);
     return RESULT_FAIL;
 }
   else {
     LOGE(TAG, "message encoded successfully: %s", res);
     return RESULT_OK;
 }
}



//diagnostic parser is going to be a bit bigger

 */
