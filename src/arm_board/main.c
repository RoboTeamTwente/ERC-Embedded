/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "cubemx_main.h"
#include "dma.h"
#include "gpio.h"
#include "stepper.h"
#include "tim.h"
#include <stdint.h>
#include <stdlib.h>

#include "logging.h"
#include "result.h"

#include "components/arm_board/movement_control_in.pb.h"
#include "pb_message.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"

#include "ethernet.h"
#include "ip_mac_constants.h"
#include "networking_constants.h"

#include "packet_dispatcher.h"

#define TAG "ARM_BOARD"

extern COM_InitTypeDef BspCOMInit;
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
extern void MX_DMA_Init(void);

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart_com;

osThreadId_t pwmScopeTaskHandle;
osThreadId_t stepperTaskHandle;
osThreadId_t sendEthTaskHandle;

const osThreadAttr_t pwm_scope_attributes = {
    .name = "pwm_scope",
    .stack_size = 1024 * 2,
    .priority = tskIDLE_PRIORITY,
};

const osThreadAttr_t send_eth_attributes = {
    .name = "send eth",
    .stack_size = 1024 * 2,
    .priority = tskIDLE_PRIORITY,
};

/*Ethernet constants*/
// Sending side
uint8_t my_mac[6] = SAMPEL_BOARD_MAC;
uint8_t my_ip[4] = SAMPLE_BOARD_IP;
uint8_t netmask[4] = NETMASK;
uint8_t gateway[4] = GATEWAY;

// Receiving side
uint8_t ip[4] = NETWORK_IP;
uint8_t mac[6] = NETWORK_MAC;

static void pwm_scope_task(void *argument);
static void send_eth_task(void *argument);
void setup_ethernet();

static void my_BSP_COM_Init(void) {
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
  MPU_Config_wrapper();
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();

  my_BSP_COM_Init();
  LOG_init(&huart_com);

  // Setup using sending side params
  ETH_init(NULL, my_ip, netmask, gateway, my_mac);

  osKernelInitialize();

  // pwmScopeTaskHandle =
  // osThreadNew(pwm_scope_task,NULL,&pwm_scope_attributes);

  // if (pwmScopeTaskHandle == NULL) {
  //     //HANDLE ERROR
  // }

  sendEthTaskHandle = osThreadNew(send_eth_task, NULL, &send_eth_attributes);

  if (sendEthTaskHandle == NULL) {
    // HANDLE ERROR
  }

  osKernelStart();

  while (1) {
  }
}

/* Callback function that handles a specific packet*/
void HandlePacket(receive_frame_t *receive_frame) {
  printf("Wayoo, message received");
}

void setup_ethernet() { /*Making queues*/ }

int outgoing_counter = 0;
static void send_eth_task(void *argument) {

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

  // PacketDispatcherInit(handlers, 2);
  ETH_udp_init(2, queues, HandlePacket);

  /*Config + add ARP receiving side*/
  ETH_add_arp(ip, mac, 5);

  /*Sending a message*/
  uint8_t packet1_payload[4] = {14, 06, 20, 04};
  ETH_udp_send(ip, 8, packet1_payload, 4, 1);

  while (1) {
    LOGI(TAG, "HERE");
    osDelay(1000);
  }
}

static void pwm_scope_task(void *argument) {
  (void)argument;

  stepper_t step;
  init_stepper(&step, 1, 50, &htim2);

  static const uint32_t scope_pulse_counts[] = {
      10U, 20U, 50U, 100U, 200U, 400U, 800U, 1600U, 3200U, 6400U,
  };

  const size_t count =
      sizeof(scope_pulse_counts) / sizeof(scope_pulse_counts[0]);

  for (;;) {
    for (size_t p = 0; p < count; p++) {
      uint32_t pulse_count = scope_pulse_counts[p];

      LOGI(TAG, "--- New pulse count: %lu pulses ---",
           (unsigned long)pulse_count);

      for (int i = 0; i < 10; i++) {
        LOGI(TAG, "Burst %d/10 — %lu pulses", i + 1,
             (unsigned long)pulse_count);

        do_pwm_dma(&step, (int)pulse_count);

        while (step.pwm_dma_active) {
          osDelay(10);
        }
      }
      LOGI(TAG, "Done with %lu pulses, switching...",
           (unsigned long)pulse_count);
      osDelay(4000U);
    }
  }

  while (1) {
  }
}
