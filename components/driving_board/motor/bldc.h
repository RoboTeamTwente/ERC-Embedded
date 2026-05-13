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

void CL3E_SetFrequency(uint32_t freq);
void CL3E_Forward(uint32_t freq);
void CL3E_Backward(uint32_t freq);
void CL3E_Stop(void);
/**
 * void set_bldc_pwm(void);

void set_single_bldc_pwm(TIM_HandleTypeDef *htim, int tim_channel_no, real_T control_val);

void motor_test_ramp(void);

void set_motor_direction(GPIO_TypeDef *port, uint16_t pin, uint8_t dir);

void motor_test_forward(void);

void motor_test_reverse(void);
 */

 void CL3E_Test(void);

#endif