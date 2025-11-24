#include "cmsis_os2.h" // FreeRTOS wrapper header (v2)
#include "cubemx_main.h"
#include "gpio.h"
#include "logging.h"
#include "string.h"
#include "driving_board.pb.h"
#include "pb_message.h"
#include "control.h"
#include "rtwtypes.h"

#include "calculator.h"

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

void init_board() {
  HAL_Init();
  SystemClock_Config();
  
  osKernelInitialize();
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

  //control_initialize();

  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osKernelStart();


  while (1) {
    
  }
}

int main(void) { init_board(); }

/**
 * @brief  Main application task
 * @param  argument: Not used
 * @retval None
 */
void MainTask(void *argument) {

  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  BSP_LED_Toggle(LED_GREEN);
  
  DrivingBoardMotorMsg message = DrivingBoardMotorMsg_init_zero;
  message.distance_to_go = 2;
  message.turning_angle = 3;
  message.turning_radius = 3;
  pb_encoding_t encoding =
    pb_message_encode((void *)&message, DrivingBoardMotorMsg_fields);
  if (encoding.result != RESULT_OK) {
    LOGE(TAG, "Encoding error: %s", result_to_short_str(encoding.result));
  }
  else {
    LOGE(TAG, "message encoded successfully: %s", result_to_short_str(encoding.result));
  }

  while (1) {
   // control_step();
    BSP_LED_Toggle(LED_GREEN);
    BSP_LED_Toggle(LED_BLUE);
    BSP_LED_Toggle(LED_RED);
    LOGI(TAG, "%d + %d = %d", 5,2,add(5,2));

    LOGI(TAG, "This is the driving board");
    osDelay(1000);
  }
}
