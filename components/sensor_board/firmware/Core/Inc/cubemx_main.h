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
#define FORCE_ANALOG_DATA_2_Pin GPIO_PIN_3
#define FORCE_ANALOG_DATA_2_GPIO_Port GPIOF
#define FLOW_SENSOR_Pin GPIO_PIN_4
#define FLOW_SENSOR_GPIO_Port GPIOA
#define WEIGHT_INPUT_1_Pin GPIO_PIN_5
#define WEIGHT_INPUT_1_GPIO_Port GPIOA
#define WEIGHT_INPUT_2_Pin GPIO_PIN_6
#define WEIGHT_INPUT_2_GPIO_Port GPIOA
#define IMU_Sync_Pin GPIO_PIN_15
#define IMU_Sync_GPIO_Port GPIOB
#define PH_ANALOG_DATA_Pin GPIO_PIN_14
#define PH_ANALOG_DATA_GPIO_Port GPIOD
#define FORCE_ANALOG_DATA_1_Pin GPIO_PIN_15
#define FORCE_ANALOG_DATA_1_GPIO_Port GPIOD
#define IMU_Data_Ready_Pin GPIO_PIN_6
#define IMU_Data_Ready_GPIO_Port GPIOC
#define WEIGHT_CLOCK_1_Pin GPIO_PIN_7
#define WEIGHT_CLOCK_1_GPIO_Port GPIOC
#define WATER_PUMP_PWM_Pin GPIO_PIN_8
#define WATER_PUMP_PWM_GPIO_Port GPIOC
#define GPS_UART_TX_Pin GPIO_PIN_12
#define GPS_UART_TX_GPIO_Port GPIOC
#define GPS_UART_RX_Pin GPIO_PIN_2
#define GPS_UART_RX_GPIO_Port GPIOD
#define STEPPER_MOTOR_4_Pin GPIO_PIN_4
#define STEPPER_MOTOR_4_GPIO_Port GPIOD
#define STEPPER_MOTOR_3_Pin GPIO_PIN_5
#define STEPPER_MOTOR_3_GPIO_Port GPIOD
#define STEPPER_MOTOR_2_Pin GPIO_PIN_6
#define STEPPER_MOTOR_2_GPIO_Port GPIOD
#define STEPPER_MOTOR_1_Pin GPIO_PIN_7
#define STEPPER_MOTOR_1_GPIO_Port GPIOD
#define WEIGHT_CLOCK_2_Pin GPIO_PIN_5
#define WEIGHT_CLOCK_2_GPIO_Port GPIOB
#define IMU_I2C_Clock_Pin GPIO_PIN_8
#define IMU_I2C_Clock_GPIO_Port GPIOB
#define IMU_I2C_Data_Pin GPIO_PIN_9
#define IMU_I2C_Data_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif


/* ---- START firmware_definitions ---- */

// #define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(pbuf, netif) eth_reader(netif, pbuf)
// #define LWIP_DEBUG 1
#define LWIP_HOOK_VLAN_SET(pcb, hdr, netif, src, dst, eth_hdr_len)             \
  get_vlan_header(pcb, hdr, netif, src, dst, eth_hdr_len)

/* ---- END firmware_definitions ---- */

#endif /* __MAIN_H */
