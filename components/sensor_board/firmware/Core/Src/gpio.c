/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PC14-OSC32_IN (OSC32_IN)   ------> RCC_OSC32_IN
     PC15-OSC32_OUT (OSC32_OUT)   ------> RCC_OSC32_OUT
     PC8   ------> S_TIM3_CH3
     PC12   ------> UART5_TX
     PD2   ------> UART5_RX
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, IMU_Sync_Pin|WEIGHT_CLOCK_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(WEIGHT_CLOCK_1_GPIO_Port, WEIGHT_CLOCK_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, STEPPER_MOTOR_4_Pin|STEPPER_MOTOR_3_Pin|STEPPER_MOTOR_2_Pin|STEPPER_MOTOR_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : FORCE_ANALOG_DATA_2_Pin */
  GPIO_InitStruct.Pin = FORCE_ANALOG_DATA_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(FORCE_ANALOG_DATA_2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FLOW_SENSOR_Pin WEIGHT_INPUT_1_Pin WEIGHT_INPUT_2_Pin */
  GPIO_InitStruct.Pin = FLOW_SENSOR_Pin|WEIGHT_INPUT_1_Pin|WEIGHT_INPUT_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : IMU_Sync_Pin WEIGHT_CLOCK_2_Pin */
  GPIO_InitStruct.Pin = IMU_Sync_Pin|WEIGHT_CLOCK_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PH_ANALOG_DATA_Pin FORCE_ANALOG_DATA_1_Pin */
  GPIO_InitStruct.Pin = PH_ANALOG_DATA_Pin|FORCE_ANALOG_DATA_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : IMU_Data_Ready_Pin */
  GPIO_InitStruct.Pin = IMU_Data_Ready_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IMU_Data_Ready_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WEIGHT_CLOCK_1_Pin */
  GPIO_InitStruct.Pin = WEIGHT_CLOCK_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WEIGHT_CLOCK_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WATER_PUMP_PWM_Pin */
  GPIO_InitStruct.Pin = WATER_PUMP_PWM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(WATER_PUMP_PWM_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPS_UART_TX_Pin */
  GPIO_InitStruct.Pin = GPS_UART_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
  HAL_GPIO_Init(GPS_UART_TX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPS_UART_RX_Pin */
  GPIO_InitStruct.Pin = GPS_UART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
  HAL_GPIO_Init(GPS_UART_RX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STEPPER_MOTOR_4_Pin STEPPER_MOTOR_3_Pin STEPPER_MOTOR_2_Pin STEPPER_MOTOR_1_Pin */
  GPIO_InitStruct.Pin = STEPPER_MOTOR_4_Pin|STEPPER_MOTOR_3_Pin|STEPPER_MOTOR_2_Pin|STEPPER_MOTOR_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
