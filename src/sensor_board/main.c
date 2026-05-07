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
#include "ethernet.h"
#include "gpio.h"
#include "logging.h"
#include "lwip/netif.h"
#include "networking_constants.h"
#include "queue.h"
#include "string.h"
#include <stdint.h>
#include <stdlib.h>

#include "components/sensor_board/load_cell/load_cell_sensor.h"
#include "components/sensor_board/pressure/pressure_sensor.h"
#include "gps_sensor.h"
#include "imu_sensor.h"
#include "ph_sensor.h"
#include "sensor_basics.h"

/* ---- New drivers -------------------------------------------------------- */
#include "components/sensor_board/sampling/flow_sensor/flow_sensor.h"
#include "components/sensor_board/sampling/pump/pump.h"

// Protobuf includes
#include "components/sensor_board/diagnostics.pb.h"
#include "components/sensor_board/flow_sensor.pb.h"
#include "components/sensor_board/gps_sensor.pb.h"
#include "components/sensor_board/imu_sensor.pb.h"
#include "components/sensor_board/load_cell.pb.h"
#include "components/sensor_board/ph_sensor.pb.h"
#include "components/sensor_board/pressure_sensor.pb.h"
#include "components/sensor_board/pump.pb.h"
#include "components/sensor_board/sensor.pb.h"
#include "pb_message.h"

// Packet dispatcher includes
#include "components/common/envelope.pb.h"
#include "components/common/packet_dispatcher/packet_dispatcher.h"
#include "components/common/packet_dispatcher/packet_dispatcher_macros.h"
#include "stm/ethernet_udp.h"

#define TAG "MAIN"
#define MAIN_TASK_DELAY_MS 5000
#define SENSOR_UDP_DEST_PORT 7

static uint8_t ethernet_board_ip[4] = {192, 168, 0, 10};
static uint8_t ethernet_board_netmask[4] = {255, 255, 255, 0};
static uint8_t ethernet_board_gateway[4] = {192, 168, 0, 1};
static uint8_t ethernet_board_mac[6] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x00};
static uint8_t ethernet_peer_ip[4] = {192, 168, 0, 100};
static uint8_t ethernet_peer_mac[6] = {0x58, 0x11, 0x22, 0x3D, 0x88, 0xFC};

extern void MX_FREERTOS_Init(void);
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
extern struct netif gnetif;
void Error_Handler(void);

/* ============================================================================
 * Packet Dispatcher Handler Functions
 * ============================================================================
 */

static result_t handle_arm_control_signals(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  ArmBoardControlSignals *signals = (ArmBoardControlSignals *)buffer;
  LOGI(TAG, "Received ARM control signal (base: %.2f, jaw: %.2f, top_ena: %d)",
       signals->control_base, signals->control_jaw, signals->stepper_top_ena);
  return RESULT_OK;
}

static result_t handle_arm_diagnostics(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  ArmBoardDiagnostics *diag = (ArmBoardDiagnostics *)buffer;
  LOGI(TAG, "Received ARM diagnostics (state: %d)", diag->state);
  return RESULT_OK;
}

static result_t handle_drive_diagnostics(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  DrivingBoardDiagnostics *diag = (DrivingBoardDiagnostics *)buffer;
  LOGI(TAG, "Received Driving board diagnostics (state: %d)", diag->state);
  return RESULT_OK;
}

static result_t handle_sensor_ph_info(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  SensorBoardPHInfo *ph = (SensorBoardPHInfo *)buffer;
  LOGI(TAG, "Received pH info (value: %.2f, V: %.3f, state: %d)", ph->ph_value,
       ph->voltage, ph->state);
  return RESULT_OK;
}

static result_t handle_sensor_gps_info(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  SensorBoardGPSInfo *gps = (SensorBoardGPSInfo *)buffer;
  LOGI(TAG, "Received GPS info (lat: %.6f, lon: %.6f, alt: %.2f, sats: %ld)",
       gps->latitude, gps->longitude, gps->altitude, (long)gps->satellites);
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
 * UDP send helpers
 * ============================================================================
 */

static void sensor_udp_rx_callback(receive_frame_t *frame) {
  if (frame == NULL || frame->payload == NULL || frame->len == 0U) {
    return;
  }
  DispatchPacket(frame);
}

void ethernet_linkstatus_callback(void *arg) {
  struct netif *netif = (struct netif *)arg;
  if (netif == NULL) {
    return;
  }

  if (netif_is_link_up(netif)) {
    LOGI(TAG, "Physical ethernet link is up");
    result_t arp_result = ETH_add_arp(ethernet_peer_ip, ethernet_peer_mac, 5);
    if (arp_result != RESULT_OK) {
      LOGE(TAG, "Failed to add ARP entry after link-up: %s",
           result_to_short_str(arp_result));
    }
  } else {
    LOGE(TAG, "Physical ethernet link is down");
  }
}

void SensorBoardNetworkInit(void) {
  LOGI(TAG, "Starting ethernet setup...");

  result_t eth_result =
      ETH_init(ethernet_linkstatus_callback, ethernet_board_ip,
               ethernet_board_netmask, ethernet_board_gateway,
               ethernet_board_mac);
  if (eth_result != RESULT_OK) {
    LOGE(TAG, "ETH_init failed: %s", result_to_short_str(eth_result));
    return;
  }

  LOGI(TAG, "Ethernet init completed");
  LOGI(TAG, "Ethernet status after init: link=%s, up=%s",
       netif_is_link_up(&gnetif) ? "UP" : "DOWN",
       netif_is_up(&gnetif) ? "UP" : "DOWN");

  LOGI(TAG, "Setting MAC filtering...");
  int mac1[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  int mac2[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
  int mac3[6] = {0x13, 0x24, 0x35, 0x46, 0x57, 0x68};
  ETH_setup_MAC_address_filtering(mac1, mac2, mac3);
  LOGI(TAG, "MAC filtering completed");
}

static result_t
send_sensor_diagnostics_udp(uint8_t dest_ip[4], uint8_t port,
                            const SensorBoardDiagnostics *diagnostics,
                            uint8_t prio) {
  if (diagnostics == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  PBEnvelope envelope = PBEnvelope_init_zero;
  envelope.which_payload = PBEnvelope_sensor_diag_tag;
  envelope.payload.sensor_diag = *diagnostics;
  uint8_t *enc = NULL;
  size_t enc_sz = 0;
  result_t r = pb_message_encode(&envelope, PBEnvelope_fields, &enc, &enc_sz);
  if (r != RESULT_OK) {
    LOGE(TAG, "Encode diag failed: %s", result_to_short_str(r));
    return r;
  }
  ETH_udp_send(dest_ip, port, enc, enc_sz, prio);
  if (r != RESULT_OK) {
    LOGE(TAG, "Send diag failed: %s", result_to_short_str(r));
  }
  free(enc);
  return r;
}

static result_t send_load_cell_info_udp(uint8_t dest_ip[4], uint8_t port,
                                        const SensorBoardLoadCellInfo *info,
                                        uint8_t prio) {
  if (info == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  PBEnvelope envelope = PBEnvelope_init_zero;
  envelope.which_payload = PBEnvelope_load_cell_info_tag;
  envelope.payload.load_cell_info = *info;
  uint8_t *enc = NULL;
  size_t enc_sz = 0;
  result_t r = pb_message_encode(&envelope, PBEnvelope_fields, &enc, &enc_sz);
  if (r != RESULT_OK) {
    return r;
  }
  ETH_udp_send(dest_ip, port, enc, enc_sz, prio);
  free(enc);
  return r;
}

static result_t send_pressure_info_udp(uint8_t dest_ip[4], uint8_t port,
                                       const SensorBoardPressureInfo *info,
                                       uint8_t prio) {
  if (info == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  PBEnvelope envelope = PBEnvelope_init_zero;
  envelope.which_payload = PBEnvelope_pressure_info_tag;
  envelope.payload.pressure_info = *info;
  uint8_t *enc = NULL;
  size_t enc_sz = 0;
  result_t r = pb_message_encode(&envelope, PBEnvelope_fields, &enc, &enc_sz);
  if (r != RESULT_OK) {
    return r;
  }
  ETH_udp_send(dest_ip, port, enc, enc_sz, prio);
  free(enc);
  return r;
}

/**
 * @brief Send SensorBoardFlowSensorInfo wrapped in PBEnvelope via UDP
 */
static result_t send_flow_sensor_info_udp(uint8_t dest_ip[4], uint8_t port,
                                          const SensorBoardFlowSensorInfo *info,
                                          uint8_t prio) {
  if (info == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  PBEnvelope envelope = PBEnvelope_init_zero;
  envelope.which_payload = PBEnvelope_flow_sensor_info_tag;
  envelope.payload.flow_sensor_info = *info;
  uint8_t *enc = NULL;
  size_t enc_sz = 0;
  result_t r = pb_message_encode(&envelope, PBEnvelope_fields, &enc, &enc_sz);
  if (r != RESULT_OK) {
    LOGE(TAG, "Encode flow sensor failed: %s", result_to_short_str(r));
    return r;
  }
  ETH_udp_send(dest_ip, port, enc, enc_sz, prio);
  free(enc);
  return r;
}

/**
 * @brief Send SensorBoardPumpInfo wrapped in PBEnvelope via UDP
 */
static result_t send_pump_info_udp(uint8_t dest_ip[4], uint8_t port,
                                   const SensorBoardPumpInfo *info,
                                   uint8_t prio) {
  if (info == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  PBEnvelope envelope = PBEnvelope_init_zero;
  envelope.which_payload = PBEnvelope_pump_info_tag;
  envelope.payload.pump_info = *info;
  uint8_t *enc = NULL;
  size_t enc_sz = 0;
  result_t r = pb_message_encode(&envelope, PBEnvelope_fields, &enc, &enc_sz);
  if (r != RESULT_OK) {
    LOGE(TAG, "Encode pump info failed: %s", result_to_short_str(r));
    return r;
  }
  ETH_udp_send(dest_ip, port, enc, enc_sz, prio);
  free(enc);
  return r;
}

void MainTask(void *argument);

/* ============================================================================
 * Packet Handler Configurations
 * ============================================================================
 */

PACKET_HANDLER_CONFIG_STATIC(arm_control_handler, PBEnvelope_arm_ctrl_tag,
                             arm_ctrl, handle_arm_control_signals);

PACKET_HANDLER_CONFIG_STATIC(arm_diag_handler, PBEnvelope_arm_diag_tag,
                             arm_diag, handle_arm_diagnostics);

PACKET_HANDLER_CONFIG_STATIC(drive_diag_handler, PBEnvelope_drive_diag_tag,
                             drive_diag, handle_drive_diagnostics);

PACKET_HANDLER_CONFIG_STATIC(sensor_ph_handler, PBEnvelope_ph_info_tag, ph_info,
                             handle_sensor_ph_info);

PACKET_HANDLER_CONFIG_STATIC(sensor_gps_handler, PBEnvelope_gps_info_tag,
                             gps_info, handle_sensor_gps_info);

PACKET_HANDLER_CONFIG_STATIC(sensor_imu_handler, PBEnvelope_imu_info_tag,
                             imu_info, handle_sensor_imu_info);

PACKET_HANDLER_CONFIG_STATIC(sensor_load_cell_handler,
                             PBEnvelope_load_cell_info_tag, load_cell_info,
                             handle_sensor_load_cell_info);

PACKET_HANDLER_CONFIG_STATIC(sensor_pressure_handler,
                             PBEnvelope_pressure_info_tag, pressure_info,
                             handle_sensor_pressure_info);

/* Receive pump commands from the network (e.g. from arm board / base station)
 */
PACKET_HANDLER_CONFIG_STATIC(sensor_pump_cmd_handler, PBEnvelope_pump_info_tag,
                             pump_info, handle_sensor_pump_command);

/* ============================================================================
 * Global pump handle — accessible from the network command handler above
 * ============================================================================
 */
pump_data_t g_pump_data;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef
    htim3; /* PWM timer for pump — adjust to your CubeMX config */
COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;

const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 8,
    .priority = (osPriority_t)osPriorityAboveNormal,
};

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

  LOGI(TAG, "Sensor board taking off...");

  /* ---- Packet dispatcher ------------------------------------------------- */
  LOGI(TAG, "Initializing packet dispatcher...");
  
  packet_handler_config_t handler_configs[] = {
      arm_control_handler,      arm_diag_handler,
      drive_diag_handler,       sensor_ph_handler,
      sensor_gps_handler,       sensor_imu_handler,
      sensor_load_cell_handler, sensor_pressure_handler,
      sensor_pump_cmd_handler,
  };

  result_t dispatcher_result = PacketDispatcherInit(
      handler_configs, sizeof(handler_configs) / sizeof(handler_configs[0]));
  if (dispatcher_result != RESULT_OK) {
    LOGE(TAG, "Packet dispatcher init failed: %s",
         result_to_short_str(dispatcher_result));
  } else {
    LOGI(TAG, "Packet dispatcher initialized successfully");
  }

  /* ---- Existing sensor inits --------------------------------------------- */
  imu_data_t imu_data;
  LOGI(TAG, "Initializing IMU...");
  imu_sensor_init(&imu_data);
  LOGI(TAG, "IMU init completed");

  ph_sensor_t ph_sensor;
  LOGI(TAG, "Initializing pH Sensor...");
  ph_sensor_init(&ph_sensor, 3.3f);
  LOGI(TAG, "pH sensor init completed");

  gps_data_t gps_data;
  LOGI(TAG, "Initializing GPS...");
  gps_sensor_init(&gps_data);
  LOGI(TAG, "GPS init completed");

  load_cell_data_t load_cell_data[2];
  LOGI(TAG, "Initializing Load Cells...");
  for (size_t i = 0; i < 2; i++) {
    load_cell_sensor_init(&load_cell_data[i]);
  }
  LOGI(TAG, "Load cells init completed");

  pressure_sensor_data_t pressure_data[2];
  LOGI(TAG, "Initializing Pressure Sensors...");
  for (size_t i = 0; i < 2; i++) {
    pressure_sensor_init(&pressure_data[i]);
  }
  LOGI(TAG, "Pressure sensors init completed");

  /* ---- Flow sensor init -------------------------------------------------- */
  /*
   * g_flow_data is made static so the EXTI ISR callback (in stm32h7xx_it.c)
   * can reach it via:  extern flow_sensor_data_t g_flow_data;
   */
  static flow_sensor_data_t g_flow_data;
  LOGI(TAG, "Initializing Flow Sensor...");
  result_t flow_init_result = flow_sensor_init(&g_flow_data);
  if (flow_init_result != RESULT_OK) {
    LOGE(TAG, "Flow sensor init failed: %s",
         result_to_short_str(flow_init_result));
  } else {
    LOGI(TAG, "Flow sensor initialized");
  }

  /* ---- Pump init --------------------------------------------------------- */
  /* TODO: Enable when MX_TIM3_Init is generated by CubeMX
  pump_hw_t pump_hw = {
      .htim = &htim3,
      .tim_channel = TIM_CHANNEL_1,
      .tim_period = htim3.Init.Period,
      .dir_port = GPIOB,
      .dir_pin = GPIO_PIN_0,
      .en_port = GPIOB,
      .en_pin = GPIO_PIN_1,
  };

  LOGI(TAG, "Initializing Pump...");
  result_t pump_init_result = pump_init(&g_pump_data, &pump_hw);
  if (pump_init_result != RESULT_OK) {
    LOGE(TAG, "Pump init failed: %s", result_to_short_str(pump_init_result));
  } else {
    LOGI(TAG, "Pump initialized");
    pump_set_direction(&g_pump_data, true);
    pump_set_speed_percent(&g_pump_data, 50U);
    pump_set_enabled(&g_pump_data, true);
  }
  */

  BSP_LED_Toggle(LED_GREEN);

  /* ---- Stepper motor init ------------------------------------------------ */
  /* TODO: STEPPER MOTOR STUB
   * ===========================
   * User will implement the stepper motor library.
   * This section provides the skeleton for integration.
   * 
   * IMPLEMENTATION CHECKLIST:
   * [ ] Create stepper_motor.h/c in components/sensor_board/stepper/
   * [ ] Define stepper_motor_hw_t structure (GPIO ports, pins, timer, etc.)
   * [ ] Implement stepper_motor_init() - configure GPIO, timer, state machine
   * [ ] Implement stepper_motor_open() - start opening sequence
   * [ ] Implement stepper_motor_close() - start closing sequence
   * [ ] Implement stepper_motor_stop() - emergency stop
   * [ ] Implement stepper_motor_update() - update state machine (call in main loop)
   * [ ] Implement stepper_motor_is_moving() - check if actively moving
   * [ ] Add protobuf message definition in ERC-Protobufs/ if needed
   * 
   * EXPECTED HARDWARE INTERFACE:
   * - Stepper motor driver (e.g., DRV8825, A4988) with:
   *   - Direction pin (GPIO)
   *   - Step pin (GPIO or PWM timer)
   *   - Enable pin (GPIO)
   * - Limit switches for open/closed positions (optional EXTI)
   * - Encoder/potentiometer for position feedback (optional ADC)
   */

  /* Stepper motor data structure - IMPLEMENT IN stepper_motor.h
  typedef struct {
    // Position tracking
    int32_t current_position;      // Current position in steps (or mm)
    int32_t target_position;       // Target position
    
    // State machine
    enum {
      STEPPER_STATE_IDLE,
      STEPPER_STATE_OPENING,
      STEPPER_STATE_CLOSING,
      STEPPER_STATE_ERROR,
      STEPPER_STATE_STOPPED
    } state;
    
    // Control parameters
    uint32_t step_delay_us;        // Microseconds between steps
    uint16_t speed_percent;        // 0-100% speed
    bool direction;                // true = open, false = close
    bool enabled;                  // Enable/disable motor
    bool is_initialised;           // Init flag
    
    // Hardware reference
    TIM_HandleTypeDef *htim_step;  // Timer for step pulse
    uint32_t tim_channel;          // Timer channel
    GPIO_TypeDef *dir_port;        // Direction GPIO port
    uint16_t dir_pin;              // Direction GPIO pin
    GPIO_TypeDef *en_port;         // Enable GPIO port
    uint16_t en_pin;               // Enable GPIO pin
    
    // Optional: limit switches
    bool has_open_limit;
    bool has_close_limit;
    bool open_limit_triggered;
    bool close_limit_triggered;
  } stepper_motor_data_t;
  */

  /* Example: Declare stepper motor instance
  static stepper_motor_data_t g_stepper_motor;
  
  // Configure hardware interface
  stepper_motor_hw_t stepper_hw = {
    .htim_step = &htim1,           // Adjust to your timer
    .tim_channel = TIM_CHANNEL_1,
    .dir_port = GPIOB,
    .dir_pin = GPIO_PIN_2,
    .en_port = GPIOB,
    .en_pin = GPIO_PIN_3,
  };
  
  LOGI(TAG, "Initializing Stepper Motor...");
  result_t stepper_init_result = stepper_motor_init(&g_stepper_motor, &stepper_hw);
  if (stepper_init_result != RESULT_OK) {
    LOGE(TAG, "Stepper motor init failed: %s", result_to_short_str(stepper_init_result));
  } else {
    LOGI(TAG, "Stepper motor initialized");
  }
  */

  /* ---- UDP init ---------------------------------------------------------- */
  LOGI(TAG, "Initializing UDP...");
  
  int SendQueueSize = 80;
  static StaticQueue_t txStruct0;
  static uint8_t txStorage0[80 * sizeof(send_frame_t)];
  QueueHandle_t tx0 = xQueueCreateStatic(
      SendQueueSize, sizeof(send_frame_t), txStorage0, &txStruct0);

  static StaticQueue_t txStruct1;
  static uint8_t txStorage1[80 * sizeof(send_frame_t)];
  QueueHandle_t tx1 = xQueueCreateStatic(
      SendQueueSize, sizeof(send_frame_t), txStorage1, &txStruct1);

  if (tx0 == NULL || tx1 == NULL) {
    LOGE(TAG, "CRITICAL: Failed to create UDP send queues!");
    Error_Handler();
  }

  QueueHandle_t send_queues[2] = {tx0, tx1};
  ETH_udp_init(2, send_queues, sensor_udp_rx_callback);
  LOGI(TAG, "UDP initialized successfully");

  /* ---- Main loop --------------------------------------------------------- */
  uint32_t loop_count = 0;
  LOGI(TAG, "========== ENTERING MAIN LOOP ==========");
  LOGI(TAG, "Starting main sensor loop...");
  const bool skip_sensor_polling = false; // need to make false to pool data

  uint8_t dest_ip[4] = {192, 168, 0, 255};

  while (1) {
    LOGI(TAG, "=== Main loop iteration %lu ===", loop_count);
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
      diagnostics.has_gps_sensor_1 = false;
      diagnostics.has_imu_sensor = false;
    } else {

      /* ---------- pH sensor -------------------------------------------------
       */
      float ph_value, ph_voltage;
      result_t ph_poll_result = poll_ph_sensor(&ph_sensor);
      diagnostics.has_ph_sensor = true;
      if (ph_poll_result == RESULT_ERR_UNIMPLEMENTED ||
          ph_poll_result == RESULT_ERR_COMMS) {
        LOGW(TAG, "pH Sensor - Not connected (result: %d)", ph_poll_result);
        diagnostics.ph_sensor.state = SensorState_SENSOR_IDLE;
        diagnostics.ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
        diagnostics.ph_sensor.ph_value = 0.0f;
        diagnostics.ph_sensor.voltage = 0.0f;
      } else if (ph_poll_result != RESULT_OK) {
        LOGE(TAG, "pH Sensor - Poll error: %d", ph_poll_result);
        diagnostics.ph_sensor.state = SensorState_SENSOR_ERROR;
        diagnostics.ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
        diagnostics.ph_sensor.ph_value = 0.0f;
        diagnostics.ph_sensor.voltage = 0.0f;
      } else {
        if (ph_sensor_get_value(&ph_sensor, &ph_value) == RESULT_OK) {
          ph_sensor_get_voltage(&ph_sensor, &ph_voltage);
          if (validate_ph_value(ph_value) == RESULT_OK) {
            diagnostics.ph_sensor.ph_value = ph_value;
            diagnostics.ph_sensor.voltage = ph_voltage;
            diagnostics.ph_sensor.state = SensorState_SENSOR_OPERATING;
            diagnostics.ph_sensor.error_code = PHErrorCode_PH_NO_ERROR;
          } else {
            LOGW(TAG, "pH Sensor - Invalid value: %.2f", ph_value);
            diagnostics.ph_sensor.ph_value = ph_value;
            diagnostics.ph_sensor.voltage = ph_voltage;
            diagnostics.ph_sensor.state = SensorState_SENSOR_ERROR;
            diagnostics.ph_sensor.error_code = PHErrorCode_PH_INVALID_DATA;
          }
        } else {
          LOGE(TAG, "pH Sensor - Failed to read value");
          diagnostics.ph_sensor.state = SensorState_SENSOR_ERROR;
          diagnostics.ph_sensor.error_code =
              PHErrorCode_PH_COMMUNICATION_FAILURE;
          diagnostics.ph_sensor.ph_value = 0.0f;
          diagnostics.ph_sensor.voltage = 0.0f;
        }
      }

      /* ---------- GPS -------------------------------------------------------
       */
      result_t gps_poll_result = poll_gps_sensor(&gps_data);
      double lat, lon;
      float altitude, speed, heading;
      gps_fix_quality_t fix_quality;
      int32_t satellites;
      bool gps_valid;

      diagnostics.has_gps_sensor_1 = true;
      if (gps_poll_result == RESULT_ERR_UNIMPLEMENTED ||
          gps_poll_result == RESULT_ERR_COMMS) {
        LOGW(TAG, "GPS - Not connected (result: %d)", gps_poll_result);
        diagnostics.gps_sensor_1.state = SensorState_SENSOR_IDLE;
        diagnostics.gps_sensor_1.error_code =
            GPSErrorCode_GPS_COMMUNICATION_FAILURE;
        diagnostics.gps_sensor_1.latitude = 0.0;
        diagnostics.gps_sensor_1.longitude = 0.0;
        diagnostics.gps_sensor_1.altitude = 0.0f;
        diagnostics.gps_sensor_1.speed = 0.0f;
        diagnostics.gps_sensor_1.heading = 0.0f;
        diagnostics.gps_sensor_1.hdop = 99.9f;
        diagnostics.gps_sensor_1.vdop = 99.9f;
        diagnostics.gps_sensor_1.satellites = 0;
        diagnostics.gps_sensor_1.fix_quality = GPSFixQuality_NO_FIX;
      } else if (gps_poll_result != RESULT_OK) {
        LOGE(TAG, "GPS - Poll error: %d", gps_poll_result);
        diagnostics.gps_sensor_1.state = SensorState_SENSOR_ERROR;
        diagnostics.gps_sensor_1.error_code =
            GPSErrorCode_GPS_COMMUNICATION_FAILURE;
      } else if (gps_sensor_is_valid(&gps_data, &gps_valid) == RESULT_OK &&
                 gps_valid) {
        diagnostics.gps_sensor_1.state = SensorState_SENSOR_OPERATING;
        diagnostics.gps_sensor_1.error_code = GPSErrorCode_GPS_NO_ERROR;
        if (gps_sensor_get_coordinates(&gps_data, &lat, &lon) == RESULT_OK) {
          diagnostics.gps_sensor_1.latitude = lat;
          diagnostics.gps_sensor_1.longitude = lon;
        }
        if (gps_sensor_get_altitude(&gps_data, &altitude) == RESULT_OK) {
          diagnostics.gps_sensor_1.altitude = altitude;
        }
        if (gps_sensor_get_velocity(&gps_data, &speed, &heading) == RESULT_OK) {
          diagnostics.gps_sensor_1.speed = speed;
          diagnostics.gps_sensor_1.heading = heading;
        }
        if (gps_sensor_get_fix_info(&gps_data, &fix_quality, &satellites) ==
            RESULT_OK) {
          diagnostics.gps_sensor_1.fix_quality = (GPSFixQuality)fix_quality;
          diagnostics.gps_sensor_1.satellites = satellites;
        }
        diagnostics.gps_sensor_1.hdop = gps_data.hdop;
        diagnostics.gps_sensor_1.vdop = gps_data.vdop;
      } else {
        LOGW(TAG, "GPS - No valid data received");
        diagnostics.gps_sensor_1.state = SensorState_SENSOR_ERROR;
        diagnostics.gps_sensor_1.error_code = GPSErrorCode_GPS_INVALID_DATA;
      }

      /* ---------- IMU -------------------------------------------------------
       */
      result_t imu_poll_result = poll_imu_sensor(&imu_data);
      diagnostics.has_imu_sensor = true;
      if (imu_poll_result == RESULT_ERR_UNIMPLEMENTED ||
          imu_poll_result == RESULT_ERR_COMMS) {
        LOGW(TAG, "IMU - Not connected (result: %d)", imu_poll_result);
        diagnostics.imu_sensor.state = SensorState_SENSOR_IDLE;
        diagnostics.imu_sensor.error_code =
            IMUErrorCode_IMU_COMMUNICATION_FAILURE;
        diagnostics.imu_sensor.accel_x = 0.0f;
        diagnostics.imu_sensor.accel_y = 0.0f;
        diagnostics.imu_sensor.accel_z = 0.0f;
        diagnostics.imu_sensor.gyro_x = 0.0f;
        diagnostics.imu_sensor.gyro_y = 0.0f;
        diagnostics.imu_sensor.gyro_z = 0.0f;
        diagnostics.imu_sensor.mag_x = 0.0f;
        diagnostics.imu_sensor.mag_y = 0.0f;
        diagnostics.imu_sensor.mag_z = 0.0f;
      } else if (imu_poll_result != RESULT_OK) {
        LOGE(TAG, "IMU - Poll error: %d", imu_poll_result);
        diagnostics.imu_sensor.state = SensorState_SENSOR_ERROR;
        diagnostics.imu_sensor.error_code =
            IMUErrorCode_IMU_COMMUNICATION_FAILURE;
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
      }
      if (!imu_validate_accelerometer_range(&imu_data)) {
        LOGW(TAG, "IMU - Accelerometer out of range");
        diagnostics.imu_sensor.error_code =
            IMUErrorCode_IMU_ACCELEROMETER_ERROR;
      }
      if (!imu_validate_gyroscope_range(&imu_data)) {
        LOGW(TAG, "IMU - Gyroscope out of range");
        diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_GYROSCOPE_ERROR;
      }
      if (!imu_validate_magnetometer_range(&imu_data)) {
        LOGW(TAG, "IMU - Magnetometer out of range");
        diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_MAGNETOMETER_ERROR;
      }

      /* ---------- Load cells + pressure (index loop) -----------------------
       */
      for (size_t i = 0; i < 2; i++) {
        /* Load cell */
        SensorBoardLoadCellInfo load_cell_info =
            SensorBoardLoadCellInfo_init_zero;
        result_t lc_result = poll_load_cell_sensor(&load_cell_data[i]);
        float lc_force = 0.0f;
        float lc_mass = 0.0f;
        int32_t lc_counts = 0;
        float lc_scale = 0.0f;
        int32_t lc_tare = 0;
        bool lc_valid = false;
        load_cell_info.sensor_index = (uint32_t)i;
        if (lc_result == RESULT_ERR_UNIMPLEMENTED ||
            lc_result == RESULT_ERR_COMMS) {
          LOGW(TAG, "Load cell %lu - Not connected", (unsigned long)i);
          load_cell_info.state = SensorState_SENSOR_IDLE;
          load_cell_info.error_code =
              LoadCellErrorCode_LOAD_CELL_COMMUNICATION_FAILURE;
        } else if (lc_result != RESULT_OK) {
          LOGE(TAG, "Load cell %lu - Poll error: %d", (unsigned long)i,
               lc_result);
          load_cell_info.state = SensorState_SENSOR_ERROR;
          load_cell_info.error_code =
              LoadCellErrorCode_LOAD_CELL_COMMUNICATION_FAILURE;
        } else {
          if (load_cell_get_force_newtons(&load_cell_data[i], &lc_force) ==
                  RESULT_OK &&
              load_cell_get_mass_grams(&load_cell_data[i], &lc_mass) ==
                  RESULT_OK &&
              load_cell_get_raw_counts(&load_cell_data[i], &lc_counts) ==
                  RESULT_OK &&
              load_cell_get_calibration(&load_cell_data[i], &lc_scale,
                                        &lc_tare) == RESULT_OK) {
            load_cell_sensor_is_valid(&load_cell_data[i], &lc_valid);
            if (lc_valid) {
              load_cell_info.state = SensorState_SENSOR_OPERATING;
              load_cell_info.error_code = LoadCellErrorCode_LOAD_CELL_NO_ERROR;
            } else {
              LOGW(TAG, "Load cell %lu - Invalid data", (unsigned long)i);
              load_cell_info.state = SensorState_SENSOR_ERROR;
              load_cell_info.error_code =
                  LoadCellErrorCode_LOAD_CELL_INVALID_DATA;
            }
          } else {
            LOGE(TAG, "Load cell %lu - Failed to read values",
                 (unsigned long)i);
            load_cell_info.state = SensorState_SENSOR_ERROR;
            load_cell_info.error_code =
                LoadCellErrorCode_LOAD_CELL_COMMUNICATION_FAILURE;
          }
        }
        load_cell_info.force_newtons = lc_force;
        load_cell_info.mass_grams = lc_mass;
        load_cell_info.raw_counts = lc_counts;
        load_cell_info.scale_newtons_per_count = lc_scale;
        load_cell_info.tare_offset_counts = lc_tare;
        load_cell_info.is_calibrated = load_cell_data[i].is_calibrated;
        result_t send_r = send_load_cell_info_udp(dest_ip, SENSOR_UDP_DEST_PORT,
                                                  &load_cell_info, 1);
        if (send_r != RESULT_OK) {
          LOGE(TAG, "Load cell %lu UDP send failed: %s", (unsigned long)i,
               result_to_short_str(send_r));
        }

        /* Pressure sensor */
        SensorBoardPressureInfo pressure_info =
            SensorBoardPressureInfo_init_zero;
        result_t pr_result = poll_pressure_sensor(&pressure_data[i]);
        float pr_kpa = 0.0f;
        float pr_temp = 0.0f;
        float pr_voltage = 0.0f;
        bool pr_valid = false;
        pressure_info.sensor_index = (uint32_t)i;
        if (pr_result == RESULT_ERR_UNIMPLEMENTED ||
            pr_result == RESULT_ERR_COMMS) {
          LOGW(TAG, "Pressure %lu - Not connected", (unsigned long)i);
          pressure_info.state = SensorState_SENSOR_IDLE;
          pressure_info.error_code =
              PressureErrorCode_PRESSURE_COMMUNICATION_FAILURE;
        } else if (pr_result != RESULT_OK) {
          LOGE(TAG, "Pressure %lu - Poll error: %d", (unsigned long)i,
               pr_result);
          pressure_info.state = SensorState_SENSOR_ERROR;
          pressure_info.error_code =
              PressureErrorCode_PRESSURE_COMMUNICATION_FAILURE;
        } else {
          if (pressure_sensor_get_pressure_kpa(&pressure_data[i], &pr_kpa) ==
                  RESULT_OK &&
              pressure_sensor_get_temperature_c(&pressure_data[i], &pr_temp) ==
                  RESULT_OK &&
              pressure_sensor_get_voltage(&pressure_data[i], &pr_voltage) ==
                  RESULT_OK) {
            pressure_sensor_is_valid(&pressure_data[i], &pr_valid);
            if (pr_valid) {
              pressure_info.state = SensorState_SENSOR_OPERATING;
              pressure_info.error_code = PressureErrorCode_PRESSURE_NO_ERROR;
            } else {
              LOGW(TAG, "Pressure %lu - Invalid data", (unsigned long)i);
              pressure_info.state = SensorState_SENSOR_ERROR;
              pressure_info.error_code =
                  PressureErrorCode_PRESSURE_INVALID_DATA;
            }
          } else {
            LOGE(TAG, "Pressure %lu - Failed to read values", (unsigned long)i);
            pressure_info.state = SensorState_SENSOR_ERROR;
            pressure_info.error_code =
                PressureErrorCode_PRESSURE_COMMUNICATION_FAILURE;
          }
        }
        pressure_info.pressure_kpa = pr_kpa;
        pressure_info.temperature_c = pr_temp;
        pressure_info.voltage = pr_voltage;
        pressure_info.is_calibrated = pressure_data[i].is_calibrated;
        send_r = send_pressure_info_udp(dest_ip, SENSOR_UDP_DEST_PORT,
                                        &pressure_info, 1);
        if (send_r != RESULT_OK) {
          LOGE(TAG, "Pressure %lu UDP send failed: %s", (unsigned long)i,
               result_to_short_str(send_r));
        }
      }

    } /* end skip_sensor_polling */

    /* ==========================================================================
     * Flow Sensor — poll + build proto + send
     * ==========================================================================
     * poll_flow_sensor() computes flow rate from pulses counted by the ISR
     * since the last call.  Called every MAIN_TASK_DELAY_MS (5000 ms).
     */
    {
      SensorBoardFlowSensorInfo flow_info = SensorBoardFlowSensorInfo_init_zero;
      result_t flow_poll_result = poll_flow_sensor(&g_flow_data);

      if (flow_poll_result == RESULT_ERR_UNIMPLEMENTED ||
          flow_poll_result == RESULT_ERR_COMMS) {
        LOGW(TAG, "Flow Sensor - Not connected (result: %d)", flow_poll_result);
        flow_info.state = SensorState_SENSOR_IDLE;
        flow_info.status = SensorStatus_STATUS_DISCONNECTED;
        flow_info.flow_rate_ml_min_x100 = 0U;
        flow_info.total_volume_ml = 0U;
        flow_info.pulse_count = 0U;
      } else if (flow_poll_result != RESULT_OK) {
        LOGE(TAG, "Flow Sensor - Poll error: %d", flow_poll_result);
        flow_info.state = SensorState_SENSOR_ERROR;
        flow_info.status = SensorStatus_STATUS_ERROR;
      } else {
        bool flow_valid = false;
        flow_sensor_is_valid(&g_flow_data, &flow_valid);

        if (flow_valid) {
          uint32_t rate = 0U;
          uint32_t vol_ml = 0U;
          uint32_t pulses = 0U;
          flow_sensor_get_flow_rate(&g_flow_data, &rate);
          flow_sensor_get_total_volume_ml(&g_flow_data, &vol_ml);
          flow_sensor_get_pulse_count(&g_flow_data, &pulses);

          flow_info.flow_rate_ml_min_x100 = rate;
          flow_info.total_volume_ml = vol_ml;
          flow_info.pulse_count = pulses;
          flow_info.state = SensorState_SENSOR_OPERATING;
          flow_info.status = SensorStatus_STATUS_OK;

          /* Log as integer to avoid float (rate/100 = ml/min integer part) */
          LOGI(TAG, "Flow: %lu.%02lu ml/min, total: %lu ml, pulses: %lu",
               (unsigned long)(rate / 100U), (unsigned long)(rate % 100U),
               (unsigned long)vol_ml, (unsigned long)pulses);
        } else {
          /* First window not yet complete */
          flow_info.state = SensorState_SENSOR_IDLE;
          flow_info.status = SensorStatus_STATUS_INITIALIZING;
        }
      }

      result_t flow_send_r = send_flow_sensor_info_udp(
          dest_ip, SENSOR_UDP_DEST_PORT, &flow_info, 1);
      if (flow_send_r != RESULT_OK) {
        LOGE(TAG, "Flow sensor UDP send failed: %s",
             result_to_short_str(flow_send_r));
      }
    }

    /* ==========================================================================
     * Pump — build proto from current state + send status
     * ==========================================================================
     * The pump is actuated via network commands (handle_sensor_pump_command).
     * Here we just report the current state back to the network every loop.
     */
    {
      SensorBoardPumpInfo pump_info = SensorBoardPumpInfo_init_zero;

      if (!g_pump_data.is_initialised) {
        pump_info.state = SensorState_SENSOR_ERROR;
        pump_info.status = SensorStatus_STATUS_ERROR;
      } else {
        pump_get_enabled(&g_pump_data, &pump_info.enabled);
        pump_get_direction(&g_pump_data, &pump_info.direction);
        pump_get_speed_percent(&g_pump_data, &pump_info.speed_percent);
        pump_get_speed_rpm(&g_pump_data, &pump_info.speed_rpm);
        pump_info.state = g_pump_data.enabled ? SensorState_SENSOR_OPERATING
                                              : SensorState_SENSOR_IDLE;
        pump_info.status = SensorStatus_STATUS_OK;
      }

      result_t pump_send_r =
          send_pump_info_udp(dest_ip, SENSOR_UDP_DEST_PORT, &pump_info, 1);
      if (pump_send_r != RESULT_OK) {
        LOGE(TAG, "Pump UDP send failed: %s", result_to_short_str(pump_send_r));
      }
    }

    /* ==========================================================================
     * Stepper Motor — state machine update
     * ==========================================================================
     * TODO: STEPPER MOTOR STUB - STATE MACHINE UPDATE
     * 
     * Call the stepper motor state machine update function here.
     * This will:
     * - Decrement step timers
     * - Generate step pulses
     * - Check limit switches
     * - Update position tracking
     * - Handle error conditions
     * 
     * IMPLEMENTATION:
     * if (g_stepper_motor.is_initialised) {
     *   stepper_motor_update(&g_stepper_motor);
     *   
     *   // Optionally: send status over network every N iterations
     *   if (loop_count % 1 == 0) {  // Every loop, or adjust as needed
     *     SensorBoardStepperMotorInfo stepper_info = 
     *       SensorBoardStepperMotorInfo_init_zero;
     *     stepper_info.position = g_stepper_motor.current_position;
     *     stepper_info.is_moving = stepper_motor_is_moving(&g_stepper_motor);
     *     stepper_info.state = (uint32_t)g_stepper_motor.state;
     *     // send_stepper_motor_info_udp(dest_ip, SENSOR_UDP_DEST_PORT, 
     *     //                            &stepper_info, 1);
     *   }
     * }
     */
    // stepper_motor_update(&g_stepper_motor);  // UNCOMMENT when implemented

    /* ==========================================================================
     * Stepper Motor Commands — example handler for network commands
     * ==========================================================================
     * TODO: STEPPER MOTOR STUB - NETWORK COMMAND HANDLER
     * 
     * Add this handler function above with other packet handlers:
     * 
     * static result_t handle_stepper_motor_command(void *buffer) {
     *   if (buffer == NULL) {
     *     return RESULT_ERR_INVALID_ARG;
     *   }
     *   extern stepper_motor_data_t g_stepper_motor;
     *   SensorBoardStepperMotorCommand *cmd = 
     *     (SensorBoardStepperMotorCommand *)buffer;
     *   
     *   LOGI(TAG, "Stepper cmd: action=%d, speed=%lu%%", 
     *        cmd->action, (unsigned long)cmd->speed_percent);
     *   
     *   switch (cmd->action) {
     *     case STEPPER_ACTION_OPEN:
     *       stepper_motor_open(&g_stepper_motor);
     *       break;
     *     case STEPPER_ACTION_CLOSE:
     *       stepper_motor_close(&g_stepper_motor);
     *       break;
     *     case STEPPER_ACTION_STOP:
     *       stepper_motor_stop(&g_stepper_motor);
     *       break;
     *     default:
     *       return RESULT_ERR_INVALID_ARG;
     *   }
     *   stepper_motor_set_speed_percent(&g_stepper_motor, cmd->speed_percent);
     *   return RESULT_OK;
     * }
     * 
     * Then register in handler_configs array:
     * PACKET_HANDLER_CONFIG_STATIC(stepper_cmd_handler, 
     *                              PBEnvelope_stepper_cmd_tag,
     *                              stepper_cmd, 
     *                              handle_stepper_motor_command);
     */

    /* Send overall diagnostics envelope */
    send_sensor_diagnostics_udp(dest_ip, SENSOR_UDP_DEST_PORT, &diagnostics, 1);

    osDelay(MAIN_TASK_DELAY_MS);
  }
}

#endif //! PIO_UNIT_TESTING
