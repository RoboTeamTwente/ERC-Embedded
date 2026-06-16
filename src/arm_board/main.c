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
#include "cmsis_os2.h"
#include "erc-control-arm/control_arm_ert_rtw/control_arm.h"

#include "cubemx_main.h"
#include "erc-control-arm/control_arm_ert_rtw/rtwtypes.h"
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
osThreadId_t testEthernetTaskHandle;

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
static void vWristController(void *argument);

void setup_control_parameters() {
    rtU.x = 0.795;
    rtU.y = 0.0;
    rtU.z = 0.322;
    rtU.gripperAng = 5 * (M_PI / 180);
    rtU.gripperPitchOldPosition = 0;
    rtU.baseOldPosition = 0;
    rtU.stepperLeftOldPosition = 0;
    rtU.stepperRightOldPosition = 0;
    rtU.timePerMovement = 10;
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
        LOGE("CAN", "Filter config failed, err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
        Error_Handler();
    }

    if (HAL_FDCAN_ConfigGlobalFilter(
        &hfdcan1,
        FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
        FDCAN_REJECT_REMOTE,
        FDCAN_REJECT_REMOTE) != HAL_OK) {
        LOGE("CAN", "Global filter failed, err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
        Error_Handler();
    }
}

void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes) {
    LOGI("CAN", "TX complete buffers=0x%08lx\n", BufferIndexes);
}

void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes) {
    LOGI("CAN", "TX abort buffers=0x%08lx\n", BufferIndexes);
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
    LOGE("CAN", "Error callback HALerr=0x%08lx\n", HAL_FDCAN_GetError(hfdcan));
}

static cubemars_ak_information motor_info = {0};

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0) {
        return;
    }

    FDCAN_RxHeaderTypeDef rx_header = {0};
    uint8_t rx_data[8] = {0};

    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        LOGE("CAN", "RX read failed, err=0x%08lx", HAL_FDCAN_GetError(hfdcan));
        return;
    }
    cubemars_ak_parse_can_feedback(&rx_header, rx_data, &motor_info);
}

// static void CAN2_ConfigRx_AllStandard(void) {
//   FDCAN_FilterTypeDef filter = {0};
//
//   filter.IdType = FDCAN_STANDARD_ID;
//   filter.FilterIndex = 0;
//   filter.FilterType = FDCAN_FILTER_MASK;
//   filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
//
//   // Accept everything: (ID & 0x000) == (0x000 & 0x000)
//   filter.FilterID1 = 0x000;
//   filter.FilterID2 = 0x000;
//
//   if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter) != HAL_OK) {
//     LOGE("CAN", "Filter config failed, err=0x%08lx",
//          HAL_FDCAN_GetError(&hfdcan2));
//     Error_Handler();
//   }
//
//   if (HAL_FDCAN_ConfigGlobalFilter(
//           &hfdcan2, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
//           FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
//     LOGE("CAN", "Global filter failed, err=0x%08lx",
//          HAL_FDCAN_GetError(&hfdcan2));
//     Error_Handler();
//   }
// }

static void CAN_LogStatus(FDCAN_HandleTypeDef *hfdcan) {
    FDCAN_ProtocolStatusTypeDef protocol_status;
    FDCAN_ErrorCountersTypeDef error_counters;

    if (HAL_FDCAN_GetProtocolStatus(hfdcan, &protocol_status) == HAL_OK) {
        LOGI("CAN",
             "LastErrorCode=%lu DataLastErrorCode=%lu Activity=%lu BusOff=%lu",
             protocol_status.LastErrorCode,
             protocol_status.DataLastErrorCode,
             protocol_status.Activity,
             protocol_status.BusOff);
    }

    if (HAL_FDCAN_GetErrorCounters(hfdcan, &error_counters) == HAL_OK) {
        LOGI("CAN",
             "TxErrorCnt=%lu RxErrorCnt=%lu RxErrorPassive=%lu",
             error_counters.TxErrorCnt,
             error_counters.RxErrorCnt,
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
        for (;;);
    }
    LOGI("CAN",
         "Mode=%lu Presc=%lu TS1=%lu TS2=%lu SJW=%lu", hfdcan1.Init.Mode,
         hfdcan1.Init.NominalPrescaler, hfdcan1.Init.NominalTimeSeg1,
         hfdcan1.Init.NominalTimeSeg2, hfdcan1.Init.NominalSyncJumpWidth);

    xTaskCreate(vArmController, "ArmController", 1024 * 8, NULL, tskIDLE_PRIORITY + 2U, NULL);
    xTaskCreate(vWristController, "WristController", 1024 * 8, NULL, tskIDLE_PRIORITY + 3U, NULL);

    // Start scheduler
    osKernelStart();

    // We should never get here as control is now taken by the scheduler
    while (1) {}
}

int state = 0;
bool reachedPosition = false;

static void position_setter() {
    switch (state) {
    case 0:
        //setting new position
        rtU.x = 0.50;
        rtU.y = 0.0;
        rtU.z = 0.05;
        rtU.gripperAng = 90 * (M_PI / 180);
        state = 5;
        break;
    case 5:
        //waiting for movement to finish
        if(reachedPosition){
            rtU.gripperPitchOldPosition = rtY.controlGripperPitch;
            rtU.baseOldPosition = rtY.controlBase;
            rtU.stepperLeftOldPosition = rtY.stepperLeftSteps;
            rtU.stepperRightOldPosition = rtY.stepperRightSteps;
            state = 10;
        }
        else {
            return;
        }
        break;
    case 10:
        //setting new position
        rtU.gripperAng = 80 * (M_PI / 180);
        reachedPosition = false;
        state = 20;
        break;
    case 20:
        //waiting for movement to finish
        if(reachedPosition){
            rtU.gripperPitchOldPosition = rtY.controlGripperPitch;
            rtU.baseOldPosition = rtY.controlBase;
            rtU.stepperLeftOldPosition = rtY.stepperLeftSteps;
            rtU.stepperRightOldPosition = rtY.stepperRightSteps;
            state = 30;
        }
        else {
            return;
        }
        break;
    case 30:
        //setting new position
        rtU.gripperAng = 100 * (M_PI / 180);
        reachedPosition = false;
        state = 40;
        break;
    case 40:
        //waiting for movement to finish
        if(reachedPosition){
            rtU.gripperPitchOldPosition = rtY.controlGripperPitch;
            rtU.baseOldPosition = rtY.controlBase;
            rtU.stepperLeftOldPosition = rtY.stepperLeftSteps;
            rtU.stepperRightOldPosition = rtY.stepperRightSteps;
            state = 10;
        }
        else {
            return;
        }
        break;
    case 500:
        LOGI(TAG, "reached final position");
        break;
    }

    LOGI(TAG, "state: %i", state);
    LOGI(TAG, "rtU.x: %f", rtU.x);
    LOGI(TAG, "rtU.y: %f", rtU.y);
    LOGI(TAG, "rtU.z: %f", rtU.z);
    LOGI(TAG, "rtU.gripperAng: %f", rtU.gripperAng);
    control_arm_step();
}

uint32_t old_time;

bool startWristControl = false;
const float start_gripper_angle = 0.94;

#define calibration 0          // 0 = off, 1 = right, 2 = left, 3 = both, 4 = wrist motor
#define calibrationDirection 1 // 0 = clockwise, 1 = counter clockwise
#define calibrationSpeed 100   // fequency when calibrating

static void vWristController(void *argument) {
    //const float d_t = 0.001;
    //const float speed = 0.01 * d_t;
    //float current_pos = start_gripper_angle;
    //doing nothing before wrist position is initialized, and if calibrating the stepper motors
    while (!startWristControl || calibration != 0) {
        osDelay(1000);
    }

    while (1) {
        control_arm_step();
        float setpoint = (rtY.controlGripperPitch/2) + start_gripper_angle;
        //LOGI(TAG, "wrist pitch position: %f", setpoint);
        cubemars_ak_set_position(&hfdcan1, 111, setpoint);
        osDelay(10);
    }
}

static void vArmController(void *argument) {
    pin_t pin1 = {GPIOA, GPIO_PIN_4};
    pin_t pin2 = {GPIOC, GPIO_PIN_0};
    pin_t pin3 = {GPIOA, GPIO_PIN_5};
    pin_t pin4 = {GPIOB, GPIO_PIN_6};
    init_stepper(&stepper1, 50, &htim2, pin1, pin2);
    init_stepper(&stepper2, 50, &htim3, pin3, pin4);
    int32_t steps;
    int freq;
    osDelay(5000);

    //only calibrating while calibration is enabled
    while(calibration != 0){
        steps = 10000;
        freq = calibrationSpeed;
        if(calibrationDirection){
            steps *= -1;
        }

        if(calibration == 1){
            LOGI(TAG, "calibration1");
            LOGI(TAG, "steps = %d, freq = %d", steps, freq);
            rotate_stepper(&stepper2, steps, freq);
        }
        else if(calibration == 2){
            LOGI(TAG, "calibration2");
            LOGI(TAG, "steps = %d, freq = %d", steps, freq);
            rotate_stepper(&stepper1, -steps, freq);
        }
        else if(calibration == 3){
            LOGI(TAG, "calibration3");
            LOGI(TAG, "steps = %d, freq = %d", steps, freq);
            rotate_stepper(&stepper2, steps, freq);
            rotate_stepper(&stepper1, -steps, freq);
        }
        else if(calibration == 4){
            float delta = 0.0005;
            float currentPos = 0;
            if(calibrationDirection){
                delta *= -1;
            }
            while (true) {
                cubemars_ak_set_position(&hfdcan1, 111, currentPos);
                LOGI(TAG, "gripper angle: %f", currentPos);
                currentPos += delta;
                osDelay(10);
            }
        }

        while (htim2.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
            osDelay(1); // Delay for thread switching
        }
        while (htim3.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
            osDelay(1); // Delay for thread switching
        }

        osDelay(100);
    }

    //setting gripper angle to initial position
    for (float f = 0; f < start_gripper_angle; f += 0.005) {
        cubemars_ak_set_position(&hfdcan1, 111, f);
        LOGI(TAG, "gripper angle: %f", f);
        osDelay(10);
    }
    for (int i = 0; i < 100; i++) {
        cubemars_ak_set_position(&hfdcan1, 111, start_gripper_angle);
        osDelay(10);
    }
    startWristControl = true;

    rtU.timePerMovement = 5;
    while (1) {
        position_setter();
        //only doing something if the position is not reached
        if(reachedPosition){
            continue;
        }

        rtU.deltaTime = 0.02;
        //rtU.stepperLeftActualPosition = rtY.stepperLeftSteps; // stepper count - keep count
        //rtU.stepperRightActualPosition = rtY.stepperRightSteps; // stepper count - keep count

        old_time = osKernelGetTickCount();

        rtY.stepperLeftFrequency;
        rtY.stepperRightFrequency;
        rtY.controlGripperPitch;
        LOGI(TAG, "gripper pitch: %f", rtY.controlGripperPitch);
        CAN_LogStatus(&hfdcan1);
        // if (rtY.stepperLeftSteps == 0 && rtY.stepperRightSteps == 0) {
        //   LOGI(TAG, "ERROR EVEYRTHING 0");
        //   osDelay(10);
        //   __builtin_trap();
        //   while (true) {
        //   };
        // }

        steps = rtY.stepperRightSteps;
        freq = rtY.stepperRightFrequency;
        LOGI(TAG, "RIGHT steps: %d", steps);
        LOGI(TAG, "RIGHT freq: %d", freq);
        rotate_stepper(&stepper1, steps, freq);

        steps = rtY.stepperLeftSteps;
        freq = rtY.stepperLeftFrequency;
        LOGI(TAG, "LEFT steps: %d", steps);
        LOGI(TAG, "LEFT freq: %d", freq);
        rotate_stepper(&stepper2, -steps, freq);

        while (htim2.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
            osDelay(1); // Delay for thread switching
        }
        while (htim3.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
            osDelay(1); // Delay for thread switching
        }
        reachedPosition = true;
        osDelay(10);
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

    ArmBoardControlSignals *pckt = (ArmBoardControlSignals *)buffer;

    int32_t steps1 = 50;

    BaseType_t xStatus;
    xStatus = xQueueSendToBack(xQueueStepper1, &steps1, 0); //! TODO: wiat how many seconds?

    if (xStatus != pdPASS) {
        LOGE(TAG, "Could not send into queue (probably full)");
    }

    return RESULT_OK;
}

Callback_BaseStationManualArmControl(void *buffer) { // define callback met de callback signature
    BasestationManualArmMovement *pckt = (BasestationManualArmMovement *)buffer;

    rtU.x += pckt->delta_x / (double)(1ULL << 31) * 0.01;
    rtU.y += pckt->delta_x / (double)(1ULL << 31) * 0.01;
    rtU.z += pckt->delta_x / (double)(1ULL << 31) * 0.01;
    rtU.gripperAng += pckt->delta_final_gripper_angle / (double)(1ULL << 31) * 0.005;
    rtU.gripperPitchActualPosition; // get from can
}
// PACKET_HANDLER_CONFIG_STATIC(Handler_ArmBoardControlSignals,
// PBEnvelope_arm_ctrl_tag, arm_ctrl, Callback_ArmBoardControlSignals);

/*Init and pass packet dispatcher*/
static uint8_t Handle_ArmBoardControlSignals_queue_buffer
    [PACKET_HANDLER_DEFAULT_QUEUE_LENGTH * sizeof(((PBEnvelope *)0)->payload.arm_ctrl)];
static uint8_t Handle_BaseStationManualArmControl_queue_buffer
    [PACKET_HANDLER_DEFAULT_QUEUE_LENGTH * sizeof(((PBEnvelope *)0)->payload.manual_arm)];
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
    }
};
