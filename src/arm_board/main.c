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

// Controls code
#include "erc-control-arm/control_arm_ert_rtw/rtwtypes.h"
#include "erc-control-arm/control_arm_ert_rtw/control_arm.h"

#include "cmsis_os2.h"
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

extern ExtU rtU; // Get controls in  :)
extern ExtY rtY; // Get controls out :)
/*External functions*/
extern COM_InitTypeDef BspCOMInit;
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

// Stepper objects
stepper_t stepper1;
stepper_t stepper2;

static void vEthernetTask(void *argument);
static void vControlTask(void *arguments);
static void vArmController(void *argument);
static void vWristController(void *argument);
static void vStepperTask1(void *argument);
static void vStepperTask2(void *argument);

void setup_control_parameters() {
    rtU.x = 0.795;
    rtU.y = 0.0;
    rtU.z = 0.322;
    rtU.gripperAng = 0 * (M_PI / 180);
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

    LOGI(TAG, "---------------main---------------");

    // ETH_init(NULL, my_ip, netmask, gateway, my_mac);

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

    // Init scheduler
    osKernelInitialize();

    const osThreadAttr_t controlTaskAttr = {
        .name = "controlController",
        .stack_size = 1024 * 8,
        .priority = (osPriority_t)tskIDLE_PRIORITY + 5U,
    };
    const osThreadAttr_t armTaskAttr = {
        .name = "ArmController",
        .stack_size = 1024 * 8,
        .priority = (osPriority_t)tskIDLE_PRIORITY + 5U,
    };
    const osThreadAttr_t wristTaskAttr = {
        .name = "WristController",
        .stack_size = 1024 * 8,
        .priority = (osPriority_t)tskIDLE_PRIORITY + 4U,
    };
    const osThreadAttr_t stepper1TaskAttr = {
        .name = "Stepper1Controller",
        .stack_size = 1024 * 8,
        .priority = (osPriority_t)tskIDLE_PRIORITY + 3U,
    };
    const osThreadAttr_t stepper2TaskAttr = {
        .name = "Stepper2Controller",
        .stack_size = 1024 * 8,
        .priority = (osPriority_t)tskIDLE_PRIORITY + 2U,
    };

    osThreadNew(vControlTask, NULL, &controlTaskAttr);
    osThreadNew(vArmController, NULL, &armTaskAttr);
    osThreadNew(vWristController, NULL, &wristTaskAttr);
    osThreadNew(vStepperTask1, NULL, &stepper1TaskAttr);
    osThreadNew(vStepperTask2, NULL, &stepper2TaskAttr);
    /*
    xTaskCreate(vControlTask,   "controlController", 1024 * 8, NULL, tskIDLE_PRIORITY + 5U, NULL);
    xTaskCreate(vArmController,     "ArmController", 1024 * 8, NULL, tskIDLE_PRIORITY + 5U, NULL);
    xTaskCreate(vWristController, "WristController", 1024 * 8, NULL, tskIDLE_PRIORITY + 4U, NULL);
    xTaskCreate(vStepperTask1, "Stepper1Controller", 1024 * 8, NULL, tskIDLE_PRIORITY + 3U, NULL);
    xTaskCreate(vStepperTask2, "Stepper2Controller", 1024 * 8, NULL, tskIDLE_PRIORITY + 2U, NULL);
    */

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
            state = 500;
            //state = 10;
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
    LOGI(TAG, "rtU.x: %.3f", rtU.x);
    LOGI(TAG, "rtU.y: %.3f", rtU.y);
    LOGI(TAG, "rtU.z: %.3f", rtU.z);
    LOGI(TAG, "rtU.gripperAng: %.3f", rtU.gripperAng);
    control_arm_step();
    LOGI(TAG, "rtY.stepperRightSteps: %.1f", rtY.stepperRightSteps);
    LOGI(TAG, "rtY.stepperRightFrequency: %.1f", rtY.stepperRightFrequency);
    LOGI(TAG, "rtY.stepperLeftSteps: %.1f", rtY.stepperLeftSteps);
    LOGI(TAG, "rtY.stepperLeftFrequency: %.1f", rtY.stepperLeftFrequency);
}

uint32_t old_time;

bool startMovements = false;
const float start_gripper_angle = 87; //in degrees

bool stepper1ReachedPosition = false;
bool stepper2ReachedPosition = false;
bool wristReachedPosition = false;

static void vControlTask(void *argument){
    LOGI(TAG, "control task running");
    while (1) {
        rtU.deltaTime = 0.01;
        rtU.timePerMovement = 5;
        control_arm_step();
        osDelay(10);
    }
}

#define calibration 1          // 0 = off, 1 = right, 2 = left, 3 = both, 4 = wrist motor
#define calibrationDirection 1 // 0 = clockwise, 1 = counter clockwise
#define calibrationSpeed 100   // fequency when calibrating stepper motors
#define maxFrequency 250       // maximum frequency, to prevent the pullies from slipping

static void vArmController(void *argument) {
    //initializing stepper motors
    pin_t pin1 = {GPIOA, GPIO_PIN_4};
    pin_t pin2 = {GPIOC, GPIO_PIN_0};
    init_stepper(&stepper1, 50, &htim2, pin1, pin2);
    pin_t pin3 = {GPIOA, GPIO_PIN_5};
    pin_t pin4 = {GPIOB, GPIO_PIN_6};
    init_stepper(&stepper2, 50, &htim3, pin3, pin4);

    LOGI(TAG, "initialized stepper motors");

    //osDelay(5000);

    while(calibration != 0){
        int32_t steps;
        int32_t freq;
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
            float delta = 0.05;
            float maxPos = 270;
            if(calibrationDirection){
                delta *= -1;
            }
            for (float currentPos = 0; abs(currentPos) < maxPos; currentPos += delta) {
                cubemars_ak_set_position(&hfdcan1, 111, currentPos);
                LOGI(TAG, "gripper angle: %f", currentPos);
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
    LOGI(TAG, "setting gripper angle to %.2f", start_gripper_angle);
    for (float f = 0; f < start_gripper_angle; f += 0.5) {
        cubemars_ak_set_position(&hfdcan1, 111, f);
        osDelay(10);
    }
    for (int i = 0; i < 100; i++) {
        cubemars_ak_set_position(&hfdcan1, 111, start_gripper_angle);
        osDelay(10);
    }
    //stopping program here for testing
    //for(;;){};

    position_setter();
    startMovements = true;
    LOGI(TAG, "starting movements");

    while (1) {
        reachedPosition = (stepper1ReachedPosition && stepper2ReachedPosition);
        //only doing something if the position is not reached
        if(reachedPosition){
            LOGI(TAG, "new position");
            //position_setter();
            continue;
        }
        osDelay(100);



        /*
        if (rtY.stepperLeftSteps == 0 && rtY.stepperRightSteps == 0 && rtY.controlGripperPitch == 0 && rtY.controlBase == 0) {
          LOGI(TAG, "ERROR EVEYRTHING 0");
          osDelay(10);
          __builtin_trap();
          while (true) {};
        }
        */
    }
}

static void vStepperTask1(void *argument) {
    LOGI(TAG, "stepper1 controller started");
    //doing nothing before wrist position is initialized, and if calibrating the stepper motors
    while (!startMovements || calibration != 0) {
        osDelay(1000);
    }
    LOGI(TAG, "stepper1 controller running");
    while(1) {
        if(rtU.stepperRightActualPosition != rtY.stepperRightSteps){
            if(rtY.stepperRightFrequency > maxFrequency){
                rtY.stepperRightFrequency = maxFrequency;
                LOGE(TAG, "the frequency for stepper1 is to high, the timePerMovement should be higher");
            }
            LOGI(TAG, "Right steps: %.1f", rtY.stepperRightSteps);
            LOGI(TAG, "Right freq: %.1f", rtY.stepperRightFrequency);
            rotate_stepper(&stepper1, rtY.stepperRightSteps, rtY.stepperRightFrequency);

            //waiting for stepper to be done
            while (htim2.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
                LOGI(TAG, "waiting");
                osDelay(100); // Delay for thread switching
            }
            rtU.stepperRightActualPosition = rtY.stepperRightSteps;
            stepper1ReachedPosition = true;
            LOGI(TAG, "stepper1 reached position");
        }
        else {
            //LOGI(TAG, "else1");
            stepper1ReachedPosition = false;
        }
        osDelay(100);
    }
}

static void vStepperTask2(void *argument) {
    LOGI(TAG, "stepper2 controller started");
    //doing nothing before wrist position is initialized, and if calibrating the stepper motors
    while (!startMovements || calibration != 0) {
        osDelay(1000);
    }
    LOGI(TAG, "stepper2 controller running");
    while(1) {
        if(rtU.stepperLeftActualPosition != rtY.stepperLeftSteps){
            if(rtY.stepperLeftFrequency > maxFrequency){
                rtY.stepperLeftFrequency = maxFrequency;
                LOGE(TAG, "the frequency for stepper2 is to high, the timePerMovement should be higher");
            }
            LOGI(TAG, "Left steps: %.1f", rtY.stepperLeftSteps);
            LOGI(TAG, "Left freq: %.1f", rtY.stepperLeftFrequency);
            rotate_stepper(&stepper2, rtY.stepperLeftSteps, rtY.stepperLeftFrequency);

            //waiting for stepper to be done
            while (htim3.hdma[TIM_DMA_ID_CC1]->State != HAL_DMA_STATE_READY) {
                LOGI(TAG, "waiting");
                osDelay(100); // Delay for thread switching
            }
            LOGI(TAG, "stepper2 reached position");
            rtU.stepperLeftActualPosition = rtY.stepperLeftSteps;
            stepper2ReachedPosition = true;
        }
        else {
            //LOGI(TAG, "else2");
            stepper2ReachedPosition = false;
        }
        osDelay(100);
    }
}

static void vWristController(void *argument) {
    LOGI(TAG, "wrist controller started");
    //doing nothing before wrist position is initialized, and if calibrating the stepper motors
    while (!startMovements || calibration != 0) {
        osDelay(1000);
    }
    LOGI(TAG, "wrist controller running");
    while (1) {
        float setpoint = (rtY.controlGripperPitch)*(180/M_PI) + start_gripper_angle;
        cubemars_ak_set_position(&hfdcan1, 111, setpoint);
        //LOGI(TAG, "wrist pitch position: %f", setpoint);
        osDelay(100);
    }
}

/* Callback function that handles a specific packet*/
void HandlePacket(receive_frame_t *receive_frame) {
    LOGI(TAG, "Wayoo, message received");
}

/*
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
*/
