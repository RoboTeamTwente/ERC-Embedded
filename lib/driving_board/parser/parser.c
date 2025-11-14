//logic fr getting the type of message from envelope

//will use that type in the future right now it assumes I got drivingboard msg

#include "pb_message.h"
#include <stdint.h>


DrivingBoardTest message = DrivingBoardTest_init_zero;
  message.a = 2 * n;
  message.b = 2 * n + 1;
  pb_encoding_t encoding = pb_message_encode((void *)&message, MainBoardTest_fields);
  if (encoding.result != RESULT_OK) {
    LOGE(TAG, "Encoding error: %s", result_to_short_str(encoding.result));
    continue;
  }
//rn DrivingBoardMotorMsg can come from test