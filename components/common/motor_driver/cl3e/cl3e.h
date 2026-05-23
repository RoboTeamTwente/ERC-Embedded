#ifndef CL3E_H
#define CL3E_H

#include <stdint.h>
#include "stm32h7xx_hal.h"

typedef struct
{
    int32_t actual_position;
    uint8_t node_id;

} cl3e_information;

extern cl3e_information g_cl3e_info;

void cl3e_request_position(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id);

void cl3e_parse_can_message(const FDCAN_RxHeaderTypeDef *rx_header, const uint8_t data[8]);

float cl3e_get_position_deg(int32_t encoder_cpr);

float cl3e_get_position_rad(int32_t encoder_cpr);

float cl3e_get_position_rev(int32_t encoder_cpr);

void CL3E_SetFrequency(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t freq);
uint32_t CL3E_ControlToFreq(real_T u);
void CL3E_DriveFromControl(TIM_HandleTypeDef *htim, uint32_t channel, GPIO_TypeDef *dir_port, uint16_t dir_pin, real_T u);
uint32_t CL3E_GetTimerClock(TIM_HandleTypeDef *htim);

#endif