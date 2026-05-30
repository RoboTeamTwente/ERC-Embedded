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
#include "main.h"
#include <stdint.h>

// Controls code
#include "erc-control-arm/control_arm_manual_ert_rtw/control_arm_manual.h"

#include "cubemx_main.h"
#include "erc-control-arm/control_arm_manual_ert_rtw/rtwtypes.h"
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
#include "components/common/networking/inc/ethernet.h" //long path since LWIP also has ethernet.h
#include "ethernet_udp.h"
#include "ip_mac_constants.h"
#include "networking_constants.h"

// packetdispatcher
#include "cubemars_ak.h"
#include "fdcan.h"
#include "packet_dispatcher.h"
#include "packet_dispatcher_macros.h"

#define TAG "ARM_BOARD"

extern ExtY rtY; // Get controls in :)
extern ExtU rtU; // Get controls in :)
extern void controlArmManualStep(void);
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
uint8_t my_ip[4] = {192, 168, 0, 111};
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
//     .stack_size = 1024 * 10, // Make sure this is enough
//     .priority = tskIDLE_PRIORITY + 1U,
// };
// static void test_ethernet(void* argument);

osThreadId_t pwmScopeTaskHandle;
const osThreadAttr_t pwm_scope_attributes = {
    .name = "pwm_scope",
    .stack_size = 1024 * 8,
    .priority = tskIDLE_PRIORITY,
};
static void pwm_scope_task(void *argument);

// Stepper objects
stepper_t stepper1;
stepper_t stepper2;

int stepper1_count = 0;
int stepper2_count = 0;
/* FOR QUEUE CREATION */
const int queue_size = 5;
const int item_size = sizeof(uint32_t);

// QueueHandle_t stepper1_queue_handle;
// QueueHandle_t stepper2_queue_handle;
// static StaticQueue_t stepper2_queue;
// static StaticQueue_t stepper1_queue;

// TaskHandle_t stepper1_notifier = NULL;
// #define STACK_SIZE 1024 * 8
// StaticTask_t xTaskBuffer;
// StackType_t xStack[STACK_SIZE];

#define STEPPER_QUEUE_SIZE sizeof(arm_stepper_signals)
#define STEPPER_QUEUE_LENGTH 5
QueueHandle_t xQueueStepper1;
static uint8_t xQueueStepper1Storage[STEPPER_QUEUE_SIZE * STEPPER_QUEUE_LENGTH];
static StaticQueue_t xQueueStepper1QueueBuffer;
QueueHandle_t xQueueStepper2;
static uint8_t xQueueStepper2Storage[STEPPER_QUEUE_SIZE * STEPPER_QUEUE_LENGTH];
static StaticQueue_t xQueueStepper2QueueBuffer;
QueueHandle_t xQueueStepper3;
static uint8_t xQueueStepper3Storage[STEPPER_QUEUE_SIZE * STEPPER_QUEUE_LENGTH];
static StaticQueue_t xQueueStepper3QueueBuffer;

static void vEthernetTask(void *argument);
static void vStepperTask1(void *argument);
static void vStepperTask2(void *argument);
static void vArmInTask(void *argument);
static void vArmController(void *argument);

void setup_control_parameters() {

  rtU.x = 0.795;
  rtU.y = 0.0;
  rtU.z = 0.322;
  rtU.gripperAng = 5 * (M_PI / 180);
  rtU.gripperPitchActualPosition = 0; // input can
  rtU.stepperLeftActualPosition = 0;
  rtU.stepperRightActualPosition = 0;
}

static void CAN_ConfigRx_AllStandard(void) {
  FDCAN_FilterTypeDef filter = {0};

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = 0;
  filter.FilterType = FDCAN_FILTER_MASK;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

  // Accept everything: (ID & 0x000) == (0x000 & 0x000)
  filter.FilterID1 = 0x000;
  filter.FilterID2 = 0x000;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filter) != HAL_OK) {
    LOGE("CAN", "Filter config failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_ConfigGlobalFilter(
          &hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
          FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
    LOGE("CAN", "Global filter failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }
}
void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan,
                                        uint32_t BufferIndexes) {
  LOGI("CAN", "TX complete buffers=0x%08lx\n", BufferIndexes);
}

void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan,
                                     uint32_t BufferIndexes) {
  LOGI("CAN", "TX abort buffers=0x%08lx\n", BufferIndexes);
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
  LOGE("CAN", "Error callback HALerr=0x%08lx\n", HAL_FDCAN_GetError(hfdcan));
}

static cubemars_ak_information motor_info = {0};

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
  if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0) {
    return;
  }

  FDCAN_RxHeaderTypeDef rx_header = {0};
  uint8_t rx_data[8] = {0};

  if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) !=
      HAL_OK) {
    LOGE("CAN", "RX read failed, err=0x%08lx", HAL_FDCAN_GetError(hfdcan));
    return;
  }
  cubemars_ak_parse_can_feedback(&rx_header, rx_data, &motor_info);
}

static void CAN2_ConfigRx_AllStandard(void) {
  FDCAN_FilterTypeDef filter = {0};

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = 0;
  filter.FilterType = FDCAN_FILTER_MASK;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

  // Accept everything: (ID & 0x000) == (0x000 & 0x000)
  filter.FilterID1 = 0x000;
  filter.FilterID2 = 0x000;

  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter) != HAL_OK) {
    LOGE("CAN", "Filter config failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan2));
    Error_Handler();
  }

  if (HAL_FDCAN_ConfigGlobalFilter(
          &hfdcan2, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
          FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
    LOGE("CAN", "Global filter failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan2));
    Error_Handler();
  }
}

static void CAN_LogStatus(FDCAN_HandleTypeDef *hfdcan) {
  FDCAN_ProtocolStatusTypeDef protocol_status;
  FDCAN_ErrorCountersTypeDef error_counters;

  if (HAL_FDCAN_GetProtocolStatus(hfdcan, &protocol_status) == HAL_OK) {
    LOGI("CAN",
         "LastErrorCode=%lu DataLastErrorCode=%lu Activity=%lu BusOff=%lu",
         protocol_status.LastErrorCode, protocol_status.DataLastErrorCode,
         protocol_status.Activity, protocol_status.BusOff);
  }

  if (HAL_FDCAN_GetErrorCounters(hfdcan, &error_counters) == HAL_OK) {
    LOGI("CAN", "TxErrorCnt=%lu RxErrorCnt=%lu RxErrorPassive=%lu",
         error_counters.TxErrorCnt, error_counters.RxErrorCnt,
         error_counters.RxErrorPassive);
  }

  LOGI("CAN", "HAL error=0x%08lx", HAL_FDCAN_GetError(hfdcan));
}
int main(void) {
  setup_control_parameters();
  LOGI(TAG, "-----------------main-----------------");

  /*Inits*/
  MPU_Config_wrapper();
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();

  SCB_EnableICache();
  // SCB_EnableDCache();

  MX_FDCAN1_Init();
  MX_FDCAN2_Init();

  // Init timers
  MX_TIM2_Init();
  MX_TIM3_Init();

  // INit all configured peripherals
  my_BSP_COM_Init();

  // Log init
  LOG_init(&huart_com);

  // ETH_init(NULL, my_ip, netmask, gateway, my_mac);

  // Init scheduler
  osKernelInitialize();

  CAN_ConfigRx_AllStandard();
  if (HAL_FDCAN_ConfigInterruptLines(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     FDCAN_INTERRUPT_LINE0) != HAL_OK) {
    LOGE("CAN", "Interrupt line config failed err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     0) != HAL_OK) {
    LOGE("CAN", "Activate RX notification failed err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
    LOGE(TAG, "FDCAN start failed, err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
    for (;;)
      ;
  }
  LOGI("CAN", "Mode=%lu Presc=%lu TS1=%lu TS2=%lu SJW=%lu", hfdcan1.Init.Mode,
       hfdcan1.Init.NominalPrescaler, hfdcan1.Init.NominalTimeSeg1,
       hfdcan1.Init.NominalTimeSeg2, hfdcan1.Init.NominalSyncJumpWidth);

  CAN2_ConfigRx_AllStandard();

  if (HAL_FDCAN_ConfigInterruptLines(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     FDCAN_INTERRUPT_LINE0) != HAL_OK) {
    LOGE("CAN", "Interrupt line config failed err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan2));
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     0) != HAL_OK) {
    LOGE("CAN", "Activate RX notification failed err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan2));
    Error_Handler();
  }
  if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) {
    LOGE(TAG, "FDCAN start failed, err=0x%08lx", HAL_FDCAN_GetError(&hfdcan2));
    for (;;)
      ;
  }

  LOGI("CAN", "FDCAN2: Mode=%lu Presc=%lu TS1=%lu TS2=%lu SJW=%lu",
       hfdcan2.Init.Mode, hfdcan2.Init.NominalPrescaler,
       hfdcan2.Init.NominalTimeSeg1, hfdcan2.Init.NominalTimeSeg2,
       hfdcan2.Init.NominalSyncJumpWidth);

  // // /* Create the thread(s) */
  // testethernetTaskHandle = osThreadNew(vEthernetTask, NULL,
  // &task2_attributes); if (testethernetTaskHandle == NULL) {
  //   // HANDLE2
  // }

  // pwmScopeTaskHandle =
  // osThreadNew(pwm_scope_task,NULL,&pwm_scope_attributes); if
  // (pwmScopeTaskHandle == NULL) {
  //     //HANDLE
  // }

  // xQueueStepper1 =
  //     xQueueCreateStatic(STEPPER_QUEUE_LENGTH, STEPPER_QUEUE_SIZE,
  //                        xQueueStepper1Storage, &xQueueStepper1QueueBuffer);
  // xQueueStepper2 =
  //     xQueueCreateStatic(STEPPER_QUEUE_LENGTH, STEPPER_QUEUE_SIZE,
  //                        xQueueStepper2Storage, &xQueueStepper2QueueBuffer);
  ;
  // xQueueStepper3 = xQueueCreate(5, sizeof(rtY.baseControl));

  // if (xQueueStepper1 == NULL) {
  //   // HANDLE
  //   LOGE(TAG, "Queue could not be created");
  // }

  // Sending task
  // xTaskCreate(vArmInTask, "Sender1", 1024 * 8, NULL, tskIDLE_PRIORITY,
  // NULL);

  // Receiving tasks, prio is one above sending task so it should always empty
  // the queue when msgs are there
  // xTaskCreate(vStepperTask1, "Receiver1", 1024 * 8, NULL, tskIDLE_PRIORITY +
  // 1U,
  //             NULL);
  // xTaskCreate(vStepperTask2, "Receiver2", 1024 * 8, NULL, tskIDLE_PRIORITY +
  // 5U,
  //             NULL);

  xTaskCreate(vArmController, "ArmController", 1024 * 8, NULL,
              tskIDLE_PRIORITY + 2U, NULL);
  // xTaskCreate(vEthernetTask, "ethernet", 1024 * 8, NULL, tskIDLE_PRIORITY +
  // 1U,
  //             NULL);
  // Start scheduler
  osKernelStart();

  // We should never get here as control is now taken by the scheduler
  while (1) {
  }
}
static void vStepperTask1(void *argument) {
  // INit stepper
  pin_t pin1 = {GPIOA, GPIO_PIN_4};
  pin_t pin2 = {GPIOC, GPIO_PIN_0};
  init_stepper(&stepper1, 50, &htim2, pin1, pin2);
  osDelay(300);
  /* Declare the variable that will hold the values received from the
     queue. */
  arm_stepper_signals signals;
  BaseType_t xStatus;

  /* This task is also defined within an infinite loop. */
  while (1) {

    xStatus = xQueueReceive(xQueueStepper1, &signals, portMAX_DELAY);

    if (xStatus == pdPASS) {
      LOGI(TAG, "Received = %u", signals);

      LOGI(TAG, "freq: %u", signals.stepper_freq);
      LOGI(TAG, "steps: %u", signals.stepper_steps);

      stepper1_count += signals.stepper_steps;

      rotate_stepper(&stepper1, signals.stepper_steps, signals.stepper_freq);
    }
  }
}

static void vStepperTask2(void *argument) {
  // Init stepper
  pin_t pin3 = {GPIOA, GPIO_PIN_5};
  pin_t pin4 = {GPIOB, GPIO_PIN_6};
  init_stepper(&stepper2, 50, &htim3, pin3, pin4);
  osDelay(300);
  /* Declare the variable that will hold the values received from the
     queue. */
  BaseType_t xStatus;
  arm_stepper_signals signals;
  /* This task is also defined within an infinite loop. */
  while (1) {

    xStatus = xQueueReceive(xQueueStepper2, &signals, portMAX_DELAY);

    if (xStatus == pdPASS) {
      LOGI(TAG, "Received = %u", signals);

      LOGI(TAG, "freq: %u", signals.stepper_freq);
      LOGI(TAG, "steps: %u", signals.stepper_steps);

      stepper2_count += signals.stepper_steps;

      rotate_stepper(&stepper2, signals.stepper_steps, signals.stepper_freq);
    }
  }
}

float delta = 0.01;
static void position_setter() {

  if (rtU.x >= 0.85) {
    delta = -0.01;
  } else if (rtU.x <= 0.75) {
    delta = 0.01;
  }
  rtU.x = rtU.x + delta;

  LOGI(TAG, "rtU.x: %f", rtU.x);
}

uint32_t old_time;
static void vArmController(void *argument) {

  pin_t pin3 = {GPIOA, GPIO_PIN_5};
  pin_t pin4 = {GPIOB, GPIO_PIN_6};
  pin_t pin1 = {GPIOA, GPIO_PIN_4};
  pin_t pin2 = {GPIOC, GPIO_PIN_0};
  init_stepper(&stepper1, 50, &htim2, pin1, pin2);
  init_stepper(&stepper2, 50, &htim3, pin3, pin4);
  int steps;
  int freq;

  while (1) {
    position_setter();
    if (old_time == NULL) {
      rtU.deltaTime = 2 / 1000.0;
    } else {
      rtU.deltaTime = (osKernelGetTickCount() - old_time) / 1000.0;
    }
    rtU.stepperLeftActualPosition =
        rtY.stepperLeftSteps; // stepper count - keep count
    rtU.stepperRightActualPosition =
        rtY.stepperRightSteps; // stepper count - keep count

    old_time = osKernelGetTickCount();
    control_arm_manual_step();

    rtY.stepperLeftFrequency;
    rtY.stepperRightFrequency;
    rtY.controlGripperPitch;
    LOGI(TAG, "gripper pitch: %f", rtY.controlGripperPitch);
    cubemars_ak_set_speed(&hfdcan1, 111, 100);
    CAN_LogStatus(&hfdcan1);
    if (rtY.stepperLeftSteps == 0 || rtY.stepperRightSteps == 0) {
      __builtin_trap();
      while (true) {
      };
    }
    steps = rtY.stepperLeftSteps;
    freq = 50; // rtY.stepperLeftFrequency;
    LOGI(TAG, "LEFT freq: %u", freq);
    LOGI(TAG, "LEFT steps: %u", steps);
    // steps = -100;
    // rotate_stepper(&stepper1, steps, freq);

    steps = rtY.stepperRightSteps;
    LOGI(TAG, "RIGHT freq: %u", freq);
    LOGI(TAG, "RIGHT steps: %u", steps);
    // steps = 100;
    // rotate_stepper(&stepper2, steps, freq);

    while (htim2.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
      osDelay(1); // Delay for thread switching
    }
    while (htim3.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
      osDelay(1); // Delay for thread switching
    }

    // if (old_time > 8000) {
    //   BaseType_t xStatus1;
    //   xStatus1 = xQueueSendToBack(xQueueStepper1, &ss_left,
    //                               0); //! TODO: wait how many seconds?
    //
    //   if (xStatus1 != pdPASS) {
    //     LOGE(TAG, "Could not send into queue1 (probably full)");
    //   }
    //
    //   BaseType_t xStatus2;
    //   xStatus2 = xQueueSendToBack(xQueueStepper2, &ss_right,
    //                               0); //! TODO: wait how many seconds?
    //
    //   if (xStatus2 != pdPASS) {
    //     LOGE(TAG, "Could not send into queue2 (probably full)");
    //   }
    // }

    // BLDC

    BaseType_t xStatus3;
    // xStatus3 = xQueueSendToBack(xQueueBase, &rtY.baseControl,
    //                         0); //! TODO: wait how many seconds?

    // if (xStatus3 != pdPASS) {
    //   LOGE(TAG, "Could not send into queue3
    //   (probably full)");
    // }
  }
}

static void vArmInTask(void *argument) {
  while (1) {
    osDelay(10 * 1000); // every 10 seconds

    // Steppers

    arm_stepper_signals ss_left = {(uint32_t)rtY.stepperLeftSteps -
                                       stepper1_count,
                                   (uint32_t)rtY.stepperLeftFrequency};
    arm_stepper_signals ss_right = {(uint32_t)rtY.stepperRightSteps -
                                        stepper2_count,
                                    (uint32_t)rtY.stepperRightFrequency};

    if (old_time > 8000) {
      BaseType_t xStatus1;
      xStatus1 = xQueueSendToBack(xQueueStepper1, &ss_left,
                                  0); //! TODO: wait how many seconds?

      if (xStatus1 != pdPASS) {
        LOGE(TAG, "Could not send into queue1 (probably full)");
      }

      BaseType_t xStatus2;
      xStatus2 = xQueueSendToBack(xQueueStepper2, &ss_right,
                                  0); //! TODO: wait how many seconds?

      if (xStatus2 != pdPASS) {
        LOGE(TAG, "Could not send into queue2 (probably full)");
      }
    }

    // BLDC
  }
}

static void pwm_scope_task(void *argument) {
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
void HandlePacket(receive_frame_t *receive_frame) {
  LOGI(TAG, "Wayoo, message received");
}

/* Config for 1 pbmessage: ArmBoardControlSignals */
static result_t Callback_ArmBoardControlSignals(void *buffer) {

  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  Arm_ControlSignals *pckt = (Arm_ControlSignals *)buffer;

  int32_t steps1 = 50;

  BaseType_t xStatus;
  xStatus = xQueueSendToBack(xQueueStepper1, &steps1,
                             0); //! TODO: wiat how many seconds?

  if (xStatus != pdPASS) {
    LOGE(TAG, "Could not send into queue (probably full)");
  }

  return RESULT_OK;
}

Callback_BaseStationManualArmControl(
    void *buffer) { // define callback met de callback signature
  BasestationManualArmMovement *pckt = (BasestationManualArmMovement *)buffer;

  rtU.x += pckt->delta_x / (double)(1ULL << 31) * 0.01;
  rtU.y += pckt->delta_x / (double)(1ULL << 31) * 0.01;
  rtU.z += pckt->delta_x / (double)(1ULL << 31) * 0.01;
  rtU.gripperAng +=
      pckt->delta_final_gripper_angle / (double)(1ULL << 31) * 0.005;
  rtU.gripperPitchActualPosition; // get from can
}
// PACKET_HANDLER_CONFIG_STATIC(Handler_ArmBoardControlSignals,
// PBEnvelope_arm_ctrl_tag, arm_ctrl, Callback_ArmBoardControlSignals);

/*Init and pass packet dispatcher*/
static uint8_t Handle_ArmBoardControlSignals_queue_buffer
    [PACKET_HANDLER_DEFAULT_QUEUE_LENGTH *
     sizeof(((PBEnvelope *)0)->payload.arm_ctrl)];
static uint8_t Handle_BaseStationManualArmControl_queue_buffer
    [PACKET_HANDLER_DEFAULT_QUEUE_LENGTH *
     sizeof(((PBEnvelope *)0)->payload.manual_arm)];
// These are found in handler_stuff.h
static packet_handler_config_t handlers[] = {
    {
        .handler = (Callback_ArmBoardControlSignals),
        .task_name = "Handle_ArmBoardControlSignals",
        .packet_type = (PBEnvelope_arm_ctrl_tag),
        .task_priority = PACKET_HANDLER_DEFAULT_PRIORITY,
        .task_stack_depth = PACKET_HANDLER_DEFAULT_STACK_DEPTH,
        .item_size = sizeof(((PBEnvelope *)0)->payload.arm_ctrl),
        .queue_length = PACKET_HANDLER_DEFAULT_QUEUE_LENGTH,
        .queue_buffer = Handle_ArmBoardControlSignals_queue_buffer,
        .queue_struct = {0},
        .queue = NULL,
    },
    {
        .handler = (Callback_BaseStationManualArmControl),
        .task_name = "Handle_BaseStationManualArmControl",
        .packet_type = (PBEnvelope_manual_arm_tag),
        .task_priority = PACKET_HANDLER_DEFAULT_PRIORITY,
        .task_stack_depth = PACKET_HANDLER_DEFAULT_STACK_DEPTH,
        .item_size = sizeof(((PBEnvelope *)0)->payload.manual_arm),
        .queue_length = PACKET_HANDLER_DEFAULT_QUEUE_LENGTH,
        .queue_buffer = Handle_BaseStationManualArmControl_queue_buffer,
        .queue_struct = {0},
        .queue = NULL,
    }};

extern int receiving_counter;
int outgoing_counter = 0;

void vEthernetTask(void *argument) {
  // Setup using sending side params

  /*Making queues*/
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

  PacketDispatcherInit(handlers, 1);
  ETH_udp_init(2, queues, HandlePacket);

  /*Config + add ARP receiving side*/
  ETH_add_arp(ip, mac, 5);

  /*Sending a message*/
  uint8_t packet1_payload[4] = {14, 06, 20, 04};

  /*Test sending*/
  // while (outgoing_counter <
  //        100) { // NOTE: after 80 packages the queue will be full!
  //   ETH_udp_send(ip, 8, packet1_payload, 4, 1);
  //   outgoing_counter += 1;
  //   LOGI(TAG, "%d", outgoing_counter);
  //   osDelay(5000);
  // }

  while (1) {
    LOGI(TAG, "...ethernet still receiving");
    osDelay(3000);
  }
}
