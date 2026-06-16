
#include "cubemars_ak.h"
#include "fdcan.h"
#include "logging.h"
#include "stm32h7xx_hal_fdcan.h"

void CAN_ConfigRx_AllStandard(void) {
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

void CAN_LogStatus(FDCAN_HandleTypeDef *hfdcan) {
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
