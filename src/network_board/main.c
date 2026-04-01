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
#include "components/common/packet_dispatcher/packet_dispatcher.h"
#include "components/sensor_board/gps_sensor.pb.h"
#include "components/sensor_board/ph_sensor.pb.h"
#include "ethernet.h"
#include "gpio.h"
#include "ip_mac_constants.h"
#include "logging.h"
#include "netif.h"
#include "networking_constants.h"
#include "queue.h"
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
static int incomming_counter = 0;
static int outgoing_counter = 0;
static result_t HandleType1Packet(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  SensorBoardGPSInfo *packet = (SensorBoardGPSInfo *)buffer;
  incomming_counter += 1;
  printf("Envelope of type gps info has value: %f\n", packet->speed);
  printf("This is packet: %d\n", incomming_counter);
  return RESULT_OK;
}

static result_t HandleType2Packet(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  SensorBoardPHInfo *packet = (SensorBoardPHInfo *)buffer;
  printf("envelope of type ph info has value: %f\n", packet->ph_value);
  return RESULT_OK;
}

static uint8_t packet1_payload[] = {
    0x62, 0x2C, 0x09, 0x13, 0xF2, 0x41, 0xCF, 0x66, 0x1D, 0x4A, 0x40, 0x11,
    0x2C, 0x65, 0x19, 0xE2, 0x58, 0x97, 0x1B, 0x40, 0x1D, 0x00, 0x00, 0x0C,
    0x42, 0x2D, 0x00, 0x00, 0x87, 0x43, 0x35, 0x9A, 0x99, 0x99, 0x3F, 0x3D,
    0x66, 0x66, 0xE6, 0x3F, 0x40, 0x09, 0x48, 0x01, 0x50, 0x01};

static uint8_t packet1_buffer[SensorBoardGPSInfo_size * 5];
static uint8_t packet2_buffer[SensorBoardPHInfo_size * 5];

static packet_handler_config_t handler_configs[] = {
    {.handler = HandleType1Packet,
     .task_name = "GPS Handler",
     .packet_type = PBEnvelope_gps_info_tag,
     .item_size = SensorBoardGPSInfo_size,
     .task_priority = tskIDLE_PRIORITY + 2U,
     .queue_length = 5,
     .queue_buffer = packet1_buffer}};

extern int receive_counter;
void MainTask(void *argument) {

  uint8_t ip[4] = SAMPLE_BOARD_IP;
  uint8_t mac[6] = SAMPEL_BOARD_MAC;

<<<<<<< HEAD
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
  result_t udp_init =
      ETH_udp_init(ETHERNET_SQ_PRIORITY_BUFFERS, send_queues, NULL);
  if (udp_init != RESULT_OK) {
    LOGE(TAG, "UDP init failed: %s", result_to_short_str(udp_init));
  }

  ETH_add_arp(ip, mac);
  while (1) {
    (void)ETH_udp_send(ip, 7, "udp message", 1);
    osDelay(100);
    ETH_raw_send(mac,
                 "long ass raw message looooong looooooonger "
                 "looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
                 "oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
                 "oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
                 "oooooooooooooooooooooooooooooongest WASAAPPPPPPPP SHISHIR HERE AND "
                 "THERE AND EVERYWHERE");
    osDelay(100);
=======
  PacketDispatcherInit(handler_configs, 1);

  ETH_udp_init(2, queues, DispatchPacket);
  ETH_add_arp(ip, mac);
  while (outgoing_counter < 100) {
    ETH_udp_send(ip, 8, packet1_payload, 46, 1);
    osDelay(100);
    outgoing_counter += 1;
    LOGI(TAG, "%d", outgoing_counter);
  }

  while (1) {
    __asm__ __volatile__("nop");
    LOGI(TAG, "Total messages send: %d", outgoing_counter);
    LOGI(TAG, "Total messages received: %d", receive_counter);
    osDelay(300);
>>>>>>> 2aab4a75d8b73e7179c9866939b138f2fe84fea4
  }
}
