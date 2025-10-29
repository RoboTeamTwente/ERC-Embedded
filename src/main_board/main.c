#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "gpio.h"
#include "logging.h"
#include "main_board.pb.h"
#include "menu_driver.h"
#include "pb_message.h"
#include "result.h"
#include "ssd1306.h"
#include "ssd1306/inc/ssd1306.h"
#include "ssd1306_fonts.h"
#include "stm32h7xx_hal_gpio.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

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
    .stack_size = 1024 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

void init_board() {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();

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
  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osKernelStart();

  while (1) {
  }
}

int main(void) { init_board(); }

uint8_t btn_input() {
  uint8_t input = 0;
  SET_INPUT_DIRECTION_BOTTOM(input);
  return input;
}

/**
 * @brief  Main application task
 * @param  argument: Not used
 * @retval None
 */
void MainTask(void *argument) {
  LOGI(TAG, "In main task");
  ssd1306_Init();
  char *buffer = malloc(32);
  LOGI(TAG, "Initializing components...");
  int n = 10;
  char *temp_elements[10] = {
      "Element 0", "Element 1", "Element 2", "Element 3", "Element 4",
      "Element 5", "Element 6", "Element 7", "Element 8", "Element 9",
  };
  char **elements = malloc(10 * sizeof(char *));
  for (int i = 0; i < 10; i++) {
    char *elem = malloc(strlen(temp_elements[i]) + 1);
    strcpy(elem, temp_elements[i]);
    elements[i] = elem;
    LOGI(TAG, "Elements: %s", elements[i]);
  }

  menu_component list = get_simple_list_of_elements("The List!", elements, n);
  menu_component_manager manager = {.active_component = &list,
                                    .get_input = btn_input};
  LOGI(TAG, "Components Initialized");
  while (1) {
    tick_menu_component_manager(&manager);
    LOGI(TAG, "Iterating");
    osDelay(500);
  }
}
