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

  //--------------------------- CAN CONFIGS
  CAN_ConfigRx_AllStandard();
  if (HAL_FDCAN_ConfigInterruptLines(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, FDCAN_INTERRUPT_LINE0) != HAL_OK) {
    LOGE("CAN", "Interrupt line config failed err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
    LOGE("CAN", "Activate RX notification failed err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
    LOGE(TAG, "FDCAN start failed, err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
    while(1){}
  }

  LOGI("CAN", "Mode=%lu Presc=%lu TS1=%lu TS2=%lu SJW=%lu", hfdcan1.Init.Mode, hfdcan1.Init.NominalPrescaler, hfdcan1.Init.NominalTimeSeg1, hfdcan1.Init.NominalTimeSeg2, hfdcan1.Init.NominalSyncJumpWidth);

  //--------------------------- QUEUES  
  xQueueStepperLeft = xQueueCreate(5, sizeof(Arm_StepperSignals_size));
  if (xQueueStepperLeft == NULL) {
    //!TODO: HANDLE
    LOGE(TAG, "Queue could not be created");
  }

  xQueueStepperRight = xQueueCreate(5, sizeof(Arm_StepperSignals_size));
  if (xQueueStepperRight == NULL) {
    //!TODO: HANDLE
    LOGE(TAG, "Queue could not be created");
  }

  //--------------------------- TASKS
  xTaskCreate(vArmController, "ArmController", 1024*8, NULL, tskIDLE_PRIORITY + 2U, NULL);
  xTaskCreate(vWristController, "WristController", 1024*8, NULL, tskIDLE_PRIORITY + 3U, NULL);

  //Receiving tasks, prio is one above sending task so it should always empty the queue when msgs are there
  xTaskCreate(vStepperTaskLeft, "ReceiverLeft", 1024*8, NULL, tskIDLE_PRIORITY + 4U, NULL);
  xTaskCreate(vStepperTaskRight, "ReceiverRight", 1024*8, NULL, tskIDLE_PRIORITY + 5U, NULL);

  // Start scheduler
  osKernelStart();

  // We should never get here as control is now taken by the scheduler
  while (1) {}
}

stepper_t stepperLeft;
static void vStepperTaskLeft(void* argument) {
  //Init stepper
  pin_t pin1 = {GPIOA, GPIO_PIN_4};
  pin_t pin2 = {GPIOC, GPIO_PIN_0};
  init_stepper(&stepperLeft, 50, &htim2, pin1, pin2);

  /* Declare the variable that will hold the values received from the
     queue. */
  void* buffer;
  BaseType_t xStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(10); //Queue checks receiving every 10 ms

  /* This task is also defined within an infinite loop. */
  while(1) {
    if (uxQueueMessagesWaiting(xQueueStepperLeft) != 0) {
      LOGE(TAG, "Queue is not empty!\r\n");
    }

    xStatus = xQueueReceive(xQueueStepperLeft, &buffer, xTicksToWait);

    if (xStatus == pdPASS) {
      LOGI(TAG, "Received = %u", buffer);

      Arm_StepperSignals* decoded_ss = Arm_StepperSignals_DEFAULT;
      size_t size = Arm_StepperSignals_size;
      result_t res = pb_message_decode(buffer, Arm_StepperSignals_size, Arm_StepperSignals_fields, Arm_StepperSignals_size, &decoded_ss);

      LOGI(TAG, "freq: %u", decoded_ss->stepper_freq);
      LOGI(TAG, "steps: %u", decoded_ss->stepper_steps);

      rotate_stepper(&stepperLeft, decoded_ss->stepper_steps, decoded_ss->stepper_freq);

      TIM_HandleTypeDef* htim = stepperLeft.htim;

    }

    osDelay(1); // Delay for thread switching

  }
}

stepper_t stepperRight;
static void vStepperTaskRight(void* argument) {
  //Init stepper
  pin_t pin1 = {GPIOA, GPIO_PIN_5};
  pin_t pin2 = {GPIOB, GPIO_PIN_6};
  init_stepper(&stepperRight, 50, &htim3, pin1, pin2);

  /* Declare the variable that will hold the values received from the queue. */
  void* buffer;
  BaseType_t xStatus;
  const TickType_t xTicksToWait = pdMS_TO_TICKS(10); //Queue checks receiving every 10 ms

  /* This task is also defined within an infinite loop. */
  while(1) {
    if (uxQueueMessagesWaiting(xQueueStepperRight) != 0) {
      LOGE(TAG, "Queue is not empty!\r\n");
    }

    xStatus = xQueueReceive(xQueueStepperRight, &buffer, xTicksToWait);

    if (xStatus == pdPASS) {
      LOGI(TAG, "Received = %u", buffer);

      Arm_StepperSignals* decoded_ss = Arm_StepperSignals_DEFAULT;
      size_t size = Arm_StepperSignals_size;
      result_t res = pb_message_decode(buffer, Arm_StepperSignals_size, Arm_StepperSignals_fields, Arm_StepperSignals_size, &decoded_ss);

      LOGI(TAG, "freq: %u", decoded_ss->stepper_freq);
      LOGI(TAG, "steps: %u", decoded_ss->stepper_steps);

      rotate_stepper(&stepperRight, decoded_ss->stepper_steps, decoded_ss->stepper_freq);

      TIM_HandleTypeDef* htim = stepperRight.htim;

    }

    osDelay(1); // Delay for thread switching

  }
}

float delta = 0.05;
float delta_gripper = 0.001;

//HARDCODED STATE SETTING FOR TESTING
static void position_setter(int state) {

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

  LOGI(TAG, "rtU.x: %f", rtU.x);
}

uint32_t old_time;

const float start_gripper_angle = 0.9;

static void vWristController(void *argument) {

  const float d_t = 0.01;
  const float speed = 0.01 * d_t;
  float current_pos = start_gripper_angle;
  while (true) {
    if (rtY.controlGripperPitch != 0) {
      float setpoint = rtY.controlGripperPitch * 10 * (M_PI / 180) + start_gripper_angle;
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
  pin_t pin1 = {GPIOA, GPIO_PIN_4};
  pin_t pin2 = {GPIOC, GPIO_PIN_0};
  init_stepper(&stepper1, 50, &htim2, pin1, pin2);

  pin_t pin3 = {GPIOA, GPIO_PIN_5};
  pin_t pin4 = {GPIOB, GPIO_PIN_6};
  init_stepper(&stepper2, 50, &htim3, pin3, pin4);

  osDelay(1000);
  int32_t steps;
  int freq;

  //if calibration off, move cubemars to start_gripper_angle to keep wrist in place while doing movements
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

  int state = 0;
  while (1) {
    position_setter(state); //set positions

    if (old_time == NULL) {
      rtU.deltaTime = 2 / 1000.0;
    } else {
      rtU.deltaTime = (osKernelGetTickCount() - old_time) / 1000.0;
    }

    // update positions
    rtU.stepperLeftActualPosition = rtY.stepperLeftSteps; 
    rtU.stepperRightActualPosition = rtY.stepperRightSteps;

    old_time = osKernelGetTickCount();

    //do actual control, calling rowans control code :)
    control_arm_manual_step(); 

    //rtY = outputs
    //The three below are the ones we can use for now
    rtY.stepperLeftFrequency;
    rtY.stepperRightFrequency;
    rtY.controlGripperPitch;

    //log gripper
    LOGI(TAG, "gripper pitch: %i", rtY.controlGripperPitch * 10 * (M_PI / 180));
    CAN_LogStatus(&hfdcan1);

    //TODO: should be rtY.stepperLeft/RightFrequency but rowan needs to bug fix
    freq = 200;
    int32_t stepsR;
    int32_t stepsL;
    switch (calibration) {
      case 0: //off, the steppers move the calulated amount
        stepsR = rtY.stepperRightSteps;
        rotate_stepper(&stepper2, stepsR, freq);

        stepsL = rtY.stepperLeftSteps;
        rotate_stepper(&stepper1, -stepsL, freq);

        break;
      case 1: //left, the left stepper moves 10.000
        stepsL = -10000;
        rotate_stepper(&stepper1, -stepsL, freq);
        break;
      case 2: //right, the right stepper moves 10.000
        stepsR = 10000;
        rotate_stepper(&stepper2, stepsR, freq);
        break;
      case 3: //both, both steppers move 10.000 
        stepsR = 10000;
        rotate_stepper(&stepper2, stepsR, freq);

        stepsL = -10000;
        rotate_stepper(&stepper1, -stepsL, freq);
        break;
    }


    // Arm_StepperSignals ss_right = {stepsR,freq};
    // Arm_StepperSignals ss_left = {stepsL,freq};

    // uint8_t* ss_right_enc = NULL;
    // size_t msg_size = 0;
    // pb_message_encode(&ss_right,Arm_StepperSignals_fields,&ss_right_enc,&msg_size);
    // //!TODO: error handling
    // uint8_t* ss_left_enc = NULL;
    // size_t msg_size1 = 0;
    // pb_message_encode(&ss_left,Arm_StepperSignals_fields,&ss_left_enc,&msg_size1);
    // //!TODO: error handling

    // BaseType_t xStatus1 = xQueueSendToBack(xQueueStepperRight, &ss_right_enc, 0); //!TODO: wait how many seconds?
    // if (xStatus1 != pdPASS) {
    //   LOGE(TAG, "Could not send into queue Reciever Right (probably full)");
    // }
    // BaseType_t xStatus2 = xQueueSendToBack(xQueueStepperLeft, &ss_left_enc, 0); //!TODO: wait how many seconds?
    // if (xStatus2 != pdPASS) {
    //   LOGE(TAG, "Could not send into queue Reciever Right (probably full)");
    // }

    osDelay(1);
  }
}

// void vEthernetTask(void *argument) {
//   // Setup using sending side params

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

//   PacketDispatcherInit(handlers, 1);
//   ETH_udp_init(2, queues, HandlePacket);

//   /*Config + add ARP receiving side*/
//   ETH_add_arp(ip, mac, 5);

//   /*Sending a message*/
//   uint8_t packet1_payload[4] = {14, 06, 20, 04};

//   /*Test sending*/
//   // while (outgoing_counter <
//   //        100) { // NOTE: after 80 packages the queue will be full!
//   //   ETH_udp_send(ip, 8, packet1_payload, 4, 1);
//   //   outgoing_counter += 1;
//   //   LOGI(TAG, "%d", outgoing_counter);
//   //   osDelay(5000);
//   // }

//   while (1) {
//     LOGI(TAG, "...ethernet still receiving");
//     osDelay(3000);
//   }
// }
