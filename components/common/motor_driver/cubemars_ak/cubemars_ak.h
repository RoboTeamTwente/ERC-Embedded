#ifndef CUBEMARS_AK

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

typedef struct {
    int16_t motor_position;
    int16_t motor_speed;
    int16_t motor_current;
    int8_t motor_temperature;
    cubemars_ak_error_code status_code;
    uint8_t motor_id;
} cubemars_ak_information;

result_t cubemars_ak_information_from_buffer(
    uint8_t* buffer, size_t buffer_size, cubemars_ak_information* information);

uint32_t cubemars_ak_get_can_id(uint8_t controler_id,
                                cubemars_ak_message_type type);

result_t cubemars_ak_set_speed(FDCAN_HandleTypeDef* can_handler,
                               uint8_t controler_id, uint32_t erpm);

result_t cubemars_ak_set_position(FDCAN_HandleTypeDef* can_handler,
                                  uint8_t controler_id, uint32_t position);

result_t cubermars_ak_set_origin(FDCAN_HandleTypeDef* can_handler,
                                 uint8_t controler_id);
#endif
