/**
 * @file tim.c
 * @brief Timer initialization for sensor board pump PWM control
 */

#include "stm32h7xx_hal.h"

/* Timer handle for pump PWM (TIM3) */
TIM_HandleTypeDef htim3;

/**
 * @brief Initialize TIM3 for pump PWM control
 * Configured for PWM mode, 1kHz frequency, TIM_CHANNEL_3
 */
void MX_TIM3_Init(void) {
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 119;           /* 480MHz / (119+1) = 4MHz */
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 3999;             /* 4MHz / 4000 = 1kHz */
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
    /* Initialization error */
  }
  
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
    /* Configuration error */
  }
  
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
    /* Configuration error */
  }
  
  /* Configure Channel 3 for PWM */
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
    /* Configuration error */
  }
  
  /* Start PWM generation */
  if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3) != HAL_OK) {
    /* Start error */
  }
}
