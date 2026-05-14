#include "cubemars_ak.h"

#include "result.h"

inline uint32_t cubemars_ak_get_can_id(uint8_t _controler_id,
                                       cubemars_ak_message_type _type) {
    return _controler_id | ((uint32_t)_type << 8);
}

static int16_t cubemars_get_i16_be(const uint8_t* buf, size_t* idx) {
    uint16_t value = ((uint16_t)buf[*idx] << 8) | ((uint16_t)buf[*idx + 1]);

    *idx += 2;

    return (int16_t)value;
}

static int32_t cubemars_get_i32_be(const uint8_t* buf, size_t* idx) {
    uint32_t value = ((uint32_t)buf[*idx] << 24) |
                     ((uint32_t)buf[*idx + 1] << 16) |
                     ((uint32_t)buf[*idx + 2] << 8) | ((uint32_t)buf[*idx + 3]);

    *idx += 4;

    return (int32_t)value;
}

result_t cubemars_ak_parse_can_feedback(const FDCAN_RxHeaderTypeDef* rx_header,
                                        const uint8_t data[8],
                                        cubemars_ak_information* out) {
    if (rx_header == NULL || data == NULL || out == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    if (rx_header->IdType != FDCAN_EXTENDED_ID) {
        return RESULT_ERR_BAD_FORMAT;
    }

    if (rx_header->DataLength != FDCAN_DLC_BYTES_8) {
        return RESULT_ERR_BAD_FORMAT;
    }

    uint8_t motor_id = (uint8_t)(rx_header->Identifier & 0xFFu);
    uint32_t function_id = rx_header->Identifier >> 8;

    if (function_id != CUBEMARS_AK_CAN_SERVO_FEEDBACK_ID) {
        return RESULT_ERR_BAD_FORMAT;
    }

    out->motor_id = motor_id;
    out->motor_position = (int16_t)*(&data[0]) / CUBEMARS_AK_CAN_POSITION_SCALE;
    out->motor_speed = (int16_t)*(&data[2]) / CUBEMARS_AK_CAN_SPEED_SCALE;
    out->motor_current = (int16_t)*(&data[4]) / CUBEMARS_AK_CAN_CURRENT_SCALE;
    out->motor_temperature =
        (int8_t)data[6] / CUBEMARS_AK_CAN_TEMPERATURE_SCALE;
    out->status_code = (cubemars_ak_error_code)data[7];

    return RESULT_OK;
}
