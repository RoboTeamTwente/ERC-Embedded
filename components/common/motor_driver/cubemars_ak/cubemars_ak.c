#include "cubemars_ak.h"

#include "result.h"

static FDCAN_TxHeaderTypeDef tx_header = {
    .Identifier = 0,
    .IdType = FDCAN_EXTENDED_ID,
    .TxFrameType = FDCAN_DATA_FRAME,
    .DataLength = FDCAN_DLC_BYTES_4,
    .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
    .BitRateSwitch = FDCAN_BRS_OFF,
    .FDFormat = FDCAN_CLASSIC_CAN,
    .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
    .MessageMarker = 0,
};

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
    size_t index = 0;
    out->motor_current = cubemars_get_i16_be(data, &index);
    out->motor_position = (int16_t)*(data) / CUBEMARS_AK_CAN_POSITION_SCALE;
    out->motor_speed = (int16_t)*(data + 2) / CUBEMARS_AK_CAN_SPEED_SCALE;
    out->motor_current = (int16_t)*(data + 4) / CUBEMARS_AK_CAN_CURRENT_SCALE;
    out->motor_temperature =
        (int8_t)data[6] / CUBEMARS_AK_CAN_TEMPERATURE_SCALE;
    out->status_code = (cubemars_ak_error_code)data[7];

    return RESULT_OK;
}

result_t cubemars_ak_set_speed(FDCAN_HandleTypeDef* can_handler,
                               uint8_t controller_id, int32_t erpm) {
    if (can_handler == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    tx_header.Identifier =
        cubemars_ak_get_can_id(controller_id, CUBEMARS_AK_SET_RPM);
    tx_header.DataLength = FDCAN_DLC_BYTES_4;

    erpm = (int32_t)__REV(
        (uint32_t)
            erpm);  // You can hate me for this, but its funky and I like it

    if (HAL_FDCAN_AddMessageToTxFifoQ(can_handler, &tx_header,
                                      (uint8_t*)&erpm) != HAL_OK) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}

result_t cubemars_ak_set_position(FDCAN_HandleTypeDef* can_handler,
                                  uint8_t controller_id, float position_deg) {
    if (can_handler == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    int32_t position_raw =
        (int32_t)(position_deg * CUBEMARS_AK_MOTOR_POSITION_SCALE);

    tx_header.Identifier =
        cubemars_ak_get_can_id(controller_id, CUBEMARS_AK_SET_POSITION);
    tx_header.DataLength = FDCAN_DLC_BYTES_4;

    position_raw =
        (int32_t)__REV((uint32_t)position_raw);  // You can hate me for this,
                                                 // but its funky and I like it

    if (HAL_FDCAN_AddMessageToTxFifoQ(can_handler, &tx_header,
                                      (uint8_t*)&position_raw) != HAL_OK) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}

result_t cubemars_ak_set_origin(FDCAN_HandleTypeDef* can_handler,
                                uint8_t controller_id, uint8_t origin_mode) {
    if (can_handler == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    if (origin_mode > CUBEMARS_AK_ORIGIN_PERMANENT) {
        return RESULT_ERR_INVALID_ARG;
    }

    tx_header.Identifier = cubemars_ak_get_can_id(
        controller_id, CUBEMARS_AK_CAN_PACKET_SET_ORIGIN_HERE);
    tx_header.DataLength = FDCAN_DLC_BYTES_1;

    if (HAL_FDCAN_AddMessageToTxFifoQ(can_handler, &tx_header, &origin_mode) !=
        HAL_OK) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}
