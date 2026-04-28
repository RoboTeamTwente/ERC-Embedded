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

#define TAG "ARM_BOARD"

extern COM_InitTypeDef BspCOMInit;
extern void MX_FREERTOS_Init(void);
UART_HandleTypeDef huart_com;
osThreadId Task3Handle;

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
    .stack_size = 128,
    .priority = (osPriority_t)osPriorityNormal,
};

osThreadId_t task_3Handle;
const osThreadAttr_t task3_attributes = {
    .name = "task3",
    .stack_size = 128,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes */
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void Task2_init(void* argument);
void Task3_init(void* argument);
void test_ethernet(void* argument);


int main(void) {

    /* Inits */
    // Reset of all peripherals, Initializes the Flash interface and the Systick
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    //INit all configured peripherals
    my_BSP_COM_Init(); 

    //Log init
    LOG_init(&huart_com);

    // Init scheduler
    osKernelInitialize();

    /* Create the thread(s) */
    task_2Handle = osThreadNew(Task2_init, NULL, &task2_attributes);
    // task_3Handle = osThreadNew(Task3_init, NULL, &task3_attributes);

    // // Start scheduler
    osKernelStart();
    // We should never get here as control is now taken by the scheduler

}

void test_ethernet(void* argument) {

    while(1) {
        LOGI(TAG, "Testing ethernet");
        HAL_Delay(1000);

        //Enable D&I cache (for ETH)
        SCB_EnableICache();
        SCB_EnableDCache();

        //Memory protection unit
        MPU_Config_wrapper();

        uint8_t ip[4] = {192, 168, 0, 223};
        uint8_t mac[6] = {255, 255, 255, 255, 255, 255};
        uint8_t gateway[4] = {192, 168, 0, 1};
        uint8_t netmask[4] = {255, 255, 255, 0};

        // init
        ETH_init(NULL, ip, netmask, gateway, mac);
        // ETH_udp_init();

        // ETH_udp_send(ip, 7, "udp message");
        // osDelay(100);
        // ETH_raw_send(mac, "ggg");
        // ETH_raw_send(mac, "long ass raw message looooong looooooonger looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongest");
        // osDelay(100);

        //Populated protobuf
        // ArmBoardControlSignals* sig;
        // sig->control_base = 10.0f;
        // sig->control_gripper_pitch = 20.0f;
        // sig->control_gripper_rotation = 30.0f;
        // sig->control_jaw = 40.0f;
        // sig->stepper_bottom_ena = 1;
        // sig->stepper_bottom_rev = 1;
        // sig->stepper_top_ena = 1;
        // sig->stepper_top_rev = 1;

        // // sending packet
        // uint8_t** encoded_data = NULL;
        // size_t* encoded_length = 0;
        // result_t res = pb_message_encode(sig, ArmBoardControlSignals_fields, &encoded_data, &encoded_length);

        // if (res != RESULT_OK) {
        //     free(encoded_data);
        //     LOGE(TAG, "Encoding failed");
        //     return;
        // }

        // LOGI(TAG, "Encoding successfull");

        // control_signals_t* structVar = {0};
        // size_t struct_len = 0;
        // res = pb_message_decode(encoded_data, encoded_length, ArmBoardControlSignals_fields, struct_len, (void **) &structVar);

        // if (res != RESULT_OK) {
        //     free(encoded_data);
        //     LOGE(TAG, "Decoding failed");
        //     return;
        // }
        // LOGI(TAG, "Decoding successfull");

        // LOGI(TAG, "Message says: %s %d %f", structVar->stepper_bottom_ENA);

        // ETH_udp_send(ip, 7, encoded_data);
        // free(encoded_data);
    }
}

void Task2_init(void *argument) {
    init_stepper();
    rotate_stepper(20);
}

void Task3_init (void* argument) {
     while (1) {
        LOGI(TAG, "task3");
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
        HAL_Delay(200);
     }
}