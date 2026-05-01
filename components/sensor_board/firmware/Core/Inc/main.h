/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

#include "stm32h7xx_nucleo.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SYNC_in_IMU_Pin GPIO_PIN_13
#define SYNC_in_IMU_GPIO_Port GPIOF
#define RESET_Pin GPIO_PIN_14
#define RESET_GPIO_Port GPIOF
#define Input_weight_Pin GPIO_PIN_15
#define Input_weight_GPIO_Port GPIOF
#define SYNC_out_IMU_Pin GPIO_PIN_9
#define SYNC_out_IMU_GPIO_Port GPIOE
#define PPS_Pin GPIO_PIN_11
#define PPS_GPIO_Port GPIOE
#define PPS_EXTI_IRQn EXTI15_10_IRQn
#define DRDY_Pin GPIO_PIN_13
#define DRDY_GPIO_Port GPIOE
#define DRDY_EXTI_IRQn EXTI15_10_IRQn
#define Clock_weight_Pin GPIO_PIN_14
#define Clock_weight_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
