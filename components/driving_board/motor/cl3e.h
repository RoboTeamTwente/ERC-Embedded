#ifndef BLDC_H
#define BLDC_H

#include <stdint.h>
#include "stm32h7xx_hal.h"



void CL3E_SetFrequency(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t freq);
uint32_t CL3E_ControlToFreq(real_T u);
void CL3E_DriveFromControl(TIM_HandleTypeDef *htim, uint32_t channel, GPIO_TypeDef *dir_port, uint16_t dir_pin, real_T u);
uint32_t CL3E_GetTimerClock(TIM_HandleTypeDef *htim);

#endif