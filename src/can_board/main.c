// #include "FreeRTOS.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "components/common/motor_driver/cubemars_ak/cubemars_ak.h"
#include "components/debugging_board/firmware/Drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal.h"
#include "cubemx_main.h"
#include "fdcan.h"
#include "gpio.h"
#include "logging.h"
#include "result.h"
#include "stdbool.h"
#include "stm32h7xx_hal_fdcan.h"
#include "stm32h7xx_nucleo.h"
#include "usart.h"
#include "components/common/networking/inc/ethernet.h"
// #include "task.h"
#include "tim.h"

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
UART_HandleTypeDef huart4;

// Task attributes for CMSIS-RTOS v2
const osThreadAttr_t mainTaskSender_attributes = {
    .name = "mainTaskSender",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
};

const osThreadAttr_t mainTaskListener_attributes = {
    .name = "mainTaskListener",
    .stack_size = 1024 * 2,
    .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
};


const static char *TAG = "MAIN";

FDCAN_TxHeaderTypeDef tx_header = {
    .Identifier = 0x123,
    .IdType = FDCAN_STANDARD_ID,
    .TxFrameType = FDCAN_DATA_FRAME,
    .DataLength = FDCAN_DLC_BYTES_4,
    .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
    .BitRateSwitch = FDCAN_BRS_OFF,
    .FDFormat = FDCAN_CLASSIC_CAN,
    .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
    .MessageMarker = 0,
};

static uint32_t counter = 0;

static cubemars_ak_information motor_info = {0};

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
  if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0) {
    return;
  }

  FDCAN_RxHeaderTypeDef rx_header = {0};
  uint8_t rx_data[8] = {0};

  if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) !=
      HAL_OK) {
    LOGE("CAN", "RX read failed, err=0x%08lx", HAL_FDCAN_GetError(hfdcan));
    return;
  }
  cubemars_ak_parse_can_feedback(&rx_header, rx_data, &motor_info);
}
static void CAN_LogStatus(FDCAN_HandleTypeDef *hfdcan) {
  FDCAN_ProtocolStatusTypeDef protocol_status;
  FDCAN_ErrorCountersTypeDef error_counters;

  if (HAL_FDCAN_GetProtocolStatus(hfdcan, &protocol_status) == HAL_OK) {
    LOGI("CAN",
         "LastErrorCode=%lu DataLastErrorCode=%lu Activity=%lu BusOff=%lu",
         protocol_status.LastErrorCode, protocol_status.DataLastErrorCode,
         protocol_status.Activity, protocol_status.BusOff);
  }

  if (HAL_FDCAN_GetErrorCounters(hfdcan, &error_counters) == HAL_OK) {
    LOGI("CAN", "TxErrorCnt=%lu RxErrorCnt=%lu RxErrorPassive=%lu",
         error_counters.TxErrorCnt, error_counters.RxErrorCnt,
         error_counters.RxErrorPassive);
  }

  LOGI("CAN", "HAL error=0x%08lx", HAL_FDCAN_GetError(hfdcan));
}

static void CAN_PrintRxState(void) {
  if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) >= 0) {
    LOGI("CAN", "RX FIFO0 fill=%lu HALerr=0x%08lx",
         HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0),
         HAL_FDCAN_GetError(&hfdcan1));
  } else {
    LOGI("CAN", "No messages in RX");
  }
}
void MainTaskListener() {
  LOGI(TAG, "Listener Task");
  for (;;) {
    LOGI(TAG, "Listening...");
    LOGI("CAN", "RX FIFO0 fill=%lu",
         HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0));
    HAL_Delay(5000);
  }
}

FDCAN_TxHeaderTypeDef can_test_header = {0};
uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};

static void CAN_SendTestFrame(void) {
  can_test_header.Identifier = 0x123;
  can_test_header.IdType = FDCAN_STANDARD_ID;
  can_test_header.TxFrameType = FDCAN_DATA_FRAME;
  can_test_header.DataLength = FDCAN_DLC_BYTES_8;
  can_test_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  can_test_header.BitRateSwitch = FDCAN_BRS_OFF;
  can_test_header.FDFormat = FDCAN_CLASSIC_CAN;
  can_test_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  can_test_header.MessageMarker = 0;

  if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &can_test_header, data) !=
      HAL_OK) {
    LOGE("CAN", "TX failed, err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
  } else {
    LOGI("CAN", "Queued test frame");
  }
}

void MainTaskSender() {
  LOGI(TAG, "Sender Task");
  for (;;) {
    LOGI(TAG, "Sending...");
    // CAN_SendTestrame();

    // cubemars_ak_set_speed(&hfdcan1, 111, 30000);
    // cubemars_ak_print_feedback(&motor_info);
    // HAL_Delay(1000);
    // cubemars_ak_set_speed(&hfdcan1, 111, 00);
    // HAL_Delay(500);
    // // cubemars_ak_set_speed(&hfdcan1, 93, 10000);
    // cubemars_ak_set_position(&hfdcan1,  93, 360);
    // cubemars_ak_print_feedback(&motor_info);
    // HAL_Delay(1000);
    // cubemars_ak_set_speed(&hfdcan1, 93, 00);
    // HAL_Delay(500);

    cubemars_ak_set_speed(&hfdcan1, 111, 10000);
    HAL_Delay(1000);

    cubemars_ak_set_speed(&hfdcan1, 111, 0);
    HAL_Delay(2000);

    // cubemars_ak_print_feedback(&motor_info);
    //
    // cubemars_ak_set_speed(&hfdcan1, 93, -5000);
    // HAL_Delay(500);
    // cubemars_ak_print_feedback(&motor_info);
    // cubemars_ak_set_speed(&hfdcan1, 93, 000);
    // cubemars_ak_print_feedback(&motor_info);
    // HAL_Delay(500);
    // cubemars_ak_print_feedback(&motor_info);
    // HAL_Delay(5000);
  }
}

void MainTask(void *_arg) {
  LOGI(TAG, "In main function");
  for (;;) {
  }
}
static void CAN_ConfigRx(void) {
  FDCAN_FilterTypeDef filter = {0};

  filter.IdType = FDCAN_EXTENDED_ID;
  filter.FilterIndex = 0;
  filter.FilterType = FDCAN_FILTER_MASK;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

  filter.FilterID1 = 0x00000000u;
  filter.FilterID2 = 0x00000000u;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filter) != HAL_OK) {
    LOGE("CAN", "Filter config failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_ConfigGlobalFilter(
          &hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
          FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
    LOGE("CAN", "Global filter config failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }
}
static void CAN_ConfigRx_AllStandard(void) {
  FDCAN_FilterTypeDef filter = {0};

  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = 0;
  filter.FilterType = FDCAN_FILTER_MASK;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

  // Accept everything: (ID & 0x000) == (0x000 & 0x000)
  filter.FilterID1 = 0x000;
  filter.FilterID2 = 0x000;

  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filter) != HAL_OK) {
    LOGE("CAN", "Filter config failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_ConfigGlobalFilter(
          &hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
          FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
    LOGE("CAN", "Global filter failed, err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }
}

void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan,
                                        uint32_t BufferIndexes) {
  LOGI("CAN", "TX complete buffers=0x%08lx\n", BufferIndexes);
}

void HAL_FDCAN_TxBufferAbortCallback(FDCAN_HandleTypeDef *hfdcan,
                                     uint32_t BufferIndexes) {
  LOGI("CAN", "TX abort buffers=0x%08lx\n", BufferIndexes);
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
  LOGE("CAN", "Error callback HALerr=0x%08lx\n", HAL_FDCAN_GetError(hfdcan));
}
int main() {
  MPU_Config_wrapper();
  SCB_EnableICache();
  SCB_EnableDCache();
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_FDCAN1_Init();
  MX_USART2_UART_Init();
  uint8_t ip[4] = {0,0,0,0};
  uint8_t netmask[4] = {0,0,0,0};
  uint8_t gateway[4] = {0,0,0,0};
  uint8_t mac[6] = {0,0,0,0,0,0};
  ETH_init(NULL, ip, netmask, gateway, mac);

  osKernelInitialize();


  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

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

  // uint8_t motor_id;
  // for (;;) {
  //     cubemars_ak_uart_get_motor_id(&huart2, &motor_id);
  //     LOGI(TAG, "Motor ID: %d", motor_id);
  //     HAL_Delay(500);
  // }
  CAN_ConfigRx_AllStandard();
  if (HAL_FDCAN_ConfigInterruptLines(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     FDCAN_INTERRUPT_LINE0) != HAL_OK) {
    LOGE("CAN", "Interrupt line config failed err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                     0) != HAL_OK) {
    LOGE("CAN", "Activate RX notification failed err=0x%08lx",
         HAL_FDCAN_GetError(&hfdcan1));
    Error_Handler();
  }
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
    LOGE(TAG, "FDCAN start failed, err=0x%08lx", HAL_FDCAN_GetError(&hfdcan1));
    for (;;)
      ;
  }
  LOGI("CAN", "Mode=%lu Presc=%lu TS1=%lu TS2=%lu SJW=%lu", hfdcan1.Init.Mode,
       hfdcan1.Init.NominalPrescaler, hfdcan1.Init.NominalTimeSeg1,
       hfdcan1.Init.NominalTimeSeg2, hfdcan1.Init.NominalSyncJumpWidth);
  osThreadNew(MainTaskSender, NULL, &mainTaskSender_attributes);
  osThreadNew(MainTaskListener, NULL, &mainTaskListener_attributes);

  //MainTaskListener();
  // MainTask(NULL);
  osKernelStart();
}
