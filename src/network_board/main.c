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
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "ethernet.h"
#include "gpio.h"
#include "ip_mac_constants.h"
#include "logging.h"
#include "netif.h"
#include "networking_constants.h"
#include "queue.h"
#include "stm/ethernet_udp.h"
#include "tim.h"
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
extern struct netif gnetif;

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

void ethernet_linkstatus_callback(struct netif *netif) {
  if (netif_is_up(netif)) {
    LOGI(TAG, "Physical ethernet link is up");
  } else {
    LOGE(TAG, "Physical ethernet link is down");
  }
}

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
  ETH_init(ethernet_linkstatus_callback);
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
  int SendQueueSize = 80;
  static StaticQueue_t xStaticQueue1;
  uint8_t ucQueueStorageArea1[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t udp_receiver_queue1 =
      xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE,
                         ucQueueStorageArea1, &xStaticQueue1);

  static StaticQueue_t xStaticQueue2;
  uint8_t ucQueueStorageArea2[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t udp_receiver_queue2 =
      xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE,
                         ucQueueStorageArea2, &xStaticQueue2);
  QueueHandle_t queues[2] = {udp_receiver_queue1, udp_receiver_queue2};

  uint8_t ip[4] = SAMPLE_BOARD_IP;
  uint8_t mac[6] = SAMPEL_BOARD_MAC;
  ETH_udp_init(2, queues);
  ETH_add_arp(ip, mac);
  uint8_t message[12] = "udp message";
  while (1) {
    ETH_udp_send(ip, 7, message, 12, 1);
    osDelay(100);
    // ETH_raw_send(mac_other,
    //              "long ass raw message looooong looooooonger "
    //              "loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
    //              "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
    //              "ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
    //              "ooooooooooooooooooooooooooooooooooooooooooo000000000000000000ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooonger"
    //              );

    // osDelay(100);
    // ETH_raw_send(mac_other, "-");
    // osDelay(200);
  }
}
