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
#include "tim.h"
#include "usart.h"
#include <stdint.h>

#define TAG "MAIN"

extern TIM_HandleTypeDef htim1;
UART_HandleTypeDef huart_com;
extern COM_InitTypeDef BspCOMInit;
/**
 * @brief System Clock Configuration from cubemx_main.c @retval None
 */
extern void SystemClock_Config(void);
void uart_setup() {

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity
   */
  BspCOMInit.BaudRate = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits = COM_STOPBITS_1;
  BspCOMInit.Parity = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
    Error_Handler();
  }
  MX_USART3_Init(&huart_com, &BspCOMInit);
}
int main(void) {

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM1_Init();
  uart_setup();
  LOG_init(&huart_com);
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
