/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "ethernet.h"
#include "gpio.h"
// #include "logging.h"
#include "tim.h"
#include <stdint.h>

#define TAG "MAIN"

void MX_GPIO_Init(void);
void MX_TIM2_Init(void);

int main(void) {

  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  HAL_Init();

  // SystemClock_Config();
  // LOG_init(&huart_com);
  
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  while (1) {
    //Blinking the LED B)
    HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
    HAL_Delay(500);

    //PWM
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 105); //50% duty cycle
  }
}
