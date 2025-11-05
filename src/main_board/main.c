#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "diagnostics.h"
#include "diagnostics/diagnostics.h"
#include "gpio.h"
#include "kv_pool.h"
#include "logging.h"
#include "main_board.pb.h"
#include "menu_driver.h"
#include "menu_driver_list.h"
#include "menu_driver_overview.h"
#include "pb_message.h"
#include "result.h"
#include "ssd1306.h"
#include "ssd1306/inc/ssd1306.h"
#include "ssd1306_fonts.h"
#include "stm32h7xx_hal_gpio.h"
#include "string.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static char *TAG = "MAIN";

// 16x16 Skull icon (0=black, 1=white)
static unsigned char icon_skull[32] = {
    0x0F, 0xF0, // 0000111111110000
    0x1F, 0xF8, // 0001111111111000
    0x3F, 0xFC, // 0011111111111100
    0x39, 0x9C, // 0011100110011100
    0x39, 0x9C, // 0011100110011100
    0x3F, 0xFC, // 0011111111111100
    0x1D, 0xB8, // 0001110110111000
    0x1D, 0xB8, // 0001110110111000
    0x1F, 0xF8, // 0001111111111000
    0x0F, 0xF0, // 0000111111110000
    0x06, 0x60, // 0000011001100000
    0x0C, 0x30, // 0000110000110000
    0x18, 0x18, // 0001100000011000
    0x18, 0x18, // 0001100000011000
    0x0C, 0x30, // 0000110000110000
    0x06, 0x60  // 0000011001100000
};
// 16x16 Heart icon (0=black, 1=white)
static unsigned char icon_heart[32] = {
    0x00, 0x00, // 0000000000000000
    0x00, 0x00, // 0000000000000000
    0x18, 0x18, // 0001100000011000
    0x3C, 0x3C, // 0011110000111100
    0x7E, 0x7E, // 0111111001111110
    0x7F, 0xFE, // 0111111111111110
    0x7F, 0xFE, // 0111111111111110
    0x7F, 0xFE, // 0111111111111110
    0x3F, 0xFC, // 0011111111111100
    0x1F, 0xF8, // 0001111111111000
    0x0F, 0xF0, // 0000111111110000
    0x07, 0xE0, // 0000011111100000
    0x03, 0xC0, // 0000001111000000
    0x01, 0x80, // 0000000110000000
    0x00, 0x00, // 0000000000000000
    0x00, 0x00  // 0000000000000000
};
// 16x16 'X' icon (0=black, 1=white)
static unsigned char icon_cross[32] = {
    0x00, 0x00, // 0000000000000000
    0x00, 0x00, // 0000000000000000
    0x30, 0x0C, // 0011000000001100
    0x18, 0x18, // 0001100000011000
    0x0C, 0x30, // 0000110000110000
    0x06, 0x60, // 0000011001100000
    0x03, 0xC0, // 0000001111000000
    0x01, 0x80, // 0000000110000000
    0x01, 0x80, // 0000000110000000
    0x03, 0xC0, // 0000001111000000
    0x06, 0x60, // 0000011001100000
    0x0C, 0x30, // 0000110000110000
    0x18, 0x18, // 0001100000011000
    0x30, 0x0C, // 0011000000001100
    0x00, 0x00, // 0000000000000000
    0x00, 0x00  // 0000000000000000
};
// 16x16 Checkmark icon (0=black, 1=white)
static unsigned char icon_check[32] = {
    0x00, 0x00, // 0000000000000000
    0x00, 0x00, // 0000000000000000
    0x00, 0x01, // 0000000000000001
    0x00, 0x03, // 0000000000000011
    0x00, 0x06, // 0000000000000110
    0x00, 0x0C, // 0000000000001100
    0x01, 0x18, // 0000000100011000
    0x03, 0x30, // 0000001100110000
    0x06, 0x60, // 0000011001100000
    0x0C, 0xC0, // 0000110011000000
    0x18, 0x80, // 0001100010000000
    0x30, 0x00, // 0011000000000000
    0x20, 0x00, // 0010000000000000
    0x00, 0x00, // 0000000000000000
    0x00, 0x00, // 0000000000000000
    0x00, 0x00  // 0000000000000000
};
void Error_Handler(void);
void cubemx_main(void);
void SystemClock_Config(void);
void MPU_Config(void);
void MX_GPIO_Init(void);

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
void MainTask(void *argument);

#define KV_POOL_BUFFER_SIZE 512
#define KV_POOL_MAX_KEYS 5

void delay_10ms(void) { osDelay(10); }
static unsigned char kv_pool_buffer[KV_POOL_BUFFER_SIZE];
static kv_pool main_kv_pool;

// Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 8,
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

static menu_page_t pages[5];
static menu_page_state states[5];

menu_input get_input_mock() { return btn_input(); }
/**
 * @brief  Main application task
 * @param  argument: Not used
 * @retval None
 */
void MainTask(void *argument) {
  LOGI(TAG, "In main task");
  result_t res = kv_pool_init(kv_pool_buffer, KV_POOL_BUFFER_SIZE,
                              KV_POOL_MAX_KEYS, &main_kv_pool, delay_10ms);
  if (res != RESULT_OK) {
    LOGE(TAG, "Failed to initialize KV pool: %d", res);
    Error_Handler();
  }
  LOGI(TAG, "KV pool initialized successfully");
  ssd1306_Init();
  LOGI(TAG, "Initializing components...");

  LOGI(TAG, "Components Initialized");
  menu_manager_t manager = {
      .active_page_id = 0,
      .pages = pages,
      .get_input = get_input_mock,
  };
  // uint8_t entry_ids[8] = {1, 2, 3, 4, 1, 2, 3, 4};
  // strcpy(manager.pages[1].name, "Item 1");
  // strcpy(manager.pages[2].name, "Item 2");
  // strcpy(manager.pages[3].name, "Item 3");
  // strcpy(manager.pages[4].name, "Item 4");
  // LOGI(TAG, "Preparing list page");
  // unsigned char entry_icons[8][MENU_LIST_ICON_INTEGER_SIZE];
  // memcpy(entry_icons[0], icon_skull, MENU_LIST_ICON_INTEGER_SIZE);
  // memcpy(entry_icons[1], icon_heart, MENU_LIST_ICON_INTEGER_SIZE);
  // memcpy(entry_icons[2], icon_cross, MENU_LIST_ICON_INTEGER_SIZE);
  // memcpy(entry_icons[3], icon_check, MENU_LIST_ICON_INTEGER_SIZE);
  // memcpy(entry_icons[4], icon_skull, MENU_LIST_ICON_INTEGER_SIZE);
  // memcpy(entry_icons[5], icon_heart, MENU_LIST_ICON_INTEGER_SIZE);
  // memcpy(entry_icons[6], icon_cross, MENU_LIST_ICON_INTEGER_SIZE);
  // memcpy(entry_icons[7], icon_check, MENU_LIST_ICON_INTEGER_SIZE);
  //
  // res = get_menu_page_list(0, 0, 8, entry_ids, entry_icons, "List",
  // &states[0],
  //                          &manager.pages[0]);
  kv_pool_insert(&main_kv_pool, 1, &(diagnostics_t){.last_code = 0x00},
                 sizeof(diagnostics_t));
  kv_pool_insert(&main_kv_pool, 2, &(diagnostics_t){.last_code = 0x01},
                 sizeof(diagnostics_t));
  kv_pool_insert(&main_kv_pool, 3, &(diagnostics_t){.last_code = 0x02},
                 sizeof(diagnostics_t));

  kv_pool_insert(&main_kv_pool, 4, &(diagnostics_t){.last_code = 0x03},
                 sizeof(diagnostics_t));

  kv_pool_insert(&main_kv_pool, 5, &(diagnostics_t){.last_code = 0x04},
                 sizeof(diagnostics_t));
  kv_pool_insert(&main_kv_pool, 6, &(diagnostics_t){.last_code = 0x05},
                 sizeof(diagnostics_t));
  res = get_menu_page_overview(
      0, 0, "Overview", 6,
      (char[MENU_OVERVIEW_MAX_ENTRIES][MENU_OVERVIEW_MAX_ENTRY_TITLE_LEN]){
          "Main", "Wirl", "Snsor", "Drive", "Arm", "SCont"},
      (uint8_t[MENU_OVERVIEW_MAX_ENTRIES]){1, 2, 3, 4, 5, 6}, &main_kv_pool,
      &states[0], &manager.pages[0]);
  LOGI(TAG, "List page prepared with result: %d", res);
  LOGI(TAG, "Entering main loop");
  while (1) {
    LOGI(TAG, "Updating");
    manager.pages[manager.active_page_id].update(&manager);
    LOGI(TAG, "Rendering");
    manager.pages[manager.active_page_id].render(&manager);
    LOGI(TAG, "Iterating");
    osDelay(500);
  }
}
