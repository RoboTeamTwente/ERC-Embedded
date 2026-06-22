/**
 * @file C5E209.c
 * @brief Nanotec C5-E-2-09 driver: Plug & Drive Interface (PDI) over CANopen.
 *
 * See C5E209.h for the object map and command/status reference.
 * All transfers use expedited SDO over FDCAN classic frames.
 */

#include <stddef.h>
#include "C5E209.h"

/* CANopen SDO command specifiers (client -> server, expedited download). */
#define SDO_DL_1BYTE   0x2F
#define SDO_DL_2BYTE   0x2B
#define SDO_DL_4BYTE   0x23
#define SDO_UL_REQ     0x40   /* upload (read) request                  */
#define SDO_UL_MASK    0x40   /* server upload response has bit6 = scs   */

#define SDO_TX_COB_BASE  0x600  /* request:  client -> node (RX-SDO)      */
#define SDO_RX_COB_BASE  0x580  /* response: node -> client (TX-SDO)      */
#define SDO_COB_MASK     0x780

c5e209_information g_c5e209_info[C5E209_MAX_MOTORS] = {0};

/* Find the info slot for a node, allocating a free slot on first sight. */
static c5e209_information* c5e209_slot(uint8_t node_id) {
    for (int i = 0; i < C5E209_MAX_MOTORS; i++) {
        if (g_c5e209_info[i].node_id == node_id) {
            return &g_c5e209_info[i];
        }
    }
    for (int i = 0; i < C5E209_MAX_MOTORS; i++) {
        if (g_c5e209_info[i].node_id == 0) {
            g_c5e209_info[i].node_id = node_id;
            return &g_c5e209_info[i];
        }
    }
    return NULL;
}

static result_t c5e209_send(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                            const uint8_t data[8]) {
    if (can_handler == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    FDCAN_TxHeaderTypeDef tx_header = {0};
    tx_header.Identifier          = SDO_TX_COB_BASE + controller_id;
    tx_header.IdType              = FDCAN_STANDARD_ID;
    tx_header.TxFrameType         = FDCAN_DATA_FRAME;
    tx_header.DataLength          = FDCAN_DLC_BYTES_8;
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch       = FDCAN_BRS_OFF;
    tx_header.FDFormat            = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker       = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(can_handler, &tx_header, (uint8_t*)data) != HAL_OK) {
        return RESULT_ERR_COMMS;
    }
    return RESULT_OK;
}

result_t C5E209_sdo_write(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                          uint16_t index, uint8_t subindex,
                          int32_t value, uint8_t size) {
    uint8_t ccs;
    switch (size) {
        case 1:  ccs = SDO_DL_1BYTE; break;
        case 2:  ccs = SDO_DL_2BYTE; break;
        case 4:  ccs = SDO_DL_4BYTE; break;
        default: return RESULT_ERR_INVALID_ARG;
    }

    uint8_t data[8] = {
        ccs,
        (uint8_t)(index & 0xFF),
        (uint8_t)(index >> 8),
        subindex,
        (uint8_t)(value),
        (uint8_t)(value >> 8),
        (uint8_t)(value >> 16),
        (uint8_t)(value >> 24),
    };
    return c5e209_send(can_handler, controller_id, data);
}

result_t C5E209_sdo_read(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                         uint16_t index, uint8_t subindex) {
    uint8_t data[8] = {
        SDO_UL_REQ,
        (uint8_t)(index & 0xFF),
        (uint8_t)(index >> 8),
        subindex,
        0, 0, 0, 0,
    };
    return c5e209_send(can_handler, controller_id, data);
}

/* ---- PDI lifecycle -------------------------------------------------------- */

result_t C5E209_activate(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id) {
    /* 2290h:00h bit0 = 1 -> PDI active (UNSIGNED08). */
    return C5E209_sdo_write(can_handler, controller_id, C5E209_OD_PDI_CTRL, 0x00, 1, 1);
}

/* ---- Motion commands ------------------------------------------------------ */

result_t C5E209_set_position_speed(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                                   float position_degrees, int16_t speed_rpm) {
    int32_t target = (int32_t)(position_degrees * C5E209_POS_UNITS_PER_DEG);

    /* PDI-SetValue1 = target position (S32). */
    TRY(C5E209_sdo_write(can_handler, controller_id,
                         C5E209_OD_PDI_SETVALUE, 0x01, target, 4));
    /* PDI-SetValue2 = max speed (S16). */
    TRY(C5E209_sdo_write(can_handler, controller_id,
                         C5E209_OD_PDI_SETVALUE, 0x02, speed_rpm, 2));
    /* PDI-Cmd = 20 (Profile Position absolute, S08). */
    TRY(C5E209_sdo_write(can_handler, controller_id,
                         C5E209_OD_PDI_SETVALUE, 0x04, C5E209_CMD_PROFILE_POS_ABS, 1));
    return RESULT_OK;
}

result_t C5E209_set_position(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                             float position_degrees) {
    return C5E209_set_position_speed(can_handler, controller_id,
                                     position_degrees, C5E209_DEFAULT_SPEED_RPM);
}

result_t C5E209_set_speed(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                          float speed_rpm) {
    /* PDI-SetValue1 = target speed (S32). */
    TRY(C5E209_sdo_write(can_handler, controller_id,
                         C5E209_OD_PDI_SETVALUE, 0x01, (int32_t)speed_rpm, 4));
    /* PDI-Cmd = 23 (Profile Velocity, S08). */
    TRY(C5E209_sdo_write(can_handler, controller_id,
                         C5E209_OD_PDI_SETVALUE, 0x04, C5E209_CMD_PROFILE_VELOCITY, 1));
    return RESULT_OK;
}

result_t C5E209_set_torque(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                           int32_t torque_per_mille, int16_t speed_rpm) {
    /* PDI-SetValue2 = max speed (S16). */
    TRY(C5E209_sdo_write(can_handler, controller_id,
                         C5E209_OD_PDI_SETVALUE, 0x02, speed_rpm, 2));
    /* PDI-SetValue1 = target torque, thousandths of rated (S32). */
    TRY(C5E209_sdo_write(can_handler, controller_id,
                         C5E209_OD_PDI_SETVALUE, 0x01, torque_per_mille, 4));
    /* PDI-Cmd = 25 (Profile Torque, S08). */
    TRY(C5E209_sdo_write(can_handler, controller_id,
                         C5E209_OD_PDI_SETVALUE, 0x04, C5E209_CMD_PROFILE_TORQUE, 1));
    return RESULT_OK;
}

/* ---- Control / fault handling --------------------------------------------- */

result_t C5E209_switch_off(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id) {
    return C5E209_sdo_write(can_handler, controller_id,
                            C5E209_OD_PDI_SETVALUE, 0x04, C5E209_CMD_SWITCH_OFF, 1);
}

result_t C5E209_quickstop(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id) {
    return C5E209_sdo_write(can_handler, controller_id,
                            C5E209_OD_PDI_SETVALUE, 0x04, C5E209_CMD_QUICKSTOP, 1);
}

result_t C5E209_halt(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                     c5e209_cmd_t base_cmd) {
    int32_t cmd = (int32_t)base_cmd | C5E209_CMD_HALT_BIT;
    return C5E209_sdo_write(can_handler, controller_id,
                            C5E209_OD_PDI_SETVALUE, 0x04, cmd, 1);
}

result_t C5E209_clear_error(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id) {
    return C5E209_sdo_write(can_handler, controller_id,
                            C5E209_OD_PDI_SETVALUE, 0x04, C5E209_CMD_CLEAR_ERROR, 1);
}

/* ---- Status --------------------------------------------------------------- */

result_t C5E209_request_status(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id) {
    return C5E209_sdo_read(can_handler, controller_id, C5E209_OD_PDI_STATUS, 0x01);
}

void C5E209_parse_can_message(const FDCAN_RxHeaderTypeDef* rx_header,
                              const uint8_t data[8]) {
    if (rx_header == NULL || data == NULL) {
        return;
    }

    uint16_t cob_id = (uint16_t)rx_header->Identifier;
    if ((cob_id & SDO_COB_MASK) != SDO_RX_COB_BASE) {
        return;  /* not an SDO response */
    }

    uint8_t node_id = (uint8_t)(cob_id - SDO_RX_COB_BASE);
    uint16_t index  = (uint16_t)(data[1] | (data[2] << 8));
    uint8_t subindex = data[3];

    if (index != C5E209_OD_PDI_STATUS) {
        return;  /* only PDI output object is cached here */
    }

    c5e209_information* slot = c5e209_slot(node_id);
    if (slot == NULL) {
        return;
    }

    if (subindex == 0x01) {            /* PDI-Status (S16) */
        slot->pdi_status = (int16_t)(data[4] | (data[5] << 8));
    } else if (subindex == 0x02) {     /* PDI-ReturnValue (S32) */
        slot->return_value = (int32_t)((uint32_t)data[4]
                                       | ((uint32_t)data[5] << 8)
                                       | ((uint32_t)data[6] << 16)
                                       | ((uint32_t)data[7] << 24));
    }
}

int16_t C5E209_get_status(uint8_t motor_index) {
    if (motor_index >= C5E209_MAX_MOTORS) {
        return 0;
    }
    return g_c5e209_info[motor_index].pdi_status;
}
