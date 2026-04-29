/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header wrapper for cubemx_main.h
  ******************************************************************************
  * @attention
  *
  * This file provides compatibility for LwIP and other components
  * that expect main.h to exist.
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


/* ---- START firmware_definitions ---- */

// #define LWIP_HOOK_UNKNOWN_ETH_PROTOCOL(pbuf, netif) eth_reader(netif, pbuf)
// #define LWIP_DEBUG 1
#define LWIP_HOOK_VLAN_SET(pcb, hdr, netif, src, dst, eth_hdr_len)             \
  get_vlan_header(pcb, hdr, netif, src, dst, eth_hdr_len)

/* ---- END firmware_definitions ---- */

#endif /* __MAIN_H */
