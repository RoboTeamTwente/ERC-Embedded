// #include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "cubemx_main.h"
#include "fdcan.h"
#include "gpio.h"
#include "logging.h"
#include "result.h"
#include "stm32h7xx_hal_fdcan.h"
#include "stm32h7xx_nucleo.h"
#include "usart.h"
// #include "task.h"
#include "tim.h"

COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;
UART_HandleTypeDef huart4;

// Task attributes for CMSIS-RTOS v2
// const osThreadAttr_t mainTask_attributes = {
//     .name = "mainTask",
//     .stack_size = 1024 * 2,
//     .priority = (osPriority_t)tskIDLE_PRIORITY + 1U,
// };
//
const static char* TAG = "MAIN";

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

static void CAN_SendTestFrame(void) {
    HAL_StatusTypeDef status;

    LOGI(TAG, "Sending counter: %d", counter);
    status =
        HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, (uint8_t*)&counter);
    if (status != HAL_OK) {
        LOGE("CAN", "Message could not be sent, error: %d", status);
        Error_Handler();
    }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan,
                               uint32_t RxFifo0ITs) {
    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    if (hfdcan->Instance != FDCAN1) {
        return;
    }

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0U) {
        return;
    }

    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) !=
        HAL_OK) {
        LOGE("CAN", "Message could not be received");
        return;
    }
    uint32_t counter2 = ((uint32_t*)rx_data)[0];
    LOGI(TAG, "Received counter: %d", counter2);
    counter = counter2 + 1;
}
static const uint8_t ak_spin_1000_erpm[] = {0x02, 0x05, 0x08, 0x00, 0x00,
                                            0x03, 0xE8, 0x2B, 0x58, 0x03};
static const uint8_t ak_spin_100_erpm[] = {0x02, 0x05, 0x08, 0x00, 0x00,
                                           0x00, 0x64, 0x2E, 0x0F, 0x03};
static const uint8_t ak_stop[] = {0x02, 0x05, 0x08, 0x00, 0x00,
                                  0x00, 0x00, 0x65, 0xD2, 0x03};
static const uint8_t ak_spin_10000_erpm[] = {0x02, 0x05, 0x08, 0x00, 0x00,
                                             0x27, 0x10, 0x8F, 0x6D, 0x03};
static const uint8_t ak_spin_max_erpm[] = {0x02, 0x05, 0x08, 0x00, 0x01,
                                           0x86, 0xA0, 0x31, 0xC9, 0x03};
static const uint8_t ak_spin_neg_10000_erpm[] = {0x02, 0x05, 0x08, 0xFF, 0xFF,
                                                 0xD8, 0xF0, 0xF5, 0x7C, 0x03};
void MainTask(void* _arg) {
    LOGI(TAG, "In main function");

    for (;;) {
        // LOGI(TAG, "In loop");
        // BSP_LED_Toggle(LED_GREEN);
        // BSP_LED_Toggle(LED_RED);
        // CAN_SendTestFrame();

        HAL_StatusTypeDef status = HAL_UART_Transmit(
            &huart4, ak_spin_10000_erpm, sizeof(ak_spin_10000_erpm), 100);

        if (status != HAL_OK) {
            LOGE(TAG, "Could not send message about starting");
        }
        LOGI(TAG, "Motor start spinning...");
        HAL_Delay(1000);

        status = HAL_UART_Transmit(&huart4, ak_stop, sizeof(ak_stop), 100);

        if (status != HAL_OK) {
            LOGE(TAG, "Could not send message about stopping");
        }
        LOGI(TAG, "Motor stop spinning");
        HAL_Delay(1000);

        status = HAL_UART_Transmit(&huart4, ak_spin_neg_10000_erpm,
                                   sizeof(ak_spin_neg_10000_erpm), 100);

        if (status != HAL_OK) {
            LOGE(TAG, "Could not send message about stopping");
        }
        LOGI(TAG, "Motor stop spinning");
        HAL_Delay(1000);

        status = HAL_UART_Transmit(&huart4, ak_stop, sizeof(ak_stop), 100);

        if (status != HAL_OK) {
            LOGE(TAG, "Could not send message about stopping");
        }
        LOGI(TAG, "Motor stop spinning");
        HAL_Delay(1000);
    }
}

static void CAN_ConfigLoopbackRx(void) {
    FDCAN_FilterTypeDef filter = {0};

    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = 0x123;
    filter.FilterID2 = 0x7FF;

    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filter) != HAL_OK) {
        LOGE("CAN", "Filter config failed, err=0x%08lx",
             HAL_FDCAN_GetError(&hfdcan1));
        Error_Handler();
    }

    if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT,
                                     FDCAN_REJECT_REMOTE,
                                     FDCAN_REJECT_REMOTE) != HAL_OK) {
        LOGE("CAN", "Global filter config failed, err=0x%08lx",
             HAL_FDCAN_GetError(&hfdcan1));
        Error_Handler();
    }
}
int main() {
    MPU_Config();
    SCB_EnableICache();
    SCB_EnableDCache();
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_FDCAN1_Init();
    hfdcan1.Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;
    hfdcan1.Init.StdFiltersNbr = 1;

    // osKernelInitialize();
    // MX_FREERTOS_Init();

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
    MX_UART4_Init();
    LOG_init(&huart_com);

    // CAN_ConfigLoopbackRx();

    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                       0) != HAL_OK) {
        LOGE(TAG, "Interrupt for CAN new messages was not initialized");
        for (;;);
    }
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
        LOGE(TAG, "FDCAN start failed, err=0x%08lx",
             HAL_FDCAN_GetError(&hfdcan1));
        for (;;);
    }
    // osThreadNew(MainTask, NULL, &mainTask_attributes);

    MainTask(NULL);
    // osKernelStart();
}
