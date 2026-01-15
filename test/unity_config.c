#include "unity_config.h"
#include "cubemx_main.h"
#include "logging.h"
#include "stm32h7xx_hal_uart.h"
#include "stm32h7xx_nucleo.h"
UART_HandleTypeDef huart_com;
extern COM_InitTypeDef BspCOMInit;

void unityOutputStart() {
  HAL_Init();
  HAL_Delay(2000);
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

void unityOutputChar(char c) {
  HAL_UART_Transmit(&huart_com, (uint8_t *)(&c), 1, 1000);
}

void unityOutputFlush() {}

void unityOutputComplete() {}
