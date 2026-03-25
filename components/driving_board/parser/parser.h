#include <result.h>
#include <pb_message.h>
#include "diagnostics.pb.h"
#include "motor_information.pb.h"
#ifndef PARSER_H
#define PARSER_H

result_t DBMDiagnosticsEncode(const DiagnosticsData *diag, uint8_t **out_data, size_t *out_length);

typedef struct {
    uint8_t state;
    int32_t motor_id;
    float rpm;
    float voltage;
    float encoder_angle;
} MotorDiagnostic;

typedef struct {
    uint8_t board_state;
    MotorDiagnostic motors[10];
} DiagnosticsData;

#endif 