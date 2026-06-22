/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body - Sensor Board
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#ifndef PIO_UNIT_TESTING
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "gpio.h"
#include "logging.h"
#include "networking_constants.h"
#include "queue.h"
#include "result.h"
#include "string.h"
#include <stdint.h>
#include <stdlib.h>

#include "components/sensor_board/load_cell/load_cell_sensor.h"
#include "components/sensor_board/pressure/pressure_sensor.h"
#include "imu_sensor.h"
#include "ph_sensor.h"
#include "sensor_basics.h"

/* ---- New drivers -------------------------------------------------------- */
#include "components/sensor_board/sampling/flow_sensor/flow_sensor.h"
#include "components/sensor_board/sampling/pump/pump.h"

/* Nucleo BSP: VCP UART handle (hcom_uart[COM1] = USART3 on ST-Link) + COM1 */
#include "stm32h7xx_nucleo.h"

// Protobuf includes
#include "components/sensor_board/diagnostics.pb.h"
#include "components/sensor_board/imu_sensor.pb.h"
#include "components/sensor_board/load_cell.pb.h"
#include "components/sensor_board/ph_sensor.pb.h"
#include "components/sensor_board/pressure_sensor.pb.h"
#include "components/sensor_board/pump.pb.h"
#include "components/sensor_board/sensor.pb.h"

// Packet dispatcher includes
#include "components/common/packet_dispatcher/packet_dispatcher.h"
#include "components/common/packet_dispatcher/packet_dispatcher_macros.h"

// Networking includes
#include "components/common/envelope.pb.h"
#include "ethernet.h"
#include "ip_mac_constants.h"
#include "netif.h"
#include "pb_message.h"
#include "task.h"

#define TAG "MAIN"
#define MAIN_TASK_DELAY_MS 5000

#ifndef LOGD
#if (CONFIG_LOG_LEVEL <= LOG_INFO)
#define LOGD(TAG, format, ...) LOG(LOG_INFO, TAG, format, ##__VA_ARGS__)
#else
#define LOGD(TAG, format, ...) (void)0
#endif
#endif

extern void MX_FREERTOS_Init(void);
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
void Error_Handler(void);

/* LWIP / ETH handles owned by the networking component + firmware */
extern struct netif gnetif;
extern ETH_HandleTypeDef heth;

/**
 * @brief Physical Ethernet link state-change callback.
 *        Re-adds the static ARP entry for the destination when the link
 *        comes up so UDP sends resolve immediately.
 */
void ethernet_linkstatus_callback(void *arg) {
  struct netif *netif = (struct netif *)arg;
  uint8_t ip[4] = SAMPLE_BOARD_IP;
  uint8_t mac[6] = SAMPEL_BOARD_MAC;
  if (netif_is_up(netif)) {
    LOGI(TAG, "Physical ethernet link is up");
    ETH_add_arp(ip, mac, 5);
  } else {
    LOGE(TAG, "Physical ethernet link is down");
  }
}

/**
 * @brief Encode a PBEnvelope and send it as a UDP datagram, freeing the
 *        heap buffer allocated by pb_message_encode in all paths.
 *
 * @param[in] dest_ip Destination IPv4 address (4 bytes)
 * @param[in] env     Fully-populated envelope (which_payload + payload set)
 */
/* true = envelopes sent; false = encode/send skipped. */
static bool sendUDP = false;

static void udp_send_envelope(uint8_t dest_ip[4], PBEnvelope *env) {
  if (!sendUDP) {
    return;
  }

  uint8_t *encoded = NULL;
  size_t size = 0;
  result_t result = pb_message_encode(env, PBEnvelope_fields, &encoded, &size);
  if (result == RESULT_OK) {
    ETH_udp_send(dest_ip, PORT, encoded, (uint16_t)size, 1);
  } else {
    LOGE(TAG, "Envelope encode failed: %s (%s)", result_to_short_str(result),
         result_to_desc_str(result));
  }
  free(encoded);
}

/* ============================================================================
 * Packet Dispatcher Handler Functions
 * ============================================================================
 */

static result_t handle_sensor_ph_info(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  SensorBoardPHInfo *ph = (SensorBoardPHInfo *)buffer;
  LOGI(TAG, "Received pH info (value: %.2f, V: %.3f, state: %d)", ph->ph_value,
       ph->voltage, ph->state);
  return RESULT_OK;
}

static result_t handle_sensor_imu_info(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  SensorBoardIMUInfo *imu = (SensorBoardIMUInfo *)buffer;
  LOGI(TAG,
       "Received IMU info (accel: %.2f, %.2f, %.2f; gyro: %.2f, %.2f, %.2f)",
       imu->accel_x, imu->accel_y, imu->accel_z, imu->gyro_x, imu->gyro_y,
       imu->gyro_z);
  return RESULT_OK;
}

static result_t handle_sensor_load_cell_info(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  SensorBoardLoadCellInfo *load_cell = (SensorBoardLoadCellInfo *)buffer;
  LOGI(TAG,
       "Received load cell info (idx: %lu, force: %.2f N, mass: %.2f g, state: "
       "%d)",
       (unsigned long)load_cell->sensor_index, load_cell->force_newtons,
       load_cell->mass_grams, load_cell->state);
  return RESULT_OK;
}

static result_t handle_sensor_pressure_info(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  SensorBoardPressureInfo *pressure = (SensorBoardPressureInfo *)buffer;
  LOGI(TAG, "Received pressure info (idx: %lu, kPa: %.2f, state: %d)",
       (unsigned long)pressure->sensor_index, pressure->pressure_kpa,
       pressure->state);
  return RESULT_OK;
}

/**
 * @brief Handle incoming pump command over network.
 *        Caller sends a SensorBoardPumpInfo with the desired
 *        enabled/direction/speed_percent fields set.
 */
static result_t handle_sensor_pump_command(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  /* NOTE: g_pump_data must be accessible here.
   * Declared extern below; defined in MainTask scope as static. */
  extern pump_data_t g_pump_data;
  SensorBoardPumpInfo *cmd = (SensorBoardPumpInfo *)buffer;

  LOGI(TAG, "Pump cmd: enabled=%d dir=%d speed=%lu%%", cmd->enabled,
       cmd->direction, (unsigned long)cmd->speed_percent);

  pump_set_speed_percent(&g_pump_data, cmd->speed_percent);
  pump_set_direction(&g_pump_data, cmd->direction);
  pump_set_enabled(&g_pump_data, cmd->enabled);
  return RESULT_OK;
}

/* ============================================================================
 * Sensor Status Helper Functions
 * ============================================================================
 */

/**
 * @brief Map a SensorState enum to its proto name for logging.
 */
static inline const char *sensor_state_str(SensorState state) {
  switch (state) {
  case SensorState_SENSOR_IDLE:
    return "IDLE";
  case SensorState_SENSOR_OPERATING:
    return "OPERATING";
  case SensorState_SENSOR_ERROR:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

/**
 * @brief Map a SensorStatus enum to its proto name for logging.
 */
static inline const char *sensor_status_str(SensorStatus status) {
  switch (status) {
  case SensorStatus_STATUS_OK:
    return "OK";
  case SensorStatus_STATUS_DISCONNECTED:
    return "DISCONNECTED";
  case SensorStatus_STATUS_ERROR:
    return "ERROR";
  case SensorStatus_STATUS_INITIALIZING:
    return "INITIALIZING";
  default:
    return "UNKNOWN";
  }
}

/**
 * @brief Uniform per-sensor status log line.
 *        Format mirrors the proto model: name | STATUS | STATE | detail.
 *
 * @param[in] name    Sensor name
 * @param[in] status  SensorStatus (connection / data status)
 * @param[in] state   SensorState  (operating state)
 * @param[in] detail  Free-text detail (values, or result description)
 */
#define LOG_SENSOR_LINE(name, status, state, detail_fmt, ...)                  \
  LOGI(TAG, "%-10s | %-12s | %-9s | " detail_fmt, (name),                      \
       sensor_status_str(status), sensor_state_str(state), ##__VA_ARGS__)

/**
 * @brief Handle common sensor poll result, derive status/state, and log.
 *        - RESULT_ERR_UNIMPLEMENTED / RESULT_ERR_COMMS -> DISCONNECTED / IDLE
 *        - any other non-OK                            -> ERROR / ERROR
 *        Logs a uniform line using components/common/result strings.
 *        Error code handling is left to caller (different enum types).
 *
 * @param[out] state       Pointer to SensorState field to update
 * @param[out] status      Pointer to SensorStatus field to update (nullable)
 * @param[in]  sensor_name Name of sensor for logging
 * @param[in]  poll_result Result from poll_*_sensor()
 * @return true if poll succeeded (RESULT_OK), false otherwise
 */
static inline bool handle_sensor_poll_result(SensorState *state,
                                             SensorStatus *status,
                                             const char *sensor_name,
                                             result_t poll_result) {
  if (poll_result == RESULT_ERR_UNIMPLEMENTED ||
      poll_result == RESULT_ERR_COMMS) {
    *state = SensorState_SENSOR_IDLE;
    if (status != NULL) {
      *status = SensorStatus_STATUS_DISCONNECTED;
    }
    LOGW(TAG, "%-10s | %-12s | %-9s | not connected: %s (%s)", sensor_name,
         sensor_status_str(SensorStatus_STATUS_DISCONNECTED),
         sensor_state_str(SensorState_SENSOR_IDLE),
         result_to_short_str(poll_result), result_to_desc_str(poll_result));
    return false;
  } else if (poll_result != RESULT_OK) {
    *state = SensorState_SENSOR_ERROR;
    if (status != NULL) {
      *status = SensorStatus_STATUS_ERROR;
    }
    LOGE(TAG, "%-10s | %-12s | %-9s | poll error: %s (%s)", sensor_name,
         sensor_status_str(SensorStatus_STATUS_ERROR),
         sensor_state_str(SensorState_SENSOR_ERROR),
         result_to_short_str(poll_result), result_to_desc_str(poll_result));
    return false;
  }
  return true;
}

/**
 * @brief Sensor initialization wrapper functions using result_t.
 *        These wrap individual sensor inits and return result_t for consistent
 *        error handling with TRY_LOG macros.
 */

static result_t init_imu_wrapper(imu_data_t *imu) {
  imu_sensor_init(imu);
  return RESULT_OK;
}

static result_t init_ph_wrapper(ph_sensor_t *ph, float voltage) {
  return ph_sensor_init(ph, voltage);
}

static result_t init_load_cells_wrapper(load_cell_data_t *load_cells) {
  /* HX711 GPIO map (see firmware.ioc):
   *   Unit 0: DOUT = PA5 (WEIGHT_INPUT_1), SCK = PC7 (WEIGHT_CLOCK_1)
   *   Unit 1: DOUT = PA6 (WEIGHT_INPUT_2), SCK = PB5 (WEIGHT_CLOCK_2) */
  struct {
    GPIO_TypeDef *dout_port;
    uint16_t dout_pin;
    GPIO_TypeDef *sck_port;
    uint16_t sck_pin;
  } hx711_map[2] = {
      {GPIOA, GPIO_PIN_5, GPIOC, GPIO_PIN_7},
      {GPIOA, GPIO_PIN_6, GPIOB, GPIO_PIN_5},
  };

  for (size_t i = 0; i < 2; i++) {
    result_t result = load_cell_sensor_init_hw(
        &load_cells[i], hx711_map[i].dout_port, hx711_map[i].dout_pin,
        hx711_map[i].sck_port, hx711_map[i].sck_pin);
    if (result != RESULT_OK) {
      LOGE(TAG, "Load cell %lu init failed: %s (%s)", (unsigned long)i,
           result_to_short_str(result), result_to_desc_str(result));
      return result;
    }
  }
  return RESULT_OK;
}

static result_t init_pressure_sensors_wrapper(pressure_sensor_data_t *pressure) {
  for (size_t i = 0; i < 2; i++) {
    result_t result = pressure_sensor_init(&pressure[i]);
    if (result != RESULT_OK) {
      LOGE(TAG, "Pressure sensor %lu init failed: %s (%s)", (unsigned long)i,
           result_to_short_str(result), result_to_desc_str(result));
      return result;
    }
  }
  return RESULT_OK;
}

static result_t init_flow_sensor_wrapper(flow_sensor_data_t *flow) {
  return flow_sensor_init(flow);
}

static result_t init_pump_wrapper(pump_data_t *pump, pump_hw_t *hw) {
  return pump_init(pump, hw);
}

void MainTask(void *argument);

/* ============================================================================
 * Packet Handler Configurations
 * ============================================================================
 */

PACKET_HANDLER_CONFIG_STATIC(sensor_ph_handler, PBEnvelope_ph_info_tag, ph_info,
                             handle_sensor_ph_info);

PACKET_HANDLER_CONFIG_STATIC(sensor_imu_handler, PBEnvelope_imu_info_tag,
                             imu_info, handle_sensor_imu_info);

PACKET_HANDLER_CONFIG_STATIC(sensor_load_cell_handler,
                             PBEnvelope_load_cell_info_tag, load_cell_info,
                             handle_sensor_load_cell_info);

PACKET_HANDLER_CONFIG_STATIC(sensor_pressure_handler,
                             PBEnvelope_pressure_info_tag, pressure_info,
                             handle_sensor_pressure_info);

PACKET_HANDLER_CONFIG_STATIC(sensor_pump_handler, PBEnvelope_pump_info_tag,
                             pump_info, handle_sensor_pump_command);

/* ============================================================================
 * Global pump handle
 * ============================================================================
 */
pump_data_t g_pump_data;

/* Flow sensor handle — file scope so the EXTI ISR callback (below) can reach
 * it. Pulses are counted in HAL_GPIO_EXTI_Callback. */
flow_sensor_data_t g_flow_data;

/* Flow sensor signal pin — PA4 (FLOW_SENSOR). To generate pulse interrupts it
 * must be set to EXTI4 rising-edge with the EXTI4 NVIC line enabled in CubeMX;
 * until then this callback never fires and flow reads 0. */
#define FLOW_SENSOR_PIN GPIO_PIN_4

/* PWM timer for pump — enable TIM3 CH3 in CubeMX (see note below). */
extern TIM_HandleTypeDef htim3;

/**
 * @brief GPIO EXTI interrupt callback. Counts flow-sensor pulses.
 *        Weak HAL symbol — defining it here (application code) keeps it across
 *        CubeMX regenerations.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == FLOW_SENSOR_PIN) {
    flow_sensor_pulse_isr(&g_flow_data);
  }
}

/* mainTask_attributes + the osThreadNew(MainTask,...) call are generated by
 * CubeMX in freertos.c (task entry set to "As external"). Do not redefine here. */

void init_board() {
  MPU_Config_wrapper();
  SCB_EnableICache();
  SCB_EnableDCache();
  HAL_Init();
  SystemClock_Config();
  
  MX_GPIO_Init();

  /* NOTE: Do NOT create threads here - kernel not initialized yet! */
  /* NOTE: Do NOT call osKernelInitialize() here.
   * It's already called by cubemx_main.c after this function returns. */
}

// int main(void) { init_board(); }

/* ============================================================================
 * EXTI ISR glue — place in stm32h7xx_it.c (or here if you prefer)
 *
 * Example: flow sensor signal on PC6 (EXTI line 6)
 *   void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
 *       if (GPIO_Pin == FLOW_SENSOR_PIN) {
 *           flow_sensor_pulse_isr(&g_flow_data);
 *       }
 *   }
 *
 * g_flow_data is declared extern below; defined in MainTask as static.
 * ============================================================================
 */

/* ============================================================================
 * Main application task
 * ============================================================================
 */
void MainTask(void *argument) {
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  /* Initialize logging system */
  LOG_init(&hcom_uart[COM1]);
  LOGD(TAG, "Sensor board taking off...");

  /* ---- Existing sensor inits --------------------------------------------- */
  imu_data_t imu_data;
  LOGI(TAG, "Initializing IMU...");
  if (init_imu_wrapper(&imu_data) == RESULT_OK) {
    LOGI(TAG, "IMU init completed");
  }

  ph_sensor_t ph_sensor;
  LOGI(TAG, "Initializing pH Sensor...");
  if (init_ph_wrapper(&ph_sensor, 3.3f) != RESULT_OK) {
    LOGW(TAG, "pH sensor may not be available, continuing...");
  } else {
    LOGI(TAG, "pH sensor init completed");
  }

  load_cell_data_t load_cell_data[2];
  LOGI(TAG, "Initializing Load Cells...");
  if (init_load_cells_wrapper(load_cell_data) == RESULT_OK) {
    LOGI(TAG, "Load cells init completed");
  } else {
    LOGW(TAG, "Load cells init had errors, check log above");
  }

  pressure_sensor_data_t pressure_data[2];
  LOGI(TAG, "Initializing Pressure Sensors...");
  if (init_pressure_sensors_wrapper(pressure_data) == RESULT_OK) {
    LOGI(TAG, "Pressure sensors init completed");
  } else {
    LOGW(TAG, "Pressure sensors init had errors, check log above");
  }

  /* ---- Flow sensor init -------------------------------------------------- */
  /* g_flow_data is now file-scope (declared near the top) so the EXTI callback
   * HAL_GPIO_EXTI_Callback() can reach it. */
  LOGI(TAG, "Initializing Flow Sensor...");
  if (init_flow_sensor_wrapper(&g_flow_data) != RESULT_OK) {
    LOGW(TAG, "Flow sensor may not be available, continuing...");
  } else {
    LOGI(TAG, "Flow sensor initialized");
  }

  /* ---- Pump init --------------------------------------------------------- */
  /* Single low-side MOSFET: gate = TIM3_CH3 PWM (PC8). PWM duty = speed.
   * 2-wire DC pump, unidirectional — no direction/enable GPIOs needed. */
  pump_hw_t pump_hw = {
      .htim = &htim3,
      .tim_channel = TIM_CHANNEL_3,
      .tim_period = htim3.Init.Period,
  };

  LOGI(TAG, "Initializing Pump...");
  if (init_pump_wrapper(&g_pump_data, &pump_hw) != RESULT_OK) {
    LOGE(TAG, "Pump initialization failed");
  } else {
    LOGI(TAG, "Pump initialized");
    pump_set_direction(&g_pump_data, true);
    pump_set_speed_percent(&g_pump_data, 50U);
    pump_set_enabled(&g_pump_data, true);
  }

  /* ---- Ethernet init ----------------------------------------------------- */
  uint8_t self_ip[4] = NETWORK_IP;
  uint8_t self_mac[6] = NETWORK_MAC;
  uint8_t self_netmask[4] = NETMASK;
  uint8_t self_gateway[4] = GATEWAY;
  uint8_t dest_ip[4] = SAMPLE_BOARD_IP;
  uint8_t dest_mac[6] = SAMPEL_BOARD_MAC;

  LOGI(TAG, "Initializing Ethernet...");
  if (ETH_init(ethernet_linkstatus_callback, self_ip, self_netmask,
               self_gateway, self_mac) != RESULT_OK) {
    LOGE(TAG, "Ethernet init failed");
  } else {
    LOGI(TAG, "Ethernet init completed");
  }

  int mac1[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  int mac2[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
  int mac3[6] = {0x90, 0x2e, 0x16, 0xbe, 0x1b, 0x33};
  ETH_setup_MAC_address_filtering(mac1, mac2, mac3);

  /* Prioritised UDP transmit queues (statically allocated) */
#define SENSOR_SEND_QUEUE_SIZE 80
  static StaticQueue_t xStaticQueue1;
  static uint8_t
      ucQueueStorageArea1[SENSOR_SEND_QUEUE_SIZE * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t udp_send_queue1 =
      xQueueCreateStatic(SENSOR_SEND_QUEUE_SIZE, ETHERNET_SQ_ITEM_SIZE,
                         ucQueueStorageArea1, &xStaticQueue1);

  static StaticQueue_t xStaticQueue2;
  static uint8_t
      ucQueueStorageArea2[SENSOR_SEND_QUEUE_SIZE * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t udp_send_queue2 =
      xQueueCreateStatic(SENSOR_SEND_QUEUE_SIZE, ETHERNET_SQ_ITEM_SIZE,
                         ucQueueStorageArea2, &xStaticQueue2);
  QueueHandle_t send_queues[2] = {udp_send_queue1, udp_send_queue2};

  /* Inbound packet dispatcher (RX) */
  packet_handler_config_t handler_configs[] = {
      sensor_ph_handler, sensor_imu_handler, sensor_load_cell_handler,
      sensor_pressure_handler, sensor_pump_handler};
  PacketDispatcherInit(handler_configs,
                       sizeof(handler_configs) / sizeof(handler_configs[0]));

  ETH_udp_init(2, send_queues, DispatchPacket);
  ETH_add_arp(dest_ip, dest_mac, 5);

  BSP_LED_Toggle(LED_GREEN);

  /* ---- Main loop --------------------------------------------------------- */
  uint32_t loop_count = 0;
  LOGD(TAG, "========== ENTERING MAIN LOOP ==========");
  LOGD(TAG, "Starting main sensor loop...");
  const bool skip_sensor_polling = false; // need to make false to poll data

  while (1) {
    LOGD(TAG, "=== Main loop iteration %lu ===", loop_count);
    uint32_t free_heap = xPortGetFreeHeapSize();
    loop_count++;

    if (free_heap < 4096U) {  // Lower threshold - was 8192U
      LOGE(TAG, "CRITICAL: Low heap! Free: %lu bytes", free_heap);
      osDelay(10000);
      continue;
    }

    BSP_LED_Toggle(LED_GREEN);
    BSP_LED_Toggle(LED_BLUE);
    BSP_LED_Toggle(LED_RED);

    SensorBoardDiagnostics diagnostics = SensorBoardDiagnostics_init_zero;
    diagnostics.state = SensorBoardDiagnostics_State_OPERATING;

    if (skip_sensor_polling) {
      diagnostics.has_ph_sensor = false;
      diagnostics.has_imu_sensor = false;
    } else {

      /* ---------- pH sensor -------------------------------------------------
       */
      float ph_value, ph_voltage;
      result_t ph_poll_result = poll_ph_sensor(&ph_sensor);
      diagnostics.has_ph_sensor = true;

      if (!handle_sensor_poll_result(&diagnostics.ph_sensor.state, NULL,
                                      "pH", ph_poll_result)) {
        diagnostics.ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
        diagnostics.ph_sensor.ph_value = 0.0f;
        diagnostics.ph_sensor.voltage = 0.0f;
      } else if (ph_sensor_get_value(&ph_sensor, &ph_value) == RESULT_OK &&
                 ph_sensor_get_voltage(&ph_sensor, &ph_voltage) == RESULT_OK) {
        if (validate_ph_value(ph_value) == RESULT_OK) {
          diagnostics.ph_sensor.ph_value = ph_value;
          diagnostics.ph_sensor.voltage = ph_voltage;
          diagnostics.ph_sensor.state = SensorState_SENSOR_OPERATING;
          diagnostics.ph_sensor.error_code = PHErrorCode_PH_NO_ERROR;
          LOG_SENSOR_LINE("pH", SensorStatus_STATUS_OK,
                          SensorState_SENSOR_OPERATING, "pH=%.2f V=%.3f",
                          ph_value, ph_voltage);
        } else {
          diagnostics.ph_sensor.ph_value = ph_value;
          diagnostics.ph_sensor.voltage = ph_voltage;
          diagnostics.ph_sensor.state = SensorState_SENSOR_ERROR;
          diagnostics.ph_sensor.error_code = PHErrorCode_PH_INVALID_DATA;
          LOGW(TAG, "%-10s | %-12s | %-9s | invalid data: pH=%.2f out of range",
               "pH", sensor_status_str(SensorStatus_STATUS_ERROR),
               sensor_state_str(SensorState_SENSOR_ERROR), ph_value);
        }
      } else {
        diagnostics.ph_sensor.state = SensorState_SENSOR_ERROR;
        diagnostics.ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
        diagnostics.ph_sensor.ph_value = 0.0f;
        diagnostics.ph_sensor.voltage = 0.0f;
        LOGE(TAG, "%-10s | %-12s | %-9s | read failed", "pH",
             sensor_status_str(SensorStatus_STATUS_ERROR),
             sensor_state_str(SensorState_SENSOR_ERROR));
      }

      /* Transmit pH info over UDP */
      {
        PBEnvelope env = PBEnvelope_init_zero;
        env.which_payload = PBEnvelope_ph_info_tag;
        env.payload.ph_info = diagnostics.ph_sensor;
        udp_send_envelope(dest_ip, &env);
      }

      /* ---------- IMU -------------------------------------------------------
       */
      result_t imu_poll_result = poll_imu_sensor(&imu_data);
      diagnostics.has_imu_sensor = true;

      if (!handle_sensor_poll_result(&diagnostics.imu_sensor.state, NULL,
                                      "IMU", imu_poll_result)) {
        diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_COMMUNICATION_FAILURE;
        diagnostics.imu_sensor.accel_x = 0.0f;
        diagnostics.imu_sensor.accel_y = 0.0f;
        diagnostics.imu_sensor.accel_z = 0.0f;
        diagnostics.imu_sensor.gyro_x = 0.0f;
        diagnostics.imu_sensor.gyro_y = 0.0f;
        diagnostics.imu_sensor.gyro_z = 0.0f;
        diagnostics.imu_sensor.mag_x = 0.0f;
        diagnostics.imu_sensor.mag_y = 0.0f;
        diagnostics.imu_sensor.mag_z = 0.0f;
      } else {
        diagnostics.imu_sensor.accel_x = imu_data.accel[0];
        diagnostics.imu_sensor.accel_y = imu_data.accel[1];
        diagnostics.imu_sensor.accel_z = imu_data.accel[2];
        diagnostics.imu_sensor.gyro_x = imu_data.gyro[0];
        diagnostics.imu_sensor.gyro_y = imu_data.gyro[1];
        diagnostics.imu_sensor.gyro_z = imu_data.gyro[2];
        diagnostics.imu_sensor.mag_x = imu_data.mag[0];
        diagnostics.imu_sensor.mag_y = imu_data.mag[1];
        diagnostics.imu_sensor.mag_z = imu_data.mag[2];
        diagnostics.imu_sensor.state = SensorState_SENSOR_OPERATING;
        diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_NO_ERROR;
        LOG_SENSOR_LINE("IMU", SensorStatus_STATUS_OK,
                        SensorState_SENSOR_OPERATING,
                        "accel=%.2f,%.2f,%.2f gyro=%.2f,%.2f,%.2f",
                        imu_data.accel[0], imu_data.accel[1], imu_data.accel[2],
                        imu_data.gyro[0], imu_data.gyro[1], imu_data.gyro[2]);

        /* Validate IMU ranges */
        if (!imu_validate_accelerometer_range(&imu_data)) {
          LOGW(TAG, "IMU - Accelerometer out of range");
          diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_ACCELEROMETER_ERROR;
        } else if (!imu_validate_gyroscope_range(&imu_data)) {
          LOGW(TAG, "IMU - Gyroscope out of range");
          diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_GYROSCOPE_ERROR;
        } else if (!imu_validate_magnetometer_range(&imu_data)) {
          LOGW(TAG, "IMU - Magnetometer out of range");
          diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_MAGNETOMETER_ERROR;
        }
      }

      /* Transmit IMU info over UDP */
      {
        PBEnvelope env = PBEnvelope_init_zero;
        env.which_payload = PBEnvelope_imu_info_tag;
        env.payload.imu_info = diagnostics.imu_sensor;
        udp_send_envelope(dest_ip, &env);
      }

      /* ---------- Load cells + pressure (index loop) -----------------------
       */
      for (size_t i = 0; i < 2; i++) {
        /* Load cell */
        SensorBoardLoadCellInfo load_cell_info =
            SensorBoardLoadCellInfo_init_zero;
        result_t lc_result = poll_load_cell_sensor(&load_cell_data[i]);
        load_cell_info.sensor_index = (uint32_t)i;

        char lc_name[12];
        snprintf(lc_name, sizeof(lc_name), "LoadCell%lu", (unsigned long)i);
        if (!handle_sensor_poll_result(&load_cell_info.state, NULL,
                                        lc_name, lc_result)) {
          load_cell_info.error_code = LoadCellErrorCode_LOAD_CELL_COMMUNICATION_FAILURE;
          load_cell_info.force_newtons = 0.0f;
          load_cell_info.mass_grams = 0.0f;
        } else {
          float lc_force = 0.0f;
          float lc_mass = 0.0f;
          int32_t lc_counts = 0;
          float lc_scale = 0.0f;
          int32_t lc_tare = 0;
          bool lc_valid = false;

          if (load_cell_get_force_newtons(&load_cell_data[i], &lc_force) == RESULT_OK &&
              load_cell_get_mass_grams(&load_cell_data[i], &lc_mass) == RESULT_OK &&
              load_cell_get_raw_counts(&load_cell_data[i], &lc_counts) == RESULT_OK &&
              load_cell_get_calibration(&load_cell_data[i], &lc_scale, &lc_tare) == RESULT_OK) {
            load_cell_sensor_is_valid(&load_cell_data[i], &lc_valid);
            if (lc_valid) {
              load_cell_info.state = SensorState_SENSOR_OPERATING;
              load_cell_info.error_code = LoadCellErrorCode_LOAD_CELL_NO_ERROR;
              LOG_SENSOR_LINE(lc_name, SensorStatus_STATUS_OK,
                              SensorState_SENSOR_OPERATING,
                              "force=%.2fN mass=%.2fg", lc_force, lc_mass);
            } else {
              load_cell_info.state = SensorState_SENSOR_ERROR;
              load_cell_info.error_code = LoadCellErrorCode_LOAD_CELL_INVALID_DATA;
              LOGW(TAG, "%-10s | %-12s | %-9s | invalid data", lc_name,
                   sensor_status_str(SensorStatus_STATUS_ERROR),
                   sensor_state_str(SensorState_SENSOR_ERROR));
            }
            load_cell_info.force_newtons = lc_force;
            load_cell_info.mass_grams = lc_mass;
            load_cell_info.raw_counts = lc_counts;
            load_cell_info.scale_newtons_per_count = lc_scale;
            load_cell_info.tare_offset_counts = lc_tare;
          } else {
            load_cell_info.state = SensorState_SENSOR_ERROR;
            load_cell_info.error_code = LoadCellErrorCode_LOAD_CELL_COMMUNICATION_FAILURE;
            load_cell_info.force_newtons = 0.0f;
            load_cell_info.mass_grams = 0.0f;
            LOGE(TAG, "%-10s | %-12s | %-9s | read failed", lc_name,
                 sensor_status_str(SensorStatus_STATUS_ERROR),
                 sensor_state_str(SensorState_SENSOR_ERROR));
          }
        }
        load_cell_info.is_calibrated = load_cell_data[i].is_calibrated;

        /* Transmit load cell info over UDP */
        {
          PBEnvelope env = PBEnvelope_init_zero;
          env.which_payload = PBEnvelope_load_cell_info_tag;
          env.payload.load_cell_info = load_cell_info;
          udp_send_envelope(dest_ip, &env);
        }

        /* Pressure sensor */
        SensorBoardPressureInfo pressure_info =
            SensorBoardPressureInfo_init_zero;
        result_t pr_result = poll_pressure_sensor(&pressure_data[i]);
        pressure_info.sensor_index = (uint32_t)i;

        char pr_name[12];
        snprintf(pr_name, sizeof(pr_name), "Force%lu", (unsigned long)i);
        if (!handle_sensor_poll_result(&pressure_info.state, NULL,
                                        pr_name, pr_result)) {
          pressure_info.error_code = PressureErrorCode_PRESSURE_COMMUNICATION_FAILURE;
          pressure_info.pressure_kpa = 0.0f;
          pressure_info.temperature_c = 0.0f;
          pressure_info.voltage = 0.0f;
        } else {
          float pr_kpa = 0.0f;
          float pr_temp = 0.0f;
          float pr_voltage = 0.0f;
          bool pr_valid = false;

          if (pressure_sensor_get_pressure_kpa(&pressure_data[i], &pr_kpa) == RESULT_OK &&
              pressure_sensor_get_temperature_c(&pressure_data[i], &pr_temp) == RESULT_OK &&
              pressure_sensor_get_voltage(&pressure_data[i], &pr_voltage) == RESULT_OK) {
            pressure_sensor_is_valid(&pressure_data[i], &pr_valid);
            if (pr_valid) {
              pressure_info.state = SensorState_SENSOR_OPERATING;
              pressure_info.error_code = PressureErrorCode_PRESSURE_NO_ERROR;
              LOG_SENSOR_LINE(pr_name, SensorStatus_STATUS_OK,
                              SensorState_SENSOR_OPERATING,
                              "%.2fkPa %.2fC %.3fV", pr_kpa, pr_temp,
                              pr_voltage);
            } else {
              pressure_info.state = SensorState_SENSOR_ERROR;
              pressure_info.error_code = PressureErrorCode_PRESSURE_INVALID_DATA;
              LOGW(TAG, "%-10s | %-12s | %-9s | invalid data", pr_name,
                   sensor_status_str(SensorStatus_STATUS_ERROR),
                   sensor_state_str(SensorState_SENSOR_ERROR));
            }
            pressure_info.pressure_kpa = pr_kpa;
            pressure_info.temperature_c = pr_temp;
            pressure_info.voltage = pr_voltage;
          } else {
            pressure_info.state = SensorState_SENSOR_ERROR;
            pressure_info.error_code = PressureErrorCode_PRESSURE_COMMUNICATION_FAILURE;
            pressure_info.pressure_kpa = 0.0f;
            pressure_info.temperature_c = 0.0f;
            pressure_info.voltage = 0.0f;
            LOGE(TAG, "%-10s | %-12s | %-9s | read failed", pr_name,
                 sensor_status_str(SensorStatus_STATUS_ERROR),
                 sensor_state_str(SensorState_SENSOR_ERROR));
          }
        }
        pressure_info.is_calibrated = pressure_data[i].is_calibrated;

        /* Transmit pressure (FSR force) info over UDP */
        {
          PBEnvelope env = PBEnvelope_init_zero;
          env.which_payload = PBEnvelope_pressure_info_tag;
          env.payload.pressure_info = pressure_info;
          udp_send_envelope(dest_ip, &env);
        }
      }

    } /* end skip_sensor_polling */

    /* ==========================================================================
     * Flow Sensor — poll + build proto
     * ==========================================================================
     * poll_flow_sensor() computes flow rate from pulses counted by the ISR
     * since the last call.  Called every MAIN_TASK_DELAY_MS (5000 ms).
     */
    {
      SensorBoardFlowSensorInfo flow_info = SensorBoardFlowSensorInfo_init_zero;
      result_t flow_poll_result = poll_flow_sensor(&g_flow_data);

      if (!handle_sensor_poll_result(&flow_info.state, &flow_info.status,
                                      "Flow", flow_poll_result)) {
        flow_info.flow_rate_ml_min_x100 = 0U;
        flow_info.total_volume_ml = 0U;
        flow_info.pulse_count = 0U;
      } else {
        bool flow_valid = false;
        flow_sensor_is_valid(&g_flow_data, &flow_valid);

        if (!flow_valid) {
          /* First sampling window not yet complete */
          flow_info.state = SensorState_SENSOR_IDLE;
          flow_info.status = SensorStatus_STATUS_INITIALIZING;
          LOG_SENSOR_LINE("Flow", SensorStatus_STATUS_INITIALIZING,
                          SensorState_SENSOR_IDLE, "warming up");
        } else {
          uint32_t rate = 0U;
          uint32_t vol_ml = 0U;
          uint32_t pulses = 0U;
          flow_sensor_get_flow_rate(&g_flow_data, &rate);
          flow_sensor_get_total_volume_ml(&g_flow_data, &vol_ml);
          flow_sensor_get_pulse_count(&g_flow_data, &pulses);

          flow_info.flow_rate_ml_min_x100 = rate;
          flow_info.total_volume_ml = vol_ml;
          flow_info.pulse_count = pulses;

          /*
           * The flow sensor is a passive open-collector pulse source on a GPIO,
           * so electrical presence cannot be probed directly.  Zero lifetime
           * pulses => no signal ever received: treat as not connected / no flow
           * rather than reporting a bogus 0.00 ml/min "OK".
           */
          if (pulses == 0U) {
            flow_info.state = SensorState_SENSOR_IDLE;
            flow_info.status = SensorStatus_STATUS_DISCONNECTED;
            LOGW(TAG,
                 "%-10s | %-12s | %-9s | no pulses (not connected / no flow)",
                 "Flow", sensor_status_str(SensorStatus_STATUS_DISCONNECTED),
                 sensor_state_str(SensorState_SENSOR_IDLE));
          } else {
            flow_info.state = SensorState_SENSOR_OPERATING;
            flow_info.status = SensorStatus_STATUS_OK;
            LOG_SENSOR_LINE("Flow", SensorStatus_STATUS_OK,
                            SensorState_SENSOR_OPERATING,
                            "%lu.%02lu ml/min total=%lu ml pulses=%lu",
                            (unsigned long)(rate / 100U),
                            (unsigned long)(rate % 100U),
                            (unsigned long)vol_ml, (unsigned long)pulses);
          }
        }
      }

      /* Transmit flow info over UDP */
      PBEnvelope env = PBEnvelope_init_zero;
      env.which_payload = PBEnvelope_flow_info_tag;
      env.payload.flow_info = flow_info;
      udp_send_envelope(dest_ip, &env);
    }

    /* ==========================================================================
     * Pump — build proto from current state
     * ==========================================================================
     */
    {
      SensorBoardPumpInfo pump_info = SensorBoardPumpInfo_init_zero;

      /*
       * The pump is an OPEN-LOOP actuator: the L298N gives no current-sense or
       * fault line and none is wired to the MCU, so firmware cannot directly
       * tell whether a pump is plugged in. Do NOT report STATUS_OK just because
       * we commanded it. The only on-board proof that the pump is actually
       * moving fluid is the inline flow sensor, so cross-check against it:
       *   - not initialised                         -> ERROR
       *   - commanded off (disabled / 0 %)          -> IDLE  + OK  (healthy)
       *   - commanded on + flow detected            -> OPERATING + OK
       *   - commanded on + NO flow detected         -> OPERATING + DISCONNECTED
       *     (pump absent, dry, stalled, or flow sensor not installed)
       */
      if (!g_pump_data.is_initialised) {
        pump_info.state = SensorState_SENSOR_ERROR;
        pump_info.status = SensorStatus_STATUS_ERROR;
        LOGE(TAG, "%-10s | %-12s | %-9s | not initialised", "Pump",
             sensor_status_str(SensorStatus_STATUS_ERROR),
             sensor_state_str(SensorState_SENSOR_ERROR));
      } else {
        pump_get_enabled(&g_pump_data, &pump_info.enabled);
        pump_get_direction(&g_pump_data, &pump_info.direction);
        pump_get_speed_percent(&g_pump_data, &pump_info.speed_percent);
        pump_get_speed_rpm(&g_pump_data, &pump_info.speed_rpm);

        bool commanded_on = pump_info.enabled && pump_info.speed_percent > 0U;

        if (!commanded_on) {
          /* Intentionally off — driver is fine, just not running. */
          pump_info.state = SensorState_SENSOR_IDLE;
          pump_info.status = SensorStatus_STATUS_OK;
          LOG_SENSOR_LINE("Pump", SensorStatus_STATUS_OK,
                          SensorState_SENSOR_IDLE,
                          "off (enabled=%d speed=%lu%%)", pump_info.enabled,
                          (unsigned long)pump_info.speed_percent);
        } else {
          /* Commanded to run: confirm via the inline flow sensor. */
          uint32_t pump_flow_rate = 0U;
          flow_sensor_get_flow_rate(&g_flow_data, &pump_flow_rate);

          pump_info.state = SensorState_SENSOR_OPERATING;
          if (pump_flow_rate > 0U) {
            pump_info.status = SensorStatus_STATUS_OK;
            LOG_SENSOR_LINE("Pump", SensorStatus_STATUS_OK,
                            SensorState_SENSOR_OPERATING,
                            "dir=%d speed=%lu%% rpm~%lu flow=%lu.%02lu ml/min",
                            pump_info.direction,
                            (unsigned long)pump_info.speed_percent,
                            (unsigned long)pump_info.speed_rpm,
                            (unsigned long)(pump_flow_rate / 100U),
                            (unsigned long)(pump_flow_rate % 100U));
          } else {
            /* Driving the bridge but no flow seen — cannot confirm pump. */
            pump_info.status = SensorStatus_STATUS_DISCONNECTED;
            LOGW(TAG,
                 "%-10s | %-12s | %-9s | commanded on (dir=%d speed=%lu%%) but "
                 "no flow detected: pump may be absent/dry/stalled",
                 "Pump", sensor_status_str(SensorStatus_STATUS_DISCONNECTED),
                 sensor_state_str(SensorState_SENSOR_OPERATING),
                 pump_info.direction,
                 (unsigned long)pump_info.speed_percent);
          }
        }
      }

      /* Transmit pump info over UDP */
      PBEnvelope env = PBEnvelope_init_zero;
      env.which_payload = PBEnvelope_pump_info_tag;
      env.payload.pump_info = pump_info;
      udp_send_envelope(dest_ip, &env);
    }


    LOGD(TAG, "Loop %lu complete - Heap: %lu bytes, LED toggle done", loop_count,
         free_heap);

    osDelay(MAIN_TASK_DELAY_MS);
  }
}

#endif //! PIO_UNIT_TESTING
