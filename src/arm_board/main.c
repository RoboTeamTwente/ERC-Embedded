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
#include "ethernet.h"
#include "gpio.h"
#include "logging.h"
#include "tim.h"
#include <stdint.h>
#include "stepper.h"

#define TAG "MAIN"

TIM_HandleTypeDef htim2;

int32_t ARR = 65535;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_TIM2_Init(void);

int main(void) {

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

	stepper_make_steps(300, -1);

	float DutyCycle = 0.25;
	int32_t CH1_DC = DutyCycle * ARR;

    while (1) {
    	TIM2->CCR1 = CH1_DC;
    	HAL_Delay(1);
    }
}