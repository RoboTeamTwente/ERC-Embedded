#ifndef CUBEMARS_AK
#define CUBEMARS_AK

#include "cubemars_ak_definitions.h"
#include "fdcan.h"
#include "result.h"
#include "stdint.h"

#define CUBEMARS_AK_INFORMATION_PARAM_MASK      \
    (CUBEMARS_AK_PARAM_INPUT_CURRENT_MASK |     \
     CUBEMARS_AK_PARAM_IQ_CURRENT_MASK |        \
     CUBEMARS_AK_PARAM_MOTOR_POSITION_MASK |    \
     CUBEMARS_AK_PARAM_MOTOR_TEMPERATURE_MASK | \
     CUBEMARS_AK_PARAM_MOS_TEMPERATURE_MASK |   \
     CUBEMARS_AK_PARAM_INPUT_VOLTAGE_MASK |     \
     CUBEMARS_AK_PARAM_STATUS_CODE_MASK)

#define CUBEMARS_AK_MAX_NUMBER_OF_MOTORS (10)
/**
 * @brief Servo-mode CAN command identifiers for CubeMars AK motor drivers.
 *
 * These values are used as the function/control-mode portion of the extended
 * CAN identifier.
 *
 * The final extended CAN ID is built as:
 *
 * @code
 * can_id = (message_type << 8) | controller_id;
 * @endcode
 */
typedef enum {
    CUBEMARS_AK_SET_DUTY = 0,
    CUBEMARS_AK_SET_CURRENT,
    CUBEMARS_AK_SET_CURRENT_BRAKE,
    CUBEMARS_AK_SET_RPM,
    CUBEMARS_AK_SET_POSITION,
    CUBEMARS_AK_SET_ORIGIN_HERE,
    CUBEMARS_AK_SET_POS_SPD,
    CUBEMARS_AK_SET_MIT = 8,
} cubemars_ak_message_type;

typedef enum {
    CUBEMARS_AK70_10 = 0,
    CUBEMARS_AK_45_36,
} cubemars_ak_motor_model;

typedef enum {
    FAULT_CODE_NONE = 0,
    FAULT_CODE_OVER_VOLTAGE,                        // Over-voltage
    FAULT_CODE_UNDER_VOLTAGE,                       // Under-voltage
    FAULT_CODE_DRV,                                 // Driver fault
    FAULT_CODE_ABS_OVER_CURRENT,                    // Motor over-current
    FAULT_CODE_OVER_TEMP_FET,                       // MOS over-temperature
    FAULT_CODE_OVER_TEMP_MOTOR,                     // Motor over-temperature
    FAULT_CODE_GATE_DRIVER_OVER_VOLTAGE,            // Driver over-voltage
    FAULT_CODE_GATE_DRIVER_UNDER_VOLTAGE,           // Driver under-voltage
    FAULT_CODE_MCU_UNDER_VOLTAGE,                   // MCU under-voltage
    FAULT_CODE_BOOTING_FROM_WATCHDOG_RESET,         // Under-voltage
    FAULT_CODE_ENCODER_SPI,                         // SPI encoder fault
    FAULT_CODE_ENCODER_SINCOS_BELOW_MIN_AMPLITUDE,  // Encoder out of range
    FAULT_CODE_ENCODER_SINCOS_ABOVE_MAX_AMPLITUDE,  // Encoder out of range
    FAULT_CODE_FLASH_CORRUPTION,                    // FLASH fault
    FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_1,  // Current sampling channel 1
                                              // fault
    FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_2,  // Current sampling channel 2
                                              // fault
    FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_3,  // Current sampling channel 3
                                              // fault
    FAULT_CODE_UNBALANCED_CURRENTS,           // Current imbalance
} cubemars_ak_error_code;

/**
 * @brief Decoded servo-mode CAN feedback from a CubeMars AK motor driver.
 *
 * Values are stored in the raw fixed-point units used by the CAN feedback
 * frame. Convert to physical units only at the application or logging boundary.
 *
 * Scaling:
 * - motor_position: raw / 10 = degrees
 * - motor_speed: raw * 10 = ERPM
 * - motor_current: raw / 100 = A
 * - motor_temperature: raw = degrees Celsius
 * - status_code: raw fault/status code
 */
typedef struct {
    /** Motor position in 0.1 degree units. */
    int16_t motor_position;

    /** Motor speed in 10 ERPM units. */
    int16_t motor_speed;

    /** Motor current in 0.01 A units. */
    int16_t motor_current;

    /** Motor temperature in degrees Celsius. */
    int8_t motor_temperature;

    /** Motor fault/status code. */
    cubemars_ak_error_code status_code;

    /** Motor controller ID extracted from the CAN identifier. */
    uint8_t motor_id;
} cubemars_ak_information;

/**
 * @brief Parse a CubeMars AK servo-mode CAN feedback frame.
 *
 * Decodes the standard 8-byte servo-mode feedback payload from a CubeMars AK
 * motor driver and stores the result in a cubemars_ak_information structure.
 *
 * Expected CAN frame format:
 * - Extended CAN ID
 * - DLC: 8 bytes
 * - Function ID: CUBEMARS_AK_CAN_SERVO_FEEDBACK_ID
 *
 * Extended CAN identifier layout:
 *
 * @code
 * bits [28:8] = function ID
 * bits [7:0]  = motor/controller ID
 * @endcode
 *
 * Feedback payload layout:
 *
 * @code
 * data[0..1] = motor position
 * data[2..3] = motor speed
 * data[4..5] = motor current
 * data[6]    = motor temperature
 * data[7]    = status/error code
 * @endcode
 *
 * Raw scaling:
 * - motor_position: raw / CUBEMARS_AK_CAN_POSITION_SCALE = degrees
 * - motor_speed: raw * CUBEMARS_AK_CAN_SPEED_SCALE = ERPM
 * - motor_current: raw / CUBEMARS_AK_CAN_CURRENT_SCALE = A
 * - motor_temperature: raw / CUBEMARS_AK_CAN_TEMPERATURE_SCALE = degrees
 * Celsius
 *
 * @param[in] rx_header Pointer to the received FDCAN header.
 * @param[in] data Pointer to the 8-byte received CAN payload.
 * @param[out] out Pointer to the decoded motor information structure.
 *
 * @return RESULT_OK if the feedback frame was parsed successfully.
 * @return RESULT_ERR_INVALID_ARG if rx_header, data, or out is NULL.
 * @return RESULT_ERR_BAD_FORMAT if the frame is not a valid CubeMars AK servo
 *         feedback frame.
 */
result_t cubemars_ak_parse_can_feedback(const FDCAN_RxHeaderTypeDef* rx_header,
                                        const uint8_t data[8],
                                        cubemars_ak_information* out);

/**
 * @brief Build the extended CAN identifier for a CubeMars AK servo command.
 *
 * Servo-mode CAN commands use an extended CAN identifier where the message type
 * occupies bits [28:8] and the controller ID occupies bits [7:0].
 *
 * @param[in] controller_id CubeMars motor controller ID.
 * @param[in] type CubeMars AK command/message type.
 *
 * @return Extended CAN identifier for the requested command.
 */
uint32_t cubemars_ak_get_can_id(uint8_t controller_id,
                                cubemars_ak_message_type type);

/**
 * @brief Send a velocity command to a CubeMars AK motor.
 *
 * Sends a servo-mode CAN velocity command using ERPM units. Positive and
 * negative values command opposite directions.
 *
 * @param[in] can_handler Pointer to the FDCAN peripheral handle.
 * @param[in] controller_id CubeMars motor controller ID.
 * @param[in] erpm Target electrical RPM.
 *
 * @return RESULT_OK if the CAN frame was queued successfully.
 * @return RESULT_ERR_INVALID_ARG if can_handler is NULL.
 * @return RESULT_FAIL if the CAN frame could not be queued.
 */
result_t cubemars_ak_set_speed(FDCAN_HandleTypeDef* can_handler,
                               uint8_t controller_id, int32_t erpm);

/**
 * @brief Send a position command to a CubeMars AK motor.
 *
 * Sends a servo-mode CAN position command. The position is provided in degrees
 * and converted internally to the fixed-point format expected by the motor
 * driver.
 *
 * @param[in] can_handler Pointer to the FDCAN peripheral handle.
 * @param[in] controller_id CubeMars motor controller ID.
 * @param[in] position_deg Target position in degrees.
 *
 * @return RESULT_OK if the CAN frame was queued successfully.
 * @return RESULT_ERR_INVALID_ARG if can_handler is NULL.
 * @return RESULT_FAIL if the CAN frame could not be queued.
 */
result_t cubemars_ak_set_position(FDCAN_HandleTypeDef* can_handler,
                                  uint8_t controller_id, float position_deg);

/**
 * @brief Set the current motor position as the origin.
 *
 * Sends a servo-mode CAN origin command.
 *
 * Supported origin modes:
 * - CUBEMARS_AK_ORIGIN_TEMPORARY: temporary origin, lost after power cycle
 * - CUBEMARS_AK_ORIGIN_PERMANENT: permanent origin, only for supported models
 *
 * @param[in] can_handler Pointer to the FDCAN peripheral handle.
 * @param[in] controller_id CubeMars motor controller ID.
 * @param[in] origin_mode Origin mode to apply.
 *
 * @return RESULT_OK if the CAN frame was queued successfully.
 * @return RESULT_ERR_INVALID_ARG if can_handler is NULL or origin_mode is
 * invalid.
 * @return RESULT_FAIL if the CAN frame could not be queued.
 */
result_t cubermars_ak_set_origin(FDCAN_HandleTypeDef* can_handler,
                                 uint8_t controller_id, uint8_t origin_mode);

/**
 * @brief Send a brake-current command to a CubeMars AK motor.
 *
 * Sends a servo-mode CAN current brake command. The brake current is provided
 * in amperes and converted internally to the fixed-point format expected by
 * the motor driver.
 *
 * The CubeMars AK servo CAN protocol represents brake current as:
 *
 * @code
 * raw_current = current_a * 1000
 * @endcode
 *
 * Valid range:
 * - 0 A to 60 A
 *
 * @param[in] can_handler Pointer to the FDCAN peripheral handle.
 * @param[in] controller_id CubeMars motor controller ID.
 * @param[in] current_a Brake current in amperes.
 *
 * @return RESULT_OK if the CAN frame was queued successfully.
 * @return RESULT_ERR_INVALID_ARG if can_handler is NULL or current_a is
 * invalid.
 * @return RESULT_FAIL if the CAN frame could not be queued.
 */
result_t cubemars_ak_set_brake_current(FDCAN_HandleTypeDef* can_handler,
                                       uint8_t controller_id, float current_a);

void cubemars_ak_print_feedback(cubemars_ak_information* info);

result_t cubemars_ak_uart_get_motor_id(UART_HandleTypeDef* uart_handler,
                                       uint8_t* motor_id);
#endif
