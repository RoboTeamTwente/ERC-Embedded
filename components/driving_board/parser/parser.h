#include <result.h>
#include <pb_message.h>
#ifndef PARSER_H
#define PARSER_H

result_t DBMMsgEncode(float distance_to_go, float turning_angle, float turning_radius, uint8_t* out_data);//should be decode

result_t DBMPProgressEncode(float distance_left, uint8_t* out_data);

result_t DBMDReachedNotificationEncode(float distance_reached, uint8_t* out_data);
#endif 