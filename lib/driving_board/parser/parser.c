//logic fr getting the type of message from envelope

//will use that type in the future right now it assumes I got drivingboard msg
#include "logging.h"
#include "string.h"
#include "driving_board.pb.h"
#include "pb_message.h"
#include <stdint.h>

result_t DBMMsgEncode(float distance_to_go, float turning_angle, float turning_radius, pb_encoding_t* encoding_out){//pointer passing for the result encoding_out
  if (turning_angle > 360){
    LOGE(TAG, "Angle value redundant");
    return RESULT_ERR_INVALID_ARG;
  }
  if (turning_radius < 0){
    LOGE(TAG, "turning radius can't be negative");
    return RESULT_ERR_INVALID_ARG;
  }
  if (distance_to_go < 0){
    LOGE(TAG, "distance to go can't be negative");
    return RESULT_ERR_INVALID_ARG;
  }
  
  DrivingBoardMotorMsg message = DrivingBoardMotorMsg_init_zero;
    message.distance_to_go = distance_to_go;
    message.turning_angle = turning_angle;
    message.turning_radius = turning_radius;
    *encoding_out = pb_message_encode((void *)&message, DrivingBoardMotorMsg_fields);
    if (encoding_out->result != RESULT_OK) {
      LOGE(TAG, "Encoding error: %s", result_to_short_str(encoding_out->result));
      return RESULT_FAIL;
  }
    else {
      LOGE(TAG, "message encoded successfully: %s", result_to_short_str(encoding_out->result));
  }
}
//rn DrivingBoardMotorMsg can come from test