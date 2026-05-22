/* test code 
//USER CODE BEGIN Header */
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
#include <stdlib.h>
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

    /* Pulse counts to cycle through sequentially */
    const uint32_t scope_pulse_counts[] = {
    /* Very small positioning */
    10U,20U,50U,

    /* Typical motion */
    100U,200U,400U,800U,

    /* Larger movement */
    1600U,3200U,6400U
};
    const size_t scope_pulse_count_size = sizeof(scope_pulse_counts) / sizeof(scope_pulse_counts[0]);

    my_BSP_COM_Init();
    LOG_init(&huart_com);

    init_stepper(&step, 1, 50, &htim2);

    while (1) {
        for (size_t p = 0; p < scope_pulse_count_size; p++) {
            uint32_t pulse_count = scope_pulse_counts[p];

            LOGI(TAG, "--- New pulse count: %lu pulses ---", (unsigned long)pulse_count);

            for (int i = 0; i < 10; i++) {
                LOGI(TAG, "Burst %d/10 — %lu pulses", i + 1, (unsigned long)pulse_count);
                do_pwm_dma(&step, (int)pulse_count);
                while (step.pwm_dma_active) {
                    HAL_Delay(3000);
                }
            }

            LOGI(TAG, "Done with %lu pulses, switching...", (unsigned long)pulse_count);
            HAL_Delay(4000U); // added a break cause I wanted to see gaps between different pulse counts clearly on the scope. Can remove later if you want.
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

extern int receiving_counter;
int outgoing_counter = 0;

void test_stepper(void *argument) {
    stepper_t step;
    init_stepper(&step, 1, 50, &htim2);
    rotate_stepper(&step, 200);
}