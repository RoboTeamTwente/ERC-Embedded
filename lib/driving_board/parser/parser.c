//logic fr getting the type of message from envelope

//will use that type in the future right now it assumes I got drivingboard msg

#include "pb_decode.h"//nanopb decoder
#include <stdint.h>

uint8_t buffer[BUFFER_SIZE];

//rn DrivingBoardMotorMsg can come from test