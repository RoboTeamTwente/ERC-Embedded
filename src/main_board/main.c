#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "gpio.h"
#include "ili9341.h"
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
#define ILI_CS_PORT GPIOC
#define ILI_CS_PIN GPIO_PIN_8

#define ILI_RST_PORT GPIOC
#define ILI_RST_PIN GPIO_PIN_6

#define ILI_DC_PORT GPIOC
#define ILI_DC_PIN GPIO_PIN_5

// Helper Macros
#define ILI_CS_LO HAL_GPIO_WritePin(ILI_CS_PORT, ILI_CS_PIN, GPIO_PIN_RESET)
#define ILI_CS_HI HAL_GPIO_WritePin(ILI_CS_PORT, ILI_CS_PIN, GPIO_PIN_SET)
#define ILI_DC_CMD HAL_GPIO_WritePin(ILI_DC_PORT, ILI_DC_PIN, GPIO_PIN_RESET)
#define ILI_DC_DATA HAL_GPIO_WritePin(ILI_DC_PORT, ILI_DC_PIN, GPIO_PIN_SET)
#define ILI_RST_LO HAL_GPIO_WritePin(ILI_RST_PORT, ILI_RST_PIN, GPIO_PIN_RESET)
#define ILI_RST_HI HAL_GPIO_WritePin(ILI_RST_PORT, ILI_RST_PIN, GPIO_PIN_SET)

// Simple function to send a command
void ILI9341_SendCmd(uint8_t cmd) {
  ILI_DC_CMD;                             // Tell screen "Here comes a command"
  ILI_CS_LO;                              // Select screen
  HAL_SPI_Transmit(&hspi1, &cmd, 1, 100); // Send it (Assuming hspi1)
  ILI_CS_HI;                              // Deselect
}
void MainTask(void *argument) {
  ILI_CS_HI; // Deselect everything first
  ILI_RST_HI;
  osDelay(10);
  ILI_RST_LO;   // HOLD DOWN RESET
  osDelay(100); // Wait...
  ILI_RST_HI;   // RELEASE RESET
  osDelay(150); // Give it a moment to wake up.

  // --- STEP 2: The Software Wakeup Call ---
  ILI9341_SendCmd(0x01); // SOFTWARE RESET
  osDelay(100);

  ILI9341_SendCmd(0x11); // SLEEP OUT (Turn on internal oscillator)
  osDelay(150);

  ILI9341_SendCmd(0x29); // DISPLAY ON (Turn on the output)
  osDelay(100);

  // --- STEP 3: The "Invert" Test ---
  // If the screen flickers or changes color (often to black or random noise),
  // IT IS ALIVE. White screen means it's ignoring us.
  ILI9341_SendCmd(0x21);
  // Example usage of protobuf message
  while (1) {
    LOGI(TAG, "ILI9341 Initialized and Running\n");
  }
}
