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

#include "cmsis_os2.h"
#include "cubemx_main.h"
#include "gpio.h"

// protobuffers
#include "components/arm_board/movement_control_in.pb.h"
// common libraries
#include "logging.h"
#include "result.h"

// networking
#include "components/common/networking/inc/ethernet.h" //long path since LWIP also has ethernet.h
#include "ethernet_udp.h"
#include "networking_constants.h"

// packet_dispatcher
#include "can.h"
#include "cubemars_ak.h"
#include "packet_dispatcher.h"
#include "packet_dispatcher_macros.h"

#define TAG "ARM_BOARD"

/*Handles*/
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
void setup_control_parameters() {
  rtU.x = 0.795;
  rtU.y = 0.0;
  rtU.z = 0.322;
  rtU.gripperAng = 5 * (M_PI / 180);
  rtU.gripperPitchActualPosition = 0; // input can
  rtU.stepperLeftActualPosition = 0;
  rtU.stepperRightActualPosition = 0;
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
  // MX_FDCAN2_Init();

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
  xTaskCreate(vWristController, "WristController", 1024 * 8, NULL,
              tskIDLE_PRIORITY + 3U, NULL);
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

float delta = 0.05;
float delta_gripper = 0.001;
int state = 0;
static void position_setter() {

  // if (rtU.x >= 0.85) {
  //   delta = abs(delta);
  // } else if (rtU.x <= 0.75) {
  //   delta = -abs(delta);
  // }
  switch (state) {
  case 0:
    rtU.x = rtU.x - delta * 1.1;
    rtU.z = rtU.z - delta;
    if ((rtU.x <= 0.7 || rtU.z <= 0.099) && state == 0) {
      state = 1;
    }
    break;
  case 1:
    rtU.gripperAng += delta_gripper;
    if (rtU.gripperAng >= 0.2 * M_PI && state == 1) {
      state = 2;
    }
    break;
  case 2:
    // rtU.x += delta * 1.1;
    // if (rtU.x >= 0.9) {
    //   state = 3;
    // }
    // break;
  case 3:
    rtU.z += delta * 1;
    if (rtU.z > 0.2) {
      state = 4;
    }
    break;
  case 4:
    rtU.z -= delta;
    if (rtU.z < 0.1) {
      state = 3;
    }
    break;
  }
  LOGI(TAG, "state: %i", state);
  LOGI(TAG, "set x: %f", rtU.x);
  // if (rtU.x >= 0.85) {
  //   delta = abs(delta);
  // } else if (rtU.x <= 0.75) {
  //   delta = -abs(delta);
  // }
  // if (rtU.gripperAng >= 0.2) {
  //   delta_gripper = abs(delta_gripper);
  // } else if (rtU.gripperAng <= 15) {
  //   delta_gripper = -abs(delta_gripper);
  // }
  // rtU.gripperAng += delta;
  LOGI(TAG, "rtU.x: %f", rtU.x);
}

uint32_t old_time;

const float start_gripper_angle = 0.9;

#define calibration 0 // 0 = off, 1 = left, 2 = right, 3 = both

static void vWristController(void *argument) {

  const float d_t = 0.01;
  const float speed = 0.01 * d_t;
  float current_pos = start_gripper_angle;
  while (true) {
    if (rtY.controlGripperPitch != 0) {
      float setpoint =
          rtY.controlGripperPitch * 10 * (M_PI / 180) + start_gripper_angle;
      float diff = setpoint - current_pos;
      float real_speed;
      if (diff < speed) {
        real_speed = diff;
      } else {
        real_speed = speed;
      }
      current_pos += real_speed;
      if (calibration == 0) {
        cubemars_ak_set_position(&hfdcan1, 111, current_pos);
      }
    }
    osDelay(d_t * 1000);
  }
}
static void vArmController(void *argument) {

  pin_t pin3 = {GPIOA, GPIO_PIN_5};
  pin_t pin4 = {GPIOB, GPIO_PIN_6};
  pin_t pin1 = {GPIOA, GPIO_PIN_4};
  pin_t pin2 = {GPIOC, GPIO_PIN_0};
  init_stepper(&stepper1, 50, &htim2, pin1, pin2);
  init_stepper(&stepper2, 50, &htim3, pin3, pin4);
  int32_t steps;
  int freq;
  osDelay(5000);
  if (calibration == 0) {
    for (float f = 0; f < start_gripper_angle; f += 0.005) {
      cubemars_ak_set_position(&hfdcan1, 111, f);
      LOGI(TAG, "gripper angle: %f", f);
      osDelay(10);
    }

    for (int i = 0; i < 100; i++) {
      cubemars_ak_set_position(&hfdcan1, 111, start_gripper_angle);
      osDelay(10);
    }
  }

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
    LOGI(TAG, "gripper pitch: %i", rtY.controlGripperPitch * 10 * (M_PI / 180));
    CAN_LogStatus(&hfdcan1);
    // if (rtY.stepperLeftSteps == 0 && rtY.stepperRightSteps == 0) {
    //   LOGI(TAG, "ERROR EVEYRTHING 0");
    //   osDelay(10);
    //   __builtin_trap();
    //   while (true) {
    //   };
    // }

    steps = rtY.stepperRightSteps;
    LOGI(TAG, "RIGHT freq: %u", freq);
    LOGI(TAG, "RIGHT steps: %i", steps);
    if (calibration == 2 || calibration == 3) {
      steps = 10000;
    }
    if (calibration == 0 || calibration == 2 || calibration == 3) {
      rotate_stepper(&stepper2, steps, freq);
    }
    steps = rtY.stepperLeftSteps;
    freq = 200; // rtY.stepperLeftFrequency;
    LOGI(TAG, "LEFT freq: %u", freq);
    LOGI(TAG, "LEFT steps: %i", steps);
    if (calibration == 1 || calibration == 3) {
      steps = -10000;
    }
    if (calibration == 0 || calibration == 1 || calibration == 3) {
      rotate_stepper(&stepper1, -steps, freq);
    }

    while (htim2.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
      osDelay(1); // Delay for thread switching
    }
    while (htim3.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
      osDelay(1); // Delay for thread switching
    }
    osDelay(10);

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
