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
#include "ip_mac_constants.h"
#include "logging.h"
#include "networking_constants.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>
#include <time.h>
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
  ETH_init(NULL);
  ETH_raw_init(NULL);
  int mac1[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  int mac2[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
  int mac3[6] = {0x90, 0x2e, 0x16, 0xbe, 0x1b, 0x33};
  // ETH_setup_MAC_address_filtering(mac1, mac2, mac3);

  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osKernelStart();
  while (1) {
  }
}

void set_mac(int mac[6]) {}
void MainTask(void *argument) {

  uint8_t ip[4] = SAMPLE_BOARD_IP;
  uint8_t mac[6] = SAMPEL_BOARD_MAC;
  static StaticQueue_t txStruct0;
  static uint8_t txStorage0[ETHERNET_SQ_LENGTH * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t tx0 = xQueueCreateStatic(ETHERNET_SQ_LENGTH,
                                         ETHERNET_SQ_ITEM_SIZE,
                                         txStorage0, &txStruct0);

  static StaticQueue_t txStruct1;
  static uint8_t txStorage1[ETHERNET_SQ_LENGTH * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t tx1 = xQueueCreateStatic(ETHERNET_SQ_LENGTH,
                                         ETHERNET_SQ_ITEM_SIZE,
                                         txStorage1, &txStruct1);

  QueueHandle_t send_queues[ETHERNET_SQ_PRIORITY_BUFFERS] = {tx0, tx1};
  result_t udp_init = ETH_udp_init(ETHERNET_SQ_PRIORITY_BUFFERS, send_queues, NULL);
  if (udp_init != RESULT_OK) {
    LOGE(TAG, "UDP init failed: %s", result_to_short_str(udp_init));
  }
  ETH_add_arp(ip, mac);
  while (1) {
    (void)ETH_udp_send(ip, 7, "udp message", 1);
    osDelay(100);
    ETH_raw_send(mac, "long ass raw message looooong looooooonger looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongest WASAAPPPPPPPP SHISHIR HERE AND THERE AND EVERYWHERE");
    osDelay(100);
  }
}
