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
#include "cmsis_os2.h"
#include "ethernet.h"
#include "gpio.h"
#include "logging.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "pb_encode.h"
#include "tim.h"
#include "udp.h"
#include <stdint.h>
#include <string.h>

#include "gps_sensor.h"
#include "imu_sensor.h"
#include "ph_sensor.h"
#include "sensor_basics.h"

// Protobuf includes
#include "components/sensor_board/diagnostics.pb.h"
#include "components/sensor_board/gps_sensor.pb.h"
#include "components/sensor_board/imu_sensor.pb.h"
#include "components/sensor_board/ph_sensor.pb.h"
#include "components/sensor_board/sensor.pb.h"

#define TAG "MAIN"
#define MAIN_TASK_DELAY_MS 100U
#define SENSOR_BOARD_PROTOBUF_BUFFER_SIZE 256U
#define SENSOR_BOARD_UDP_LOCAL_PORT 8U
#define SENSOR_BOARD_UDP_PORT 7U
#define SENSOR_BOARD_NETWORK_STARTUP_DELAY_MS 250U

extern void MX_FREERTOS_Init(void);
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
void Error_Handler(void);

void MainTask(void *argument);
static void SensorBoard_ApplyStaticNetwork(void);
static void SensorBoard_RebindUdpClient(void);
static void SensorBoard_ConfigureDestinationRoute(void);
static void SensorBoard_InitSensors(imu_data_t *imu_data, ph_sensor_t *ph_sensor,
                                    gps_data_t *gps_data);
static void SensorBoard_UpdateDiagnostics(SensorDiagnostics *diagnostics,
                                          imu_data_t *imu_data,
                                          ph_sensor_t *ph_sensor,
                                          gps_data_t *gps_data);
static void SensorBoard_SendDiagnostics(
  const SensorDiagnostics *diagnostics);

extern TIM_HandleTypeDef htim1;
COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;

typedef struct {
  uint32_t timestamp;
  float temperature;
  float humidity;
  float pressure;
} SensorPacket;

static uint32_t sensor_board_send_interval_ms = MAIN_TASK_DELAY_MS;
static uint8_t sensor_board_ip[4] = {192, 168, 10, 2};
static uint8_t sensor_board_netmask[4] = {255, 255, 255, 0};
static uint8_t sensor_board_gateway[4] = {0, 0, 0, 0};
static uint8_t laptop_ip[4] = {192, 168, 10, 1};
static uint8_t laptop_mac[6] = {0x58, 0x11, 0x22, 0x3D, 0x88, 0xFC};
static int laptop_mac_filter[6] = {0x58, 0x11, 0x22, 0x3D, 0x88, 0xFC};

const osThreadAttr_t mainTask_attributes = {
    .name = "mainTask",
    .stack_size = 1024 * 8,
    .priority = (osPriority_t)osPriorityNormal,
};

void uart_setup() {
  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits = COM_STOPBITS_1;
  BspCOMInit.Parity = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
    Error_Handler();
  }
  MX_USART3_Init(&huart_com, &BspCOMInit);
}

void init_board() {
  MPU_Config_wrapper();
  
  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_Init();
  SystemClock_Config();
  
  osKernelInitialize();
  MX_GPIO_Init();
  MX_TIM1_Init();

  uart_setup();
  LOG_init(&huart_com);

  // Direct UART check to verify hardware/handle state before RTOS starts
  char *boot_msg = "\r\n[BOOT] System Initialized. Starting Kernel...\r\n";
  HAL_UART_Transmit(&huart_com, (uint8_t*)boot_msg, strlen(boot_msg), 100);

  // Note: ETH_init is called by StartDefaultTask after RTOS starts
  // MAC filtering will be applied in MainTask after network is ready

  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osKernelStart();

  while (1) {
  }
}

int main(void) { 
  init_board(); 
}

static void SensorBoard_ApplyStaticNetwork(void) {
  extern struct netif gnetif;
  extern uint8_t GATEWAY_ADDRESS[4];
  extern uint8_t IP_ADDRESS[4];
  extern uint8_t NETMASK_ADDRESS[4];

  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gateway;
  err_t network_result;

  memcpy(IP_ADDRESS, sensor_board_ip, sizeof(sensor_board_ip));
  memcpy(NETMASK_ADDRESS, sensor_board_netmask, sizeof(sensor_board_netmask));
  memcpy(GATEWAY_ADDRESS, sensor_board_gateway, sizeof(sensor_board_gateway));

  IP4_ADDR(&ipaddr, sensor_board_ip[0], sensor_board_ip[1], sensor_board_ip[2],
           sensor_board_ip[3]);
  IP4_ADDR(&netmask, sensor_board_netmask[0], sensor_board_netmask[1],
           sensor_board_netmask[2], sensor_board_netmask[3]);
  IP4_ADDR(&gateway, sensor_board_gateway[0], sensor_board_gateway[1],
           sensor_board_gateway[2], sensor_board_gateway[3]);

  // Apply the required static IPv4 configuration after LWIP is initialized.
  netif_set_addr(&gnetif, &ipaddr, &netmask, &gateway);
  network_result = ERR_OK;
  if (network_result == ERR_OK) {
    LOGI(TAG, "Static IP set to %u.%u.%u.%u", sensor_board_ip[0],
         sensor_board_ip[1], sensor_board_ip[2], sensor_board_ip[3]);
  } else {
    LOGE(TAG, "Failed to apply static IP configuration (%d)", network_result);
  }
}

static void SensorBoard_ConfigureDestinationRoute(void) {
  result_t arp_result;

  // Program the laptop MAC so sends can proceed without broad filtering.
  ETH_setup_MAC_address_filtering(laptop_mac_filter, NULL, NULL);

  // Add a static ARP entry so the board can send to the laptop immediately.
  arp_result = ETH_add_arp(laptop_ip, laptop_mac);
  if (arp_result == RESULT_OK) {
    LOGI(TAG, "Static ARP entry added for %u.%u.%u.%u", laptop_ip[0],
         laptop_ip[1], laptop_ip[2], laptop_ip[3]);
  } else {
    LOGW(TAG, "Static ARP entry could not be added (%d)", arp_result);
  }
}

static void SensorBoard_RebindUdpClient(void) {
  extern struct udp_pcb *upcb;

  err_t bind_result;
  ip_addr_t local_addr;
  struct udp_pcb *new_udp_client;

  // ETH_udp_init() creates the shared PCB; replace its hardcoded bind address
  // from sensor-board code so traffic originates from the required static IP.
  new_udp_client = udp_new();
  if (new_udp_client == NULL) {
    LOGE(TAG, "Failed to allocate UDP client for static IP rebinding");
    return;
  }

  IP_ADDR4(&local_addr, sensor_board_ip[0], sensor_board_ip[1],
           sensor_board_ip[2], sensor_board_ip[3]);
  bind_result = udp_bind(new_udp_client, &local_addr, SENSOR_BOARD_UDP_LOCAL_PORT);
  if (bind_result != ERR_OK) {
    LOGE(TAG, "Failed to bind UDP client to static IP (%d)", bind_result);
    udp_remove(new_udp_client);
    return;
  }

  if (upcb != NULL) {
    udp_remove(upcb);
  }
  upcb = new_udp_client;
  LOGI(TAG, "UDP client rebound to %u.%u.%u.%u:%u", sensor_board_ip[0],
       sensor_board_ip[1], sensor_board_ip[2], sensor_board_ip[3],
       SENSOR_BOARD_UDP_LOCAL_PORT);
}

static void SensorBoard_InitSensors(imu_data_t *imu_data, ph_sensor_t *ph_sensor,
                                    gps_data_t *gps_data) {
  LOGI(TAG, "Initializing IMU...");
  imu_sensor_init(imu_data);

  LOGI(TAG, "Initializing pH sensor...");
  ph_sensor_init(ph_sensor, 3.3f);

  LOGI(TAG, "Initializing GPS...");
  gps_sensor_init(gps_data);
}

static void SensorBoard_UpdateDiagnostics(SensorDiagnostics *diagnostics,
                                          imu_data_t *imu_data,
                                          ph_sensor_t *ph_sensor,
                                          gps_data_t *gps_data) {
  float ph_value = 0.0f;
  float ph_voltage = 0.0f;
  float altitude = 0.0f;
  float heading = 0.0f;
  float speed = 0.0f;
  double latitude = 0.0;
  double longitude = 0.0;
  bool gps_valid = false;
  gps_fix_quality_t fix_quality = GPS_NO_FIX;
  int32_t satellites = 0;
  result_t gps_poll_result;
  result_t imu_poll_result;
  result_t ph_poll_result;

  *diagnostics = SensorDiagnostics_init_zero;
  diagnostics->state = SensorDiagnostics_State_OPERATING;
  diagnostics->has_ph_sensor = true;
  diagnostics->has_gps_sensor_1 = true;
  diagnostics->has_imu_sensor = true;

  ph_poll_result = poll_ph_sensor(ph_sensor);
  if (ph_poll_result == RESULT_OK &&
      ph_sensor_get_value(ph_sensor, &ph_value) == RESULT_OK &&
      ph_sensor_get_voltage(ph_sensor, &ph_voltage) == RESULT_OK) {
    diagnostics->ph_sensor.ph_value = ph_value;
    diagnostics->ph_sensor.voltage = ph_voltage;
    diagnostics->ph_sensor.state =
        validate_ph_value(ph_value) == RESULT_OK ? SensorState_SENSOR_OPERATING
                                                 : SensorState_SENSOR_ERROR;
    diagnostics->ph_sensor.error_code =
        diagnostics->ph_sensor.state == SensorState_SENSOR_OPERATING
            ? PHErrorCode_PH_NO_ERROR
            : PHErrorCode_PH_INVALID_DATA;
  } else {
    diagnostics->ph_sensor.state =
        ph_poll_result == RESULT_ERR_UNIMPLEMENTED ||
                ph_poll_result == RESULT_ERR_COMMS
            ? SensorState_SENSOR_IDLE
            : SensorState_SENSOR_ERROR;
    diagnostics->ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
  }

  gps_poll_result = poll_gps_sensor(gps_data);
  if (gps_poll_result == RESULT_OK &&
      gps_sensor_is_valid(gps_data, &gps_valid) == RESULT_OK && gps_valid) {
    diagnostics->gps_sensor_1.state = SensorState_SENSOR_OPERATING;
    diagnostics->gps_sensor_1.error_code = GPSErrorCode_GPS_NO_ERROR;

    if (gps_sensor_get_coordinates(gps_data, &latitude, &longitude) == RESULT_OK) {
      diagnostics->gps_sensor_1.latitude = latitude;
      diagnostics->gps_sensor_1.longitude = longitude;
    }
    if (gps_sensor_get_altitude(gps_data, &altitude) == RESULT_OK) {
      diagnostics->gps_sensor_1.altitude = altitude;
    }
    if (gps_sensor_get_velocity(gps_data, &speed, &heading) == RESULT_OK) {
      diagnostics->gps_sensor_1.speed = speed;
      diagnostics->gps_sensor_1.heading = heading;
    }
    if (gps_sensor_get_fix_info(gps_data, &fix_quality, &satellites) == RESULT_OK) {
      diagnostics->gps_sensor_1.fix_quality = (GPSFixQuality)fix_quality;
      diagnostics->gps_sensor_1.satellites = satellites;
    }
    diagnostics->gps_sensor_1.hdop = gps_data->hdop;
    diagnostics->gps_sensor_1.vdop = gps_data->vdop;
  } else {
    diagnostics->gps_sensor_1.state =
        gps_poll_result == RESULT_ERR_UNIMPLEMENTED ||
                gps_poll_result == RESULT_ERR_COMMS
            ? SensorState_SENSOR_IDLE
            : SensorState_SENSOR_ERROR;
    diagnostics->gps_sensor_1.error_code = gps_valid
                                               ? GPSErrorCode_GPS_NO_ERROR
                                               : GPSErrorCode_GPS_INVALID_DATA;
  }

  imu_poll_result = poll_imu_sensor(imu_data);
  if (imu_poll_result == RESULT_OK) {
    diagnostics->imu_sensor.accel_x = imu_data->accel[0];
    diagnostics->imu_sensor.accel_y = imu_data->accel[1];
    diagnostics->imu_sensor.accel_z = imu_data->accel[2];
    diagnostics->imu_sensor.gyro_x = imu_data->gyro[0];
    diagnostics->imu_sensor.gyro_y = imu_data->gyro[1];
    diagnostics->imu_sensor.gyro_z = imu_data->gyro[2];
    diagnostics->imu_sensor.mag_x = imu_data->mag[0];
    diagnostics->imu_sensor.mag_y = imu_data->mag[1];
    diagnostics->imu_sensor.mag_z = imu_data->mag[2];
    diagnostics->imu_sensor.state = SensorState_SENSOR_OPERATING;
    diagnostics->imu_sensor.error_code = IMUErrorCode_IMU_NO_ERROR;

    if (!imu_validate_accelerometer_range(imu_data)) {
      diagnostics->imu_sensor.error_code = IMUErrorCode_IMU_ACCELEROMETER_ERROR;
    } else if (!imu_validate_gyroscope_range(imu_data)) {
      diagnostics->imu_sensor.error_code = IMUErrorCode_IMU_GYROSCOPE_ERROR;
    } else if (!imu_validate_magnetometer_range(imu_data)) {
      diagnostics->imu_sensor.error_code = IMUErrorCode_IMU_MAGNETOMETER_ERROR;
    }
  } else {
    diagnostics->imu_sensor.state =
        imu_poll_result == RESULT_ERR_UNIMPLEMENTED ||
                imu_poll_result == RESULT_ERR_COMMS
            ? SensorState_SENSOR_IDLE
            : SensorState_SENSOR_ERROR;
    diagnostics->imu_sensor.error_code = IMUErrorCode_IMU_COMMUNICATION_FAILURE;
  }
}

static void SensorBoard_SendDiagnostics(
  const SensorDiagnostics *diagnostics) {
  uint8_t encoded_payload[SENSOR_BOARD_PROTOBUF_BUFFER_SIZE];
  pb_ostream_t stream;

  stream = pb_ostream_from_buffer(encoded_payload, sizeof(encoded_payload));
  if (pb_encode(&stream, SensorDiagnostics_fields, diagnostics)) {
    ETH_udp_send_binary(laptop_ip, SENSOR_BOARD_UDP_PORT, encoded_payload,
                        stream.bytes_written);
    return;
  }

  // Keep a compact fallback payload available without heap allocation.
  SensorPacket fallback_packet = {
      .timestamp = HAL_GetTick(),
      .temperature = 0.0f,
      .humidity = 0.0f,
      .pressure = 0.0f,
  };

  LOGW(TAG, "Protobuf encode failed (%s), sending compact fallback packet",
       PB_GET_ERROR(&stream));
  ETH_udp_send_binary(laptop_ip, SENSOR_BOARD_UDP_PORT, &fallback_packet,
                      sizeof(fallback_packet));
}

/**
 * @brief  Main application task - reads sensors and handles networking
 * @param  argument: Not used currently
 * @retval None
 */
void MainTask(void *argument) {
  gps_data_t gps_data;
  imu_data_t imu_data;
  ph_sensor_t ph_sensor;

  (void)argument;

  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  LOGI(TAG, "Sensor board taking off...");

  // Let the scheduler settle before bringing up the network stack.
  osDelay(SENSOR_BOARD_NETWORK_STARTUP_DELAY_MS);

  LOGI(TAG, "Initializing Ethernet...");
  ETH_init(NULL);
  SensorBoard_ApplyStaticNetwork();
  LOGI(TAG, "Initializing UDP...");
  ETH_udp_init(NULL);
  SensorBoard_RebindUdpClient();
  SensorBoard_ConfigureDestinationRoute();
  SensorBoard_InitSensors(&imu_data, &ph_sensor, &gps_data);

  BSP_LED_On(LED_GREEN);
  LOGI(TAG, "Starting sensor telemetry loop (%lu ms period)...",
       sensor_board_send_interval_ms);

  while (1) {
    SensorDiagnostics diagnostics;

    SensorBoard_UpdateDiagnostics(&diagnostics, &imu_data, &ph_sensor, &gps_data);
    SensorBoard_SendDiagnostics(&diagnostics);

    BSP_LED_Toggle(LED_BLUE);
    osDelay(sensor_board_send_interval_ms);
  }
}

#endif //! PIO_UNIT_TESTING
