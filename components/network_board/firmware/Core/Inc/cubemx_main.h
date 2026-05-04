<<<<<<< HEAD
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

#ifndef __MAIN_WRAPPER_H
#define __MAIN_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Include the actual main header */
#include "cubemx_main.h"

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_WRAPPER_H */
=======
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
>>>>>>> 4eb76620421fec19924143f17a231fa1d4bf79fb
