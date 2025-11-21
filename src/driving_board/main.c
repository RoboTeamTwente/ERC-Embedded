#ifndef PIO_UNIT_TESTING
#include "calculator.h"
#include "cubemx_main.h"
#include "gpio.h"
#include "logging.h"
#include "string.h"

static char *TAG = "MAIN";

COM_InitTypeDef BspCOMInit;

UART_HandleTypeDef huart_com;

void Error_Handler(void);
void SystemClock_Config(void);
void MPU_Config(void);
void MX_GPIO_Init(void);

void init_board() {
  HAL_Init();
  SystemClock_Config();

  BspCOMInit.BaudRate = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits = COM_STOPBITS_1;
  BspCOMInit.Parity = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
    Error_Handler();
  }
  MX_USART3_Init(&huart_com, &BspCOMInit);

  LOG_init(&huart_com);

  while (1) {
    LOGI(TAG, "works");
    HAL_Delay(500);
  }
}

int main(void) { init_board(); }
#endif //! PIO_UNIT_TESTING
