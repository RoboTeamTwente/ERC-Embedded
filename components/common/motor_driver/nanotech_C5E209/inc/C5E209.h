#ifndef NANOTECH_C5E209_H
#define NANOTECH_C5E209_H

/**
 * @file C5E209.h
 * @brief Driver for the Nanotec C5-E-2-09 motor controller via the
 *        Plug & Drive Interface (PDI) over CANopen (SDO access).
 *
 * The PDI is a Nanotec-specific control layer that lets you trigger drive
 * commands directly, bypassing the CiA-402 state machine. It is mapped onto
 * three object-dictionary objects:
 *
 *   - 2290h:00h  PDI-Ctrl        UNSIGNED08  bit0 = 1 -> PDI active
 *   - 2291h:01h  PDI-SetValue1   SIGNED32    target value (position/speed/torque)
 *   - 2291h:02h  PDI-SetValue2   SIGNED16    secondary value (max speed)
 *   - 2291h:03h  PDI-SetValue3   SIGNED08    OD subindex / homing method
 *   - 2291h:04h  PDI-Cmd         SIGNED08    universal control command
 *   - 2292h:01h  PDI-Status      SIGNED16    status bitmask
 *   - 2292h:02h  PDI-ReturnValue SIGNED32    mode-dependent output / error code
 *
 * Each high-level call below pushes the required sequence of expedited SDO
 * downloads onto the FDCAN Tx FIFO (COB-ID 0x600 + node_id). They are
 * non-blocking: they do NOT wait for the SDO server response. Feed received
 * frames (COB-ID 0x580 + node_id) into C5E209_parse_can_message() to update the
 * cached PDI-Status / actual value.
 *
 * Ref: Nanotec "Functional Description Plug & Drive-Interface" v1.0.1.
 */

#include <stdint.h>
#include "fdcan.h"
#include "result.h"

#define C5E209_MAX_MOTORS 4

/* PDI object dictionary indices */
#define C5E209_OD_PDI_CTRL       0x2290
#define C5E209_OD_PDI_SETVALUE   0x2291  /* sub 01..04 */
#define C5E209_OD_PDI_STATUS     0x2292  /* sub 01 status, sub 02 return value */

/* PDI-Cmd values (object 2291h:04h) */
typedef enum {
    C5E209_CMD_NOP                 = 0,
    C5E209_CMD_SWITCH_OFF          = 1,
    C5E209_CMD_CLEAR_ERROR         = 2,
    C5E209_CMD_QUICKSTOP           = 3,
    C5E209_CMD_OD_READ             = 14,
    C5E209_CMD_OD_WRITE            = 15,
    C5E209_CMD_AUTO_SETUP          = 16,
    C5E209_CMD_HOMING_CUR_POS      = 17,
    C5E209_CMD_HOMING              = 18,
    C5E209_CMD_PROFILE_POS_ABS     = 20,
    C5E209_CMD_PROFILE_POS_REL     = 21,
    C5E209_CMD_PROFILE_VELOCITY    = 23,
    C5E209_CMD_PROFILE_TORQUE      = 25,
} c5e209_cmd_t;

#define C5E209_CMD_HALT_BIT    (1u << 6)  /* 64  - brake to 0, stay energized */
#define C5E209_CMD_TOGGLE_BIT  (1u << 7)  /* 128 - re-issue same command       */

/* PDI-Status bit masks (object 2292h:01h) */
#define C5E209_ST_OPERATION_ENABLED (1u << 0)
#define C5E209_ST_WARNING           (1u << 1)
#define C5E209_ST_FAULT             (1u << 2)
#define C5E209_ST_TARGET_REACHED    (1u << 3)
#define C5E209_ST_FOLLOWING_ERROR   (1u << 4)
#define C5E209_ST_LIMIT_REACHED     (1u << 5)
#define C5E209_ST_QS_HALT           (1u << 7)
#define C5E209_ST_HOMING_DONE       (1u << 11)
#define C5E209_ST_AUTOSETUP_DONE    (1u << 12)
#define C5E209_ST_TOGGLE_CMD        (1u << 14)
#define C5E209_ST_ERROR             (1u << 15)

/* Factory user-unit scaling: position is in tenths of a degree. */
#define C5E209_POS_UNITS_PER_DEG  10.0f

/* Default max profile speed (rpm) used by C5E209_set_position(). */
#ifndef C5E209_DEFAULT_SPEED_RPM
#define C5E209_DEFAULT_SPEED_RPM  100
#endif

typedef struct {
    uint8_t  node_id;        /* CANopen node id, 0 = slot free                */
    int16_t  pdi_status;     /* last PDI-Status (2292h:01h)                   */
    int32_t  return_value;   /* last PDI-ReturnValue (2292h:02h)              */
} c5e209_information;

extern c5e209_information g_c5e209_info[C5E209_MAX_MOTORS];

/* ---- PDI lifecycle -------------------------------------------------------- */

/** Activate the Plug & Drive interface (2290h:00h bit0 = 1). */
result_t C5E209_activate(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id);

/* ---- Motion commands ------------------------------------------------------ */

/**
 * Absolute profile-position move (PDI-Cmd 20) at C5E209_DEFAULT_SPEED_RPM.
 * @param position_degrees target position, degrees (scaled to tenths internally)
 * @note Signature kept stable for existing callers in arm_board firmware.
 */
result_t C5E209_set_position(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                             float position_degrees);

/**
 * Absolute profile-position move (PDI-Cmd 20) with explicit max speed.
 * @param position_degrees target position, degrees (scaled to tenths internally)
 * @param speed_rpm        max profile speed, rpm (SetValue2, 16-bit)
 */
result_t C5E209_set_position_speed(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                                   float position_degrees, int16_t speed_rpm);

/**
 * Profile-velocity mode (PDI-Cmd 23).
 * @param speed_rpm target speed, rpm (SetValue1)
 */
result_t C5E209_set_speed(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                          float speed_rpm);

/**
 * Profile-torque mode (PDI-Cmd 25).
 * @param torque_per_mille target torque in thousandths of rated torque (SetValue1)
 * @param speed_rpm        max speed, rpm (SetValue2)
 * @note Object 6087h (Torque Slope) must be configured first; default is 0.
 */
result_t C5E209_set_torque(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                           int32_t torque_per_mille, int16_t speed_rpm);

/* ---- Control / fault handling --------------------------------------------- */

/** Switch off power stage, de-energize motor (PDI-Cmd 1). */
result_t C5E209_switch_off(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id);

/** Quickstop: brake with quickstop ramp to 0 (PDI-Cmd 3). */
result_t C5E209_quickstop(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id);

/** Halt: ramp to 0, stay energized (sets halt bit on the last command). */
result_t C5E209_halt(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                     c5e209_cmd_t base_cmd);

/** Clear a pending fault, once its cause is rectified (PDI-Cmd 2). */
result_t C5E209_clear_error(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id);

/* ---- Status --------------------------------------------------------------- */

/** Request PDI-Status (2292h:01h) via SDO upload; reply handled by parser. */
result_t C5E209_request_status(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id);

/** Feed a received FDCAN frame; updates g_c5e209_info on SDO responses. */
void C5E209_parse_can_message(const FDCAN_RxHeaderTypeDef* rx_header,
                              const uint8_t data[8]);

/** Cached PDI-Status for a motor slot (0 if unknown). */
int16_t C5E209_get_status(uint8_t motor_index);

/* ---- Low-level helpers ---------------------------------------------------- */

/** Generic expedited SDO download (write) to index:subindex, size 1/2/4 bytes. */
result_t C5E209_sdo_write(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                          uint16_t index, uint8_t subindex,
                          int32_t value, uint8_t size);

/** Generic SDO upload (read) request to index:subindex. */
result_t C5E209_sdo_read(FDCAN_HandleTypeDef* can_handler, uint8_t controller_id,
                         uint16_t index, uint8_t subindex);

#endif /* NANOTECH_C5E209_H */
