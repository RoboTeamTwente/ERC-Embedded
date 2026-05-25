#ifndef CL3E_H
#define CL3E_H

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "fdcan.h"

#define CL3E_MAX_MOTORS 2
typedef struct
{
    int32_t actual_position;
    uint8_t node_id;

} cl3e_information;


extern cl3e_information g_cl3e_info[CL3E_MAX_MOTORS];

void cl3e_set_mode(
    FDCAN_HandleTypeDef *hfdcan,
    uint8_t node_id,
    int8_t mode);

void cl3e_set_target_position(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id, int32_t target_position);

void cl3e_start_position_move(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id, uint16_t controlword);

void cl3e_request_position(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id);

void cl3e_parse_can_message(const FDCAN_RxHeaderTypeDef *rx_header, const uint8_t data[8]);

float cl3e_get_position_deg(uint8_t motor_index, int32_t encoder_cpr);

float cl3e_get_position_rad(uint8_t motor_index, int32_t encoder_cpr);

float cl3e_get_position_rev(uint8_t motor_index, int32_t encoder_cpr);

void CL3E_SetFrequency(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t freq);
uint32_t CL3E_ControlToFreq(real_T u);
void CL3E_DriveFromControl(TIM_HandleTypeDef *htim, uint32_t channel, GPIO_TypeDef *dir_port, uint16_t dir_pin, real_T u);
uint32_t CL3E_GetTimerClock(TIM_HandleTypeDef *htim);

#endif