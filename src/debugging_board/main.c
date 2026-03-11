#ifndef UNIT_TEST
#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "gpio.h"
#include "ili9341.h"
#include "ili9341_fonts.h"
#include "logging.h"
#include "menu_driver.h"
#include "menu_driver_icons.h"
#include "menu_driver_imgs.h"
#include "menu_driver_list.h"
#include "pb_message.h"
#include "result.h"
#include "string.h"

#include "FreeRTOS.h"
#include "bucketed_pqueue.h"
#include "task.h"

static char *TAG = "MAIN";

void Error_Handler(void);
void cubemx_main(void);
void SystemClock_Config(void);
void MPU_Config(void);
void MX_GPIO_Init(void);

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
void MainTask(void *argument);

// Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)osPriorityBelowNormal,
};
SPI_HandleTypeDef hspi1;
extern s_menu_driver_task_handle;

void MainTask(void *argument) {
  osDelay(1000);
  LOGI(TAG, "Menu driver handle=%p", s_menu_driver_task_handle);
  while(1){osDelay(1000);}
}

void main() {
  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_Init();

  MX_GPIO_Init();
  SystemClock_Config();
  MX_SPI1_Init();

  HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET); // CS OFF

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

  osKernelInitialize();
  LOGI(TAG, "Kernel Initialized");
  osThreadNew(MainTask, NULL, &mainTask_attributes);
  menu_driver_task_spawn();
  osKernelStart();
  while (1) {
  }
}

#endif //! UNIT_TEST
//
