#include "button_matrix_driver.h"
#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "gpio.h"
#include "ili9341.h"
#include "ili9341_fonts.h"
#include "logging.h"
#include "main_board.pb.h"
#include "pb_message.h"
#include "result.h"
#include "string.h"

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
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

void main() {
  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
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
  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osKernelStart();

  while (1) {
  }
}
SPI_HandleTypeDef hspi1;
void MainTask(void *argument) {
  ILI9341_Init();
  ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
  ILI9341_Fill_Screen(BLUE);
  ILI9341_Draw_Rectangle(50, 50, 100, 150, RED);
  ILI9341_WriteString(60, 120, "Hello, world!!", ILI9341_Font_11x18, WHITE,
                      BLUE);
  // Example usage of protobuf message
  while (1) {

    ButtonMatrixDriver_Scan();
    LOGI(TAG, "ILI9341 Initialized and Running\n");
    osDelay(1000);
  }
}
