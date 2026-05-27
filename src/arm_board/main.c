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
#include <stdint.h>

//Controls code
#include "control_arm_manual.h"

#include "cubemx_main.h"
#include "gpio.h"
#include "stepper.h"
#include "tim.h"

// common libraries
#include "logging.h"
#include "result.h"

// protobuffers
#include "components/arm_board/movement_control_in.pb.h"
#include "pb_message.h"

// freertos
#include "FreeRTOS.h"
#include "cmsis_os.h"

// networking
#include "components/common/networking/inc/ethernet.h"  //long path since LWIP also has ethernet.h
#include "ethernet_udp.h"
#include "ip_mac_constants.h"
#include "networking_constants.h"

// packetdispatcher
#include "packet_dispatcher.h"
#include "packet_dispatcher_macros.h"

#define TAG "ARM_BOARD"

extern ExtY rtY; //Get controls in :)

/*External functions*/
extern COM_InitTypeDef BspCOMInit;
extern void MX_FREERTOS_Init(void);
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
extern void MX_DMA_Init(void);

/*Handles*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart_com;

void my_BSP_COM_Init() {
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

/*Ethernet constants*/

// MY LAPTOP
uint8_t my_mac[6] = {0x6c, 0x24, 0x08, 0xd2, 0xfa, 0x50};
uint8_t my_ip[4] = {192, 168, 0, 5};
// uint8_t my_mac[6] = {0x00, 0x80, 0xe1, 0x00, 0x00, 0x00};
// uint8_t my_ip[4] = {192, 168, 0, 111};
uint8_t netmask[4] = NETMASK;
uint8_t gateway[4] = GATEWAY;

// OTHER LAPTOP
uint8_t ip[4] = {192, 168, 0, 50};
uint8_t mac[6] = NETWORK_MAC;

osThreadId_t stepperTaskHandle;
osThreadId_t testethernetTaskHandle;

/* Task attributes for CMSIS-RTOS v2 */

// osThreadId_t task_2Handle;
// const osThreadAttr_t task2_attributes = {
//     .name = "task2",
//     .stack_size = 1024 * 10,  // Make sure this is enough
//     .priority = tskIDLE_PRIORITY + 1U,
// };
// static void test_ethernet(void* argument);

osThreadId_t pwmScopeTaskHandle;
const osThreadAttr_t pwm_scope_attributes = {
    .name = "pwm_scope",
    .stack_size = 1024 * 8,
    .priority = tskIDLE_PRIORITY,
};
static void pwm_scope_task(void* argument);

// Stepper objects
stepper_t stepper1;
stepper_t stepper2;

/* FOR QUEUE CREATION */
const int queue_size = 5;
const int item_size = sizeof(uint32_t);

QueueHandle_t stepper1_queue_handle;
QueueHandle_t stepper2_queue_handle;
static StaticQueue_t stepper2_queue;
static StaticQueue_t stepper1_queue;

TaskHandle_t stepper1_notifier = NULL;
#define STACK_SIZE 1024 * 8
StaticTask_t xTaskBuffer;
StackType_t xStack[STACK_SIZE];

QueueHandle_t xQueueStepper1;
QueueHandle_t xQueueStepper2;

static void vEthernetTask(void* argument);
static void vStepperTask1(void* argument);
static void vStepperTask2(void* argument);
static void vArmInTask(void* argument);

int main(void) {
  LOGI(TAG, "-----------------main-----------------");

  /*Inits*/
  MPU_Config_wrapper();
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();

  SCB_EnableICache();
  // SCB_EnableDCache();

  // Init timers
  MX_TIM2_Init();
  MX_TIM3_Init();

  // INit all configured peripherals
  my_BSP_COM_Init();

  // Log init
  LOG_init(&huart_com);

  // Init scheduler
  osKernelInitialize();

  // // /* Create the thread(s) */
  // testethernetTaskHandle = osThreadNew(test_ethernet, NULL,
  // &task2_attributes); if (testethernetTaskHandle == NULL) {
  //   // HANDLE
  // }

  // pwmScopeTaskHandle =
  // osThreadNew(pwm_scope_task,NULL,&pwm_scope_attributes); if
  // (pwmScopeTaskHandle == NULL) {
  //     //HANDLE
  // }

  xQueueStepper1 = xQueueCreate(5, sizeof(Arm_StepperSignals_size));
  xQueueStepper2 = xQueueCreate(5, sizeof(Arm_StepperSignals_size));

  if (xQueueStepper1 == NULL) {
    // HANDLE
    LOGE(TAG, "Queue could not be created");
  }

  //Sending task
  xTaskCreate(vArmInTask, "Sender1", 1024*8, NULL, tskIDLE_PRIORITY, NULL);

  //Receiving tasks, prio is one above sending task so it should always empty the queue when msgs are there
  xTaskCreate(vStepperTask1, "Receiver1", 1024*8, NULL, tskIDLE_PRIORITY + 1U, NULL);
  xTaskCreate(vStepperTask2, "Receiver2", 1024*8, NULL, tskIDLE_PRIORITY + 1U, NULL);

  // Start scheduler
  osKernelStart();
  
  // We should never get here as control is now taken by the scheduler
  while (1) {
  }
}

static void vStepperTask1(void* argument) {
  //INit stepper
  pin_t pin1 = {GPIOA, GPIO_PIN_4};
  pin_t pin2 = {GPIOC, GPIO_PIN_0};
  init_stepper(&stepper1, 50, &htim2, pin1, pin2);

  /* Declare the variable that will hold the values received from the
     queue. */
  void* buffer;
  BaseType_t xStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(10); //Queue checks receiving every 10 ms

  /* This task is also defined within an infinite loop. */
  while(1) {
    if (uxQueueMessagesWaiting(xQueueStepper1) != 0) {
      LOGE(TAG, "Queue is not empty!\r\n");
    }

    xStatus = xQueueReceive(xQueueStepper1, &buffer, xTicksToWait);

    if (xStatus == pdPASS) {
      LOGI(TAG, "Received = %u", buffer);

      Arm_StepperSignals* decoded_ss = Arm_StepperSignals_DEFAULT;
      size_t size = Arm_StepperSignals_size;
      result_t res = pb_message_decode(buffer, Arm_StepperSignals_size, Arm_StepperSignals_fields, Arm_StepperSignals_size, &decoded_ss);

      LOGI(TAG, "freq: %u", decoded_ss->stepper_freq);
      LOGI(TAG, "steps: %u", decoded_ss->stepper_steps);

      rotate_stepper(&stepper1, decoded_ss->stepper_steps, decoded_ss->stepper_freq);

    }
  }
}

static void vStepperTask2(void* argument) {
  //Init stepper
  pin_t pin3 = {GPIOA, GPIO_PIN_5};
  pin_t pin4 = {GPIOB, GPIO_PIN_6};
  init_stepper(&stepper2, 50, &htim3, pin3, pin4);

  /* Declare the variable that will hold the values received from the
     queue. */
  void* buffer;
  BaseType_t xStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(10); //Queue checks receiving every 10 ms

  /* This task is also defined within an infinite loop. */
  while(1) {
    if (uxQueueMessagesWaiting(xQueueStepper1) != 0) {
      LOGE(TAG, "Queue is not empty!\r\n");
    }

    xStatus = xQueueReceive(xQueueStepper2, &buffer, xTicksToWait);

    if (xStatus == pdPASS) {
      LOGI(TAG, "Received = %u", buffer);

      Arm_StepperSignals* decoded_ss = Arm_StepperSignals_DEFAULT;
      size_t size = Arm_StepperSignals_size;
      result_t res = pb_message_decode(buffer, Arm_StepperSignals_size, Arm_StepperSignals_fields, Arm_StepperSignals_size, &decoded_ss);

      LOGI(TAG, "freq: %u", decoded_ss->stepper_freq);
      LOGI(TAG, "steps: %u", decoded_ss->stepper_steps);

      rotate_stepper(&stepper2, decoded_ss->stepper_steps, decoded_ss->stepper_freq);

    }
  }
}

static void vArmInTask(void* argument) {
  while(1) {
      osDelay(10 * 1000); //every 10 seconds

      //Steppers

      rtY.stepperLeftFrequency;
      rtY.stepperLeftSteps;
      rtY.stepperRightFrequency;
      rtY.stepperRightSteps;

      /* Encode messages */

      //!NOTE: placeholder values!!!
      Arm_StepperSignals ss_left = {(uint32_t) 30,(uint32_t) 300};
      Arm_StepperSignals ss_right = {(uint32_t) 60,(uint32_t) 600};

      uint8_t *msg_encoded1 = NULL;
      size_t msg_size1 = 0;
      pb_message_encode(&ss_left,Arm_StepperSignals_fields,&msg_encoded1,&msg_size1);
      //!TODO: error handling

      uint8_t *msg_encoded2 = NULL;
      size_t msg_size2 = 0;
      pb_message_encode(&ss_right,Arm_StepperSignals_fields,&msg_encoded2,&msg_size2);
      //!TODO: error handling


      BaseType_t xStatus1;
      xStatus1 = xQueueSendToBack(xQueueStepper1, &msg_encoded1, 0); //!TODO: wait how many seconds?

      if (xStatus1 != pdPASS) {
        LOGE(TAG, "Could not send into queue1 (probably full)");
      }

      BaseType_t xStatus2;
      xStatus2 = xQueueSendToBack(xQueueStepper2, &msg_encoded2, 0); //!TODO: wait how many seconds?

      if (xStatus2 != pdPASS) {
        LOGE(TAG, "Could not send into queue2 (probably full)");
      }


      // BLDC

      

    }
}

static void pwm_scope_task(void* argument) {
  stepper_t step;
  pin_t pin1 = {GPIOA, GPIO_PIN_4};
  pin_t pin2 = {GPIOC, GPIO_PIN_0};
  init_stepper(&step, 50, &htim2, pin1, pin2);

  static const uint32_t scope_pulse_counts[] = {
      10U, 20U, 50U, 100U, 200U, 400U, 800U, 1600U, 3200U, 6400U,
  };

  const size_t count =
      sizeof(scope_pulse_counts) / sizeof(scope_pulse_counts[0]);

  while (1) {
    for (size_t p = 0; p < count; p++) {
      uint32_t pulse_count = scope_pulse_counts[p];

      LOGI(TAG, "--- New pulse count: %lu pulses ---",
           (unsigned long)pulse_count);

      for (int i = 0; i < 10; i++) {
        LOGI(TAG, "Burst %d/10 — %lu pulses", i + 1,
             (unsigned long)pulse_count);

        rotate_stepper(&step, (int)pulse_count, 300);
        osDelay(10);
      }
      LOGI(TAG, "Done with %lu pulses, switching...",
           (unsigned long)pulse_count);
      osDelay(4000U);
    }
  }
}

/* Callback function that handles a specific packet*/
void HandlePacket(receive_frame_t* receive_frame) {
  LOGI(TAG, "Wayoo, message received");
}

/* Config for 1 pbmessage: ArmBoardControlSignals */
// static result_t Callback_ArmBoardControlSignals(void* buffer) {

//   if (buffer == NULL) {
//     return RESULT_ERR_INVALID_ARG;
//   }

//   Arm_* pckt = (ArmBoardControlSignals*)buffer;

//   int32_t steps1 = 50;

//   BaseType_t xStatus;
//   xStatus = xQueueSendToBack(xQueueStepper1, &steps1, 0); //!TODO: wiat how many seconds?

//   if (xStatus != pdPASS) {
//     LOGE(TAG, "Could not send into queue (probably full)");
//   }

//   return RESULT_OK;
// }

// // PACKET_HANDLER_CONFIG_STATIC(Handler_ArmBoardControlSignals,
// // PBEnvelope_arm_ctrl_tag, arm_ctrl, Callback_ArmBoardControlSignals);

// /*Init and pass packet dispatcher*/
// static uint8_t Handle_ArmBoardControlSignals_queue_buffer
//     [PACKET_HANDLER_DEFAULT_QUEUE_LENGTH *
//      sizeof(((PBEnvelope*)0)->payload.arm_ctrl)];

// // These are found in handler_stuff.h
// static packet_handler_config_t handlers[] = {{
//     .handler = (Callback_ArmBoardControlSignals),
//     .task_name = "Handle_ArmBoardControlSignals",
//     .packet_type = (PBEnvelope_arm_ctrl_tag),
//     .task_priority = PACKET_HANDLER_DEFAULT_PRIORITY,
//     .task_stack_depth = PACKET_HANDLER_DEFAULT_STACK_DEPTH,
//     .item_size = sizeof(((PBEnvelope*)0)->payload.arm_ctrl),
//     .queue_length = PACKET_HANDLER_DEFAULT_QUEUE_LENGTH,
//     .queue_buffer = Handle_ArmBoardControlSignals_queue_buffer,
//     .queue_struct = {0},
//     .queue = NULL,
// }};

extern int receiving_counter;
int outgoing_counter = 0;
void vEthernetTask(void* argument) {
  // Setup using sending side params
  ETH_init(NULL, my_ip, netmask, gateway, my_mac);

//   /*Making queues*/
//   int SendQueueSize = 80;

//   static StaticQueue_t xStaticQueue1;
//   uint8_t ucQueueStorageArea1[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
//   QueueHandle_t udp_receiver_queue1 =
//       xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE,
//                          ucQueueStorageArea1, &xStaticQueue1);

//   static StaticQueue_t xStaticQueue2;
//   uint8_t ucQueueStorageArea2[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
//   QueueHandle_t udp_receiver_queue2 =
//       xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE,
//                          ucQueueStorageArea2, &xStaticQueue2);

//   QueueHandle_t queues[2] = {udp_receiver_queue1, udp_receiver_queue2};

  // PacketDispatcherInit(handlers, 1);
  ETH_udp_init(2, queues, HandlePacket);

//   /*Config + add ARP receiving side*/
//   ETH_add_arp(ip, mac, 5);

//   /*Sending a message*/
//   // uint8_t packet1_payload[4] = {14,06,20,04};

//   // /*Test sending*/
//   // while (outgoing_counter < 100) { //NOTE: after 80 packages the queue will
//   // be full!
//   //     ETH_udp_send(ip, 8, packet1_payload, 4, 1);
//   //     outgoing_counter += 1;
//   //     LOGI(TAG, "%d", outgoing_counter);
//   //     osDelay(5000);
//   // }

  while (1) {
    LOGI(TAG, "...ethernet still receiving");
    osDelay(3000);
  }
}
