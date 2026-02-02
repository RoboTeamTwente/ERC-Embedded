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
#include "cmsis_os2.h"
#include "ethernet.h"
#include "gpio.h"
#include "logging.h"
#include "tim.h"
#include <stdint.h>

#define TAG "MAIN"

extern void MX_FREERTOS_Init(void);
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);

void MainTask(void *argument);

extern TIM_HandleTypeDef htim1;
extern COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;

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

const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 8,
    .priority = (osPriority_t)osPriorityNormal,
};

int main(void) {

  MPU_Config_wrapper();

  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM1_Init();
  osKernelInitialize();

  uart_setup();
  LOG_init(&huart_com);
  ETH_init(NULL, NULL);
  int mac1[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  int mac2[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
  int mac3[6] = {0x13, 0x24, 0x35, 0x46, 0x57, 0x68};
  ETH_setup_MAC_address_filtering(mac1, mac2, mac3);
  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osKernelStart();
  while (1) {
  }
}

void set_mac(int mac[6]) {}
void MainTask(void *argument) {

  uint8_t ip[4] = {0, 0, 0, 0};
  uint8_t mac[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  uint8_t mac_other[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
  //ETH_udp_init();
  while (1) {
    // ETH_udp_send(ip, 7, "udp message");
     //osDelay(100);
    ETH_raw_send(mac_other,
                 "long ass raw message looooong looooooonger "
                 "loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
                 "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
                 "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
                 "ooooooooooooooooooooooooooooooooooooooooooo000000000000000000ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooonger"
                 );
                 
    osDelay(100);
    //ETH_raw_send(mac_other, "-");
    //osDelay(200);
  }
}
