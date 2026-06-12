#include "cubemars_ak.h"

#include <string.h>

#include "logging.h"
#include "result.h"

const static char* TAG = "Cubemars AK";

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

void cubemars_ak_print_feedback(cubemars_ak_information* info) {
    LOGI(TAG,
         "AK info: id=%u pos=%d(0.1deg) speed=%d(10erpm) current=%d(0.01A) "
         "temp=%dC status=%d",
         (unsigned)info->motor_id, (int)info->motor_position,
         (int)info->motor_speed, (int)info->motor_current,
         (int)info->motor_temperature, (int)info->status_code);
}

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
    LOGI(TAG, "Feedback; ID: %d, A: %d, Pos: %d, Speed: %d, Temp: %d",
         out->motor_id, out->motor_current, out->motor_position,
         out->motor_speed, out->motor_temperature);

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
        LOGE(TAG, "Did not send can message");
        return RESULT_FAIL;
    }
    LOGI(TAG, "Sent speed command to: %04x", tx_header.Identifier);

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

result_t cubemars_ak_set_brake_current(FDCAN_HandleTypeDef* can_handler,
                                       uint8_t controller_id, float current_a) {
    if (can_handler == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    if (current_a < 0.0f || current_a > CUBEMARS_AK_MAX_BRAKE_CURRENT_A) {
        return RESULT_ERR_INVALID_ARG;
    }

    int32_t current_raw =
        (int32_t)(current_a * CUBEMARS_AK_BRAKE_CURRENT_SCALE);

    tx_header.Identifier =
        cubemars_ak_get_can_id(controller_id, CUBEMARS_AK_SET_CURRENT_BRAKE);
    tx_header.DataLength = FDCAN_DLC_BYTES_4;

    current_raw = (int32_t)__REV((uint32_t)current_raw);

    if (HAL_FDCAN_AddMessageToTxFifoQ(can_handler, &tx_header,
                                      (uint8_t*)&current_raw) != HAL_OK) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}

#define CUBEMARS_AK_UART_HEADER 0x02u
#define CUBEMARS_AK_UART_TAIL 0x03u

#define CUBEMARS_AK_COMM_GET_VALUES_SETUP 0x32u

#define CUBEMARS_AK_UART_MOTOR_ID_RESPONSE_LEN 11u
#define CUBEMARS_AK_UART_TIMEOUT_MS 500u

static uint16_t cubemars_ak_crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0u;
    uint8_t bit;
    for (size_t i = 0u; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;

        for (bit = 0u; bit < 8u; bit++) {
            if ((crc & 0x8000u) != 0u) {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

static result_t cubemars_ak_uart_send_motor_id_request(
    UART_HandleTypeDef* uart_handler) {
    if (uart_handler == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    uint8_t request[10] = {
        CUBEMARS_AK_UART_HEADER,
        0x05u,
        CUBEMARS_AK_COMM_GET_VALUES_SETUP,
        0x00u,
        0x02u,
        0x00u,
        0x00u,
        0x00u,
        0x00u,
        CUBEMARS_AK_UART_TAIL,
    };

    uint16_t crc = cubemars_ak_crc16(&request[2], 5u);

    request[7] = (uint8_t)(crc >> 8);
    request[8] = (uint8_t)crc;

    if (HAL_UART_Transmit(uart_handler, request, sizeof(request),
                          CUBEMARS_AK_UART_TIMEOUT_MS) != HAL_OK) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}

static result_t cubemars_ak_uart_receive_motor_id_response(
    UART_HandleTypeDef* uart_handler, uint8_t* rx) {
    if (uart_handler == NULL || rx == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    do {
        if (HAL_UART_Receive(uart_handler, &rx[0], 1u,
                             CUBEMARS_AK_UART_TIMEOUT_MS) != HAL_OK) {
            return RESULT_FAIL;
        }
    } while (rx[0] != CUBEMARS_AK_UART_HEADER);

    if (HAL_UART_Receive(uart_handler, &rx[1],
                         CUBEMARS_AK_UART_MOTOR_ID_RESPONSE_LEN - 1u,
                         CUBEMARS_AK_UART_TIMEOUT_MS) != HAL_OK) {
        return RESULT_FAIL;
    }

    return RESULT_OK;
}

static result_t cubemars_ak_uart_parse_motor_id_response(const uint8_t* rx,
                                                         uint8_t* motor_id) {
    if (rx == NULL || motor_id == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    if (rx[0] != CUBEMARS_AK_UART_HEADER || rx[10] != CUBEMARS_AK_UART_TAIL) {
        return RESULT_ERR_BAD_FORMAT;
    }

    if (rx[1] != 0x06u) {
        return RESULT_ERR_BAD_FORMAT;
    }

    if (rx[2] != CUBEMARS_AK_COMM_GET_VALUES_SETUP) {
        return RESULT_ERR_BAD_FORMAT;
    }

    uint32_t mask = ((uint32_t)rx[3] << 24) | ((uint32_t)rx[4] << 16) |
                    ((uint32_t)rx[5] << 8) | ((uint32_t)rx[6]);

    if (mask != CUBEMARS_AK_PARAM_MOTOR_ID_MASK) {
        return RESULT_ERR_BAD_FORMAT;
    }

    uint16_t received_crc = ((uint16_t)rx[8] << 8) | rx[9];
    uint16_t calculated_crc = cubemars_ak_crc16(&rx[2], rx[1]);

    if (received_crc != calculated_crc) {
        return RESULT_ERR_BAD_FORMAT;
    }

    *motor_id = rx[7];

    return RESULT_OK;
}

result_t cubemars_ak_uart_get_motor_id(UART_HandleTypeDef* uart_handler,
                                       uint8_t* motor_id) {
    if (uart_handler == NULL || motor_id == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    uint8_t rx[CUBEMARS_AK_UART_MOTOR_ID_RESPONSE_LEN];

    result_t result = cubemars_ak_uart_send_motor_id_request(uart_handler);

    if (result != RESULT_OK) {
        return result;
    }

    result = cubemars_ak_uart_receive_motor_id_response(uart_handler, rx);

    if (result != RESULT_OK) {
        return result;
    }

    return cubemars_ak_uart_parse_motor_id_response(rx, motor_id);
}
