#include "cubemx_main.h"
#include "logging.h"
#include "stm32h7xx_hal_gpio.h"

static char *TAG = "MAIN";

void Error_Handler(void);
void cubemx_main(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;

/**
 * This is the system initialization function.
 * It initializes parts of the system that you hopefully only need to be done
 * once and remain unchanged. The **ONLY** reason to modify this funciton is if
 * additional initialization code is required. For example, if you need to
 * initialize additional peripherals or set up specific configurations. Things
 * like i2c, SPI and UART initializations should go here.
 * Mohammed Mosallam
  */
void system_initialization(void) {
  int i;
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* Initialize COM1 port */
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

  LOGI(TAG, "System initialized successfully.");
}

int main(void) {
  system_initialization();
  /** Code for initialization can go here**/

  while (1) {
    LOGI(TAG, "pin status: %i", HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) );
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
    HAL_Delay(500);
    /** Code that needs to be run multiple times can go here**/
  }
}
