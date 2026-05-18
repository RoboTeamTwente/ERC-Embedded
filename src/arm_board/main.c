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
#include "dma.h"
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

// //networking
// #include "components/common/networking/inc/ethernet.h" //long path since LWIP also has ethernet.h
// #include "networking_constants.h"
// #include "ip_mac_constants.h"

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

// Task attributes for CMSIS-RTOS v2
osThreadId_t task_2Handle;
const osThreadAttr_t task2_attributes = {
    .name = "task2",
    .stack_size = 1024 * 8,
    .priority = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t test_stepper_attributes = {
    .name = "test_stepper",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityNormal,
};

const osThreadAttr_t pwm_test_attributes = {
    .name = "pwm_test",
    .stack_size = 1024,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes */
void test_stepper(void* argument);
void test_ethernet(void* argument);
void test_dma(void* argument);
static void test_pwm_scope(void);
static void configure_pwm_frequency(uint32_t frequency_hz);


// int main(void) {

//     /*Inits*/
//     MPU_Config_wrapper();

//     SCB_EnableICache();
//     SCB_EnableDCache();

//     HAL_Init();
//     SystemClock_Config();

//     MX_GPIO_Init();

//     //Init timers
//     MX_TIM2_Init();

//     //INit all configured peripherals
//     my_BSP_COM_Init(); 

//     //Log init
//     LOG_init(&huart_com);

//     // Init scheduler
//     osKernelInitialize();

//     /* Create the thread(s) */
//     // osThreadNew(test_ethernet, NULL, &task2_attributes);
//     osThreadNew(test_dma, NULL, &task2_attributes);

//     // Start scheduler
//     osKernelStart();
//     // We should never get here as control is now taken by the scheduler

//     while(1){}

// }

int ctr = 0;
uint32_t remaining;
void main(void* argument) {

    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM2_Init();

    test_pwm_scope();

    while(1) { }

}

static void test_pwm_scope(void) {
    stepper_t step;
    const uint32_t scope_frequencies_hz[] = {
            /* Low range (this is for slow movement or for High torque) */
            125U, 250U, 500U, 1000U, 2000U, 4000U, 
            
            /* Mid range (Standard operating speeds) */
            8000U, 16000U, 32000U, 
            
            /* High range (Fast motion / Requires acceleration ramps) */
            64000U, 128000U, 256000U
        };
    const size_t scope_frequency_count = sizeof(scope_frequencies_hz) / sizeof(scope_frequencies_hz[0]);

    my_BSP_COM_Init();
    LOG_init(&huart_com);

    init_stepper(&step, 1, 50, &htim2);

    while (1) {
        for (size_t i = 0; i < scope_frequency_count; i++) {
            uint32_t frequency_hz = scope_frequencies_hz[i];

            step.step_frequency_hz = frequency_hz;
            configure_pwm_frequency(frequency_hz);
            LOGI(TAG, "Frequency set to- %lu Hz", (unsigned long) frequency_hz);

            do_pwm_dma(&step, 200);
            HAL_Delay(500);
        }
    }
}

static void configure_pwm_frequency(uint32_t frequency_hz) {
    if (frequency_hz == 0U) {
        frequency_hz = 1U;
    }

    uint32_t timer_tick_hz = 1000000U;
    uint32_t period_ticks = timer_tick_hz / frequency_hz;

    if (period_ticks < 2U) {
        period_ticks = 2U;
    }

    __HAL_TIM_SET_AUTORELOAD(&htim2, period_ticks - 1U);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, period_ticks / 2U);
}

// /* Callback function that handles a specific packet*/
// void HandlePacket(receive_frame_t *receive_frame) {
//     printf("Wayoo, message received");
// }

extern int receiving_counter;
int outgoing_counter = 0;
// void test_ethernet(void* argument) {

//     /*Config + init sending side*/
//     uint8_t my_mac[6] = SAMPEL_BOARD_MAC;
//     uint8_t my_ip[4] = SAMPLE_BOARD_IP;
//     uint8_t netmask[4] = NETMASK;
//     uint8_t gateway[4] = GATEWAY;

//     ETH_init(NULL, my_ip, netmask, gateway, my_mac);

//     /*Making queues*/
//     int SendQueueSize = 80;

//     static StaticQueue_t xStaticQueue1;
//     uint8_t ucQueueStorageArea1[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
//     QueueHandle_t udp_receiver_queue1 = xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE, ucQueueStorageArea1, &xStaticQueue1);

//     static StaticQueue_t xStaticQueue2;
//     uint8_t ucQueueStorageArea2[SendQueueSize * ETHERNET_SQ_ITEM_SIZE];
//     QueueHandle_t udp_receiver_queue2 = xQueueCreateStatic(SendQueueSize, ETHERNET_SQ_ITEM_SIZE, ucQueueStorageArea2, &xStaticQueue2);
    
//     QueueHandle_t queues[2] = {udp_receiver_queue1, udp_receiver_queue2};

//     ETH_udp_init(2, queues, HandlePacket);

//     /*Config + add ARP receiving side*/
//     uint8_t ip[4] = NETWORK_IP;
//     uint8_t mac[6] = NETWORK_MAC;

//     ETH_add_arp(ip, mac, 5);

//     /*Sending a message*/
//     uint8_t packet1_payload[4] = {14,06,20,04};

//     /*Test sending*/
//     while (outgoing_counter < 100) { //NOTE: after 80 packages the queue will be full!
//           ETH_udp_send(ip, 8, packet1_payload, 4, 1);
//           osDelay(10);
//           outgoing_counter += 1;
//           LOGI(TAG, "%d", outgoing_counter);
//       }

//     while(1){
//     }

//     // ETH_udp_send(ip, 7, "udp message");
//     // osDelay(100);
//     // ETH_raw_send(mac, "ggg");
//     // ETH_raw_send(mac, "long ass raw message looooong looooooonger looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongest");
//     // osDelay(100);

//     //Populated protobuf
//     // ArmBoardControlSignals* sig;
//     // sig->control_base = 10.0f;
//     // sig->control_gripper_pitch = 20.0f;
//     // sig->control_gripper_rotation = 30.0f;
//     // sig->control_jaw = 40.0f;
//     // sig->stepper_bottom_ena = 1;
//     // sig->stepper_bottom_rev = 1;
//     // sig->stepper_top_ena = 1;
//     // sig->stepper_top_rev = 1;

//     // // sending packet
//     // uint8_t** encoded_data = NULL;
//     // size_t* encoded_length = 0;
//     // result_t res = pb_message_encode(sig, ArmBoardControlSignals_fields, &encoded_data, &encoded_length);

//     // if (res != RESULT_OK) {
//     //     free(encoded_data);
//     //     LOGE(TAG, "Encoding failed");
//     //     return;
//     // }

//     // LOGI(TAG, "Encoding successfull");

//     // control_signals_t* structVar = {0};
//     // size_t struct_len = 0;
//     // res = pb_message_decode(encoded_data, encoded_length, ArmBoardControlSignals_fields, struct_len, (void **) &structVar);

//     // if (res != RESULT_OK) {
//     //     free(encoded_data);
//     //     LOGE(TAG, "Decoding failed");
//     //     return;
//     // }
//     // LOGI(TAG, "Decoding successfull");

//     // LOGI(TAG, "Message says: %s %d %f", structVar->stepper_bottom_ENA);

//     // ETH_udp_send(ip, 7, encoded_data);
//     // free(encoded_data);
// }

void test_stepper(void *argument) {
    stepper_t step;
    // The hardware (GPIO, Timers) is initialized in main() before the scheduler starts.
    // It's not necessary to call MX_..._Init() functions here again.

    // The global 'htim2' is initialized by MX_TIM2_Init() in main().
    // We pass its address to the stepper initializer.
    init_stepper(&step, 1, 50, &htim2);
    rotate_stepper(&step, 200);
}