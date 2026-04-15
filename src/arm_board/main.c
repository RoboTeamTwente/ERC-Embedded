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
#include "pb_message.h"
#include "ethernet.h"
#include "gpio.h"
#include "logging.h"
#include "tim.h"
#include <stdint.h>
#include "stepper.h"
#include "components/arm_board/movement_control_in.pb.h"

#define TAG "ARM_BOARD"

extern COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;

void test_ethernet()
{

    LOGI(TAG, "Testing ethernet");
    HAL_Delay(1000);

    // init
    //  ETH_Init();
    //  ETH_udp_init();

    // uint8_t ip[4] = {0, 0, 0, 0};
    // uint8_t mac[6] = {255, 255, 255, 255, 255, 255};

    // ETH_udp_send(ip, 7, "udp message");
    // osDelay(100);
    // ETH_raw_send(mac, "ggg");
    // ETH_raw_send(mac, "long ass raw message looooong looooooonger looooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongest");
    // osDelay(100);

    //Populated protobuf
    ArmBoardControlSignals* sig;
    sig->control_base = 10.0f;
    sig->control_gripper_pitch = 20.0f;
    sig->control_gripper_rotation = 30.0f;
    sig->control_jaw = 40.0f;
    sig->stepper_bottom_ena = 1;
    sig->stepper_bottom_rev = 1;
    sig->stepper_top_ena = 1;
    sig->stepper_top_rev = 1;

    // sending packet
    uint8_t** encoded_data = NULL;
    size_t* encoded_length = 0;
    result_t res = pb_message_encode(sig, ArmBoardControlSignals_fields, &encoded_data, &encoded_length);

    if (res != RESULT_OK) {
        free(encoded_data);
        LOGE(TAG, "Encoding failed");
        return;
    }

    LOGI(TAG, "Encoding successfull");

    control_signals_t* structVar = {0};
    size_t struct_len = 0;
    res = pb_message_decode(encoded_data, encoded_length, ArmBoardControlSignals_fields, struct_len, (void **) &structVar);

    if (res != RESULT_OK) {
        free(encoded_data);
        LOGE(TAG, "Decoding failed");
        return;
    }
    LOGI(TAG, "Decoding successfull");

    LOGI(TAG, "Message says: %s %d %f", structVar->stepper_bottom_ENA);

    // ETH_udp_send(ip, 7, encoded_data);
    // free(encoded_data);
}

int main(void)
{

    HAL_Init();
    MX_GPIO_Init();
    SystemClock_Config();

    /* Initialize COM1 port */
    BspCOMInit.BaudRate = 115200;
    BspCOMInit.WordLength = COM_WORDLENGTH_8B;
    BspCOMInit.StopBits = COM_STOPBITS_1;
    BspCOMInit.Parity = COM_PARITY_NONE;
    BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
    if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
    {
        Error_Handler();
    }
    MX_USART3_Init(&huart_com, &BspCOMInit);
    LOG_init(&huart_com);

    // while(1) {
    //     LOGI(TAG, "hello");
    //     HAL_Delay(1000);
    // }

    // do_pwm();
    while (1)
    {
        test_ethernet();
        HAL_Delay(1000);
    }
}
