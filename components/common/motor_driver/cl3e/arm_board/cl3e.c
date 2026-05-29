
#include "cl3e.h"

#include "erc-control-arm/control_arm_manual_ert_rtw/rtwtypes.h"
#include <stdint.h>

#include "cmsis_os2.h"
#include "math.h"
#include "result.h"
#include "stepper.h"
#include "stm32h7xx_hal.h" //needed in order to reach HAL timer handlers, dictionary for microcontroller that defines periperals

extern TIM_HandleTypeDef htim1; // from main
extern TIM_HandleTypeDef htim3; // from main

// DIR pin
// #define DIR_PORT GPIOA
// #define DIR_PIN  GPIO_PIN_3

#define MIN_FREQ 200
#define MAX_FREQ 15000
// #define STEP_FREQ 50
// #define STEP_DELAY_MS 2
#define MAX_BLDC_VOLTAGE 24.0f

void cl3e_set_target_position(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id,
                              int32_t target_position) {
  FDCAN_TxHeaderTypeDef tx_header = {0};

  tx_header.Identifier = 0x600 + node_id;
  tx_header.IdType = FDCAN_STANDARD_ID;
  tx_header.TxFrameType = FDCAN_DATA_FRAME;
  tx_header.DataLength = FDCAN_DLC_BYTES_8;

  uint8_t data[8];

  data[0] = 0x23; // expedited download, 4 bytes
  data[1] = 0x7A; // 607Ah low byte
  data[2] = 0x60; // 607Ah high byte
  data[3] = 0x00; // subindex

  data[4] = (uint8_t)(target_position);
  data[5] = (uint8_t)(target_position >> 8);
  data[6] = (uint8_t)(target_position >> 16);
  data[7] = (uint8_t)(target_position >> 24);

  HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &tx_header, data);
}

void cl3e_start_position_move(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id,
                              uint16_t controlword) {
  FDCAN_TxHeaderTypeDef tx_header = {0};

  tx_header.Identifier = 0x600 + node_id;
  tx_header.IdType = FDCAN_STANDARD_ID;
  tx_header.TxFrameType = FDCAN_DATA_FRAME;
  tx_header.DataLength = FDCAN_DLC_BYTES_8;

  uint8_t data[8];

  data[0] = 0x2B; // expedited download 2 bytes
  data[1] = 0x40; // 6040h
  data[2] = 0x60;
  data[3] = 0x00;

  data[4] = controlword & 0xFF;
  data[5] = (controlword >> 8) & 0xFF;
  data[6] = 0;
  data[7] = 0;

  HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &tx_header, data);
}

uint32_t CL3E_GetTimerClock(TIM_HandleTypeDef *htim) {
  uint32_t pclk;

  // APB2 timers
  if (htim->Instance == TIM1 || htim->Instance == TIM8 ||
      htim->Instance == TIM15 || htim->Instance == TIM16 ||
      htim->Instance == TIM17) {
    pclk = HAL_RCC_GetPCLK2Freq();
  }
  // APB1 timers
  else {
    pclk = HAL_RCC_GetPCLK1Freq();
  }

  return (pclk * 2) / (htim->Init.Prescaler + 1);
}

void CL3E_SetFrequency(TIM_HandleTypeDef *htim, uint32_t channel,
                       uint32_t freq) {
  uint32_t timer_clk = CL3E_GetTimerClock(htim);

  if (freq < MIN_FREQ)
    freq = MIN_FREQ;
  if (freq > MAX_FREQ)
    freq = MAX_FREQ;

  uint32_t arr = (timer_clk / freq) - 1;

  if (arr < 10)
    arr = 10;

  __HAL_TIM_SET_AUTORELOAD(htim, arr);
  __HAL_TIM_SET_COUNTER(htim, 0);

  __HAL_TIM_SET_COMPARE(htim, channel, arr / 2);
}

uint32_t CL3E_ControlToFreq(real_T u) {
  double norm = fabs(u) / MAX_BLDC_VOLTAGE;

  if (norm > 1.0)
    norm = 1.0;

  return (uint32_t)(MIN_FREQ + norm * (MAX_FREQ - MIN_FREQ));
}

void CL3E_DriveFromControl(TIM_HandleTypeDef *htim, uint32_t channel,
                           pin_t dir_pin, real_T u) {
  // direction
  if (u >= 0) {
    HAL_GPIO_WritePin(dir_pin.GPIOx, dir_pin.GPIO_PIN_no, GPIO_PIN_SET);
  } else {
    HAL_GPIO_WritePin(dir_pin.GPIOx, dir_pin.GPIO_PIN_no, GPIO_PIN_RESET);
  }

  // stop condition
  if (fabs(u) < 0.1) {
    __HAL_TIM_SET_COMPARE(htim, channel, 0);
    return;
  }

  // frequency obtained from scaling control signal
  uint32_t freq = CL3E_ControlToFreq(u);

  CL3E_SetFrequency(htim, channel, freq);
}

cl3e_information g_cl3e_info[CL3E_MAX_MOTORS] = {0};

void cl3e_request_position(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id) {
  FDCAN_TxHeaderTypeDef tx_header = {0};

  tx_header.Identifier = 0x600 + node_id;
  tx_header.IdType = FDCAN_STANDARD_ID;
  tx_header.TxFrameType = FDCAN_DATA_FRAME;
  tx_header.DataLength = FDCAN_DLC_BYTES_8;
  tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  tx_header.BitRateSwitch = FDCAN_BRS_OFF;
  tx_header.FDFormat = FDCAN_CLASSIC_CAN;
  tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  tx_header.MessageMarker = 0;

  uint8_t data[8] = {0x40,       // SDO upload request
                     0x64, 0x60, // index 0x6064
                     0x00,       // subindex
                     0,    0,    0, 0};

  HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &tx_header, data);
}

void cl3e_parse_can_message(const FDCAN_RxHeaderTypeDef *rx_header,
                            const uint8_t data[8]) {
  if (rx_header == NULL || data == NULL) {
    return;
  }

  uint16_t cob_id = rx_header->Identifier;

  // SDO response
  if ((cob_id & 0x780) == 0x580) {
    uint8_t node_id = cob_id - 0x580;

    // object 0x6064
    if (data[1] == 0x64 && data[2] == 0x60) {
      for (int i = 0; i < CL3E_MAX_MOTORS; i++) {
        if (g_cl3e_info[i].node_id == node_id || g_cl3e_info[i].node_id == 0) {
          g_cl3e_info[i].node_id = node_id;

          g_cl3e_info[i].actual_position =
              (int32_t)(((uint32_t)data[4]) | ((uint32_t)data[5] << 8) |
                        ((uint32_t)data[6] << 16) | ((uint32_t)data[7] << 24));

          break;
        }
      }
    }
  }
}

float cl3e_get_position_rev(uint8_t motor_index, int32_t encoder_cpr) {
  return (float)g_cl3e_info[motor_index].actual_position / (float)encoder_cpr;
}

float cl3e_get_position_deg(uint8_t motor_index, int32_t encoder_cpr) {
  return cl3e_get_position_rev(motor_index, encoder_cpr) * 360.0f;
}

float cl3e_get_position_rad(uint8_t motor_index, int32_t encoder_cpr) {
  return cl3e_get_position_rev(motor_index, encoder_cpr) * 2.0f * (float)M_PI;
}
