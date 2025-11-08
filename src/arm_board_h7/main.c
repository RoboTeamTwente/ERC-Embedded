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
#include "logging.h"
#include "stm32f7xx_hal_tim.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include <stdint.h>

#define TAG "MAIN"

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;
/**
 * @brief System Clock Configuration from cubemx_main.c @retval None
 */
extern void SystemClock_Config(void);

int main(void) {

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART3_UART_Init();
  LOG_init(&huart3);
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM1_Init();
  ETH_init(&htim1, NULL, NULL);
  ETH_udp_init();

  uint8_t ip[4] = {0, 0, 0, 0};
  uint8_t mac[6] = {255, 255, 255, 255, 255, 255};
  while (1) {
    ETH_udp_send(ip, 7, "udp message");
    HAL_Delay(100);
    ETH_raw_send(mac, "raw message");
    HAL_Delay(100);
  }
}
