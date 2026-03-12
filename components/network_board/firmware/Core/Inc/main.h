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
