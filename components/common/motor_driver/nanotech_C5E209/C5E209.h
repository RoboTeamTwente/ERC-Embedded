#ifndef NANOTECH_BASE_MOTOR
#define NANOTECH_BASE_MOTOR

#include <stdint.h>
#include "fdcan.h"
#include "result.h"

result_t C5E209_set_position(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id, float position_degrees);

result_t C5E209_set_speed(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id, float speed_rpm);

#endif
