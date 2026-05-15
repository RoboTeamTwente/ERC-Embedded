#ifndef BLDC_H
#define BLDC_H

#include <stdint.h>
#include "stm32h7xx_hal.h"

#define LF_DIR_PORT GPIOB
#define LF_DIR_PIN  GPIO_PIN_0

#define LM_DIR_PORT GPIOB
#define LM_DIR_PIN  GPIO_PIN_1

#define LB_DIR_PORT GPIOB
#define LB_DIR_PIN  GPIO_PIN_2

// RIGHT SIDE
#define RF_DIR_PORT GPIOB
#define RF_DIR_PIN  GPIO_PIN_10

#define RM_DIR_PORT GPIOB
#define RM_DIR_PIN  GPIO_PIN_11

#define RB_DIR_PORT GPIOB
#define RB_DIR_PIN  GPIO_PIN_12

void CL3E_SetFrequency(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t freq);
uint32_t CL3E_ControlToFreq(real_T u);
void CL3E_DriveFromControl(TIM_HandleTypeDef *htim, uint32_t channel, GPIO_TypeDef *dir_port, uint16_t dir_pin, real_T u);
uint32_t CL3E_GetTimerClock(TIM_HandleTypeDef *htim);
/**
 * void CL3E_Forward(uint32_t freq);
void CL3E_Backward(uint32_t freq);
void CL3E_Stop(void);

 */


 //void CL3E_Test(void);

#endif