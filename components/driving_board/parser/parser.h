#include <result.h>
#include <pb_message.h>
#ifndef PARSER_H
#define PARSER_H

result_t DBMMsgEncode(float distance_to_go, float turning_angle, float turning_radius, pb_encoding_t* encoding_out);//should be decode

result_t DBMPProgressEncode(float distance_left, pb_encoding_t* encoding_out);

result_t DBMDReachedNotificationEncode(float distance_reached, pb_encoding_t* encoding_out);
#endif 