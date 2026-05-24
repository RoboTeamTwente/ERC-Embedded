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
#include "cubemx_main.h"
#include "gpio.h"
#include "tim.h"
#include <stdint.h>
#include "stepper.h"

//common libraries
#include "logging.h"
#include "result.h"

//protobuffers
#include "components/arm_board/movement_control_in.pb.h"
#include "pb_message.h"

//freertos
#include "cmsis_os.h"
#include "FreeRTOS.h"

//networking
#include "components/common/networking/inc/ethernet.h" //long path since LWIP also has ethernet.h
#include "networking_constants.h"
#include "ip_mac_constants.h"

//packetdispatcher
#include "packet_dispatcher.h"
#include "packet_dispatcher_macros.h"

#define TAG "ARM_BOARD"

/*External functions*/
extern COM_InitTypeDef BspCOMInit;
extern void MX_FREERTOS_Init(void);
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
extern void MX_DMA_Init(void);

/*Handles*/
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart_com;

/* Private function prototypes */
static result_t Callback_ArmBoardControlSignals(void *buffer);
static result_t Callback_ArmBoardMovementFeedback(void *buffer);

/*Ethernet constants*/
//Sending side
uint8_t my_mac[6] = SAMPEL_BOARD_MAC;
uint8_t my_ip[4] = SAMPLE_BOARD_IP;
uint8_t netmask[4] = NETMASK;
uint8_t gateway[4] = GATEWAY;

//Receiving side
uint8_t ip[4] = NETWORK_IP;
uint8_t mac[6] = NETWORK_MAC;

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

osThreadId_t stepperTaskHandle;
osThreadId_t testethernetTaskHandle;

// Task attributes for CMSIS-RTOS v2
osThreadId_t task_2Handle;
const osThreadAttr_t task2_attributes = {
    .name = "task2",
    .stack_size = 1024 * 6, //Make sure this is enough
    .priority = tskIDLE_PRIORITY,
};
static void test_ethernet(void* argument);

osThreadId_t pwmScopeTaskHandle;
const osThreadAttr_t pwm_scope_attributes = {
    .name       = "pwm_scope",
    .stack_size = 1024 * 8,
    .priority   = tskIDLE_PRIORITY,
};
static void pwm_scope_task(void *argument);

osThreadId_t stepper1_task_handle;
const osThreadAttr_t stepper1_task_attr = {
    .name       = "stepper1_task",
    .stack_size = 1024 * 8,
    .priority   = tskIDLE_PRIORITY,
};
static void stepper1_task(void *argument);

osThreadId_t stepper2_task_handle;
const osThreadAttr_t stepper2_task_attr = {
    .name       = "stepper2_task",
    .stack_size = 1024 * 8,
    .priority   = tskIDLE_PRIORITY,
};
static void stepper2_task(void *argument);

int main(void) {

    /*Inits*/
    MPU_Config_wrapper();
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();

    SCB_EnableICache();
    // SCB_EnableDCache();

    //Init timers
    MX_TIM2_Init();

    //INit all configured peripherals
    my_BSP_COM_Init(); 

    //Log init
    LOG_init(&huart_com);

    // Init scheduler
    osKernelInitialize();

    /* Create the thread(s) */

    testethernetTaskHandle = osThreadNew(test_ethernet, NULL, &task2_attributes);
    if (testethernetTaskHandle == NULL) {
        //HANDLE
    }

    pwmScopeTaskHandle = osThreadNew(pwm_scope_task,NULL,&pwm_scope_attributes);
    if (pwmScopeTaskHandle == NULL) {
        //HANDLE
    }

    stepper1_task_handle = osThreadNew(stepper1_task, NULL, &stepper1_task_attr);
    if (stepper1_task_handle == NULL) {
        //HANDLE
    }

    stepper2_task_handle = osThreadNew(stepper2_task, NULL, &stepper2_task_attr);
    if (stepper2_task_handle == NULL) {
        //HANDLE
    }

    // Start scheduler
    osKernelStart();
    // We should never get here as control is now taken by the scheduler

    while(1){}

}

static void stepper1_task(void *argument) {
}

static void stepper2_task(void *argument) {
}


static void pwm_scope_task(void *argument) {

    stepper_t step;
    init_stepper(&step, 1, 50, &htim2);

    static const uint32_t scope_pulse_counts[] = {
        10U, 20U, 50U,
        100U, 200U, 400U, 800U,
        1600U, 3200U, 6400U,
    };

    const size_t count =
        sizeof(scope_pulse_counts) /
        sizeof(scope_pulse_counts[0]);

    for (;;) {
        for (size_t p = 0; p < count; p++) {
            uint32_t pulse_count = scope_pulse_counts[p];

            LOGI(TAG,"--- New pulse count: %lu pulses ---",(unsigned long)pulse_count);

            for (int i = 0; i < 10; i++) {
                LOGI(TAG,"Burst %d/10 — %lu pulses",i + 1,(unsigned long)pulse_count);

                do_pwm_dma(&step, (int)pulse_count, (step.frequency_hz));
                osDelay(10);
            }
            LOGI(TAG,"Done with %lu pulses, switching...",(unsigned long)pulse_count);
            osDelay(4000U);
        }
    }
}


/* Callback function that handles a specific packet*/
void HandlePacket(receive_frame_t *receive_frame) {
    printf("Wayoo, message received");
}

extern int receiving_counter;
int outgoing_counter = 0;
void test_ethernet(void* argument) {

    //Setup using sending side params
    ETH_init(NULL, my_ip, netmask, gateway, my_mac);

    /*Making queues*/
    int SendQueueSize = 80;

    static StaticQueue_t xStaticQueue1;
    uint8_t ucQueueStorageArea1[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
    QueueHandle_t udp_receiver_queue1 = xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE, ucQueueStorageArea1, &xStaticQueue1);

    static StaticQueue_t xStaticQueue2;
    uint8_t ucQueueStorageArea2[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
    QueueHandle_t udp_receiver_queue2 = xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE, ucQueueStorageArea2, &xStaticQueue2);
    
    QueueHandle_t queues[2] = {udp_receiver_queue1, udp_receiver_queue2};

    /*Init and pass packet dispatcher*/

    //These are found in handler_stuff.h
    static packet_handler_config_t handlers[] = {
        Callback_ArmBoardControlSignals
    };

    PacketDispatcherInit(handlers, 1);
    ETH_udp_init(2, queues, DispatchPacket);

    /*Config + add ARP receiving side*/
    ETH_add_arp(ip, mac, 5);

    /*Sending a message*/
    uint8_t packet1_payload[4] = {14,06,20,04};

    /*Test sending*/
    while (outgoing_counter < 100) { //NOTE: after 80 packages the queue will be full!
          ETH_udp_send(ip, 8, packet1_payload, 4, 1);
          outgoing_counter += 1;
          LOGI(TAG, "%d", outgoing_counter);
          osDelay(1000);
      }

    while(1){
    }
}


/* HANDLER FUNCTIONS */

/* Config for 1 pbmessage: ArmBoardControlSignals */
static result_t Callback_ArmBoardControlSignals(void *buffer) {
    if (buffer == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    ArmBoardControlSignals* pckt = (ArmBoardControlSignals *)buffer;
    //base bldc
    pckt->control_base; 

    //gripper bldc
    pckt->control_gripper_pitch; 

    //gripper bldc
    pckt->control_gripper_rotation;

    //gripper jaw bldc
    pckt->control_jaw; 

    //bottom stepper
    pckt->stepper_bottom_rev;
    pckt->stepper_bottom_freq; //ignore

    //top stepper
    pckt->stepper_top_rev;
    pckt->stepper_top_freq; //ignore
    return RESULT_OK;
}

PACKET_HANDLER_CONFIG_STATIC(Handler_ArmBoardControlSignals, PBEnvelope_arm_ctrl_tag, arm_ctrl, Callback_ArmBoardControlSignals);
