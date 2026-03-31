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
#include "networking_constants.h"
#include "tim.h"
#include "string.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>
#include <stdlib.h>

#include "imu_sensor.h"
#include "ph_sensor.h"
#include "gps_sensor.h"
#include "sensor_basics.h"

// Protobuf includes
#include "pb_message.h"
#include "components/sensor_board/diagnostics.pb.h"
#include "components/sensor_board/ph_sensor.pb.h"
#include "components/sensor_board/imu_sensor.pb.h"
#include "components/sensor_board/gps_sensor.pb.h"
#include "components/sensor_board/sensor.pb.h"

#define TAG "MAIN"
#define MAIN_TASK_DELAY_MS 5000
#define SENSOR_UDP_DEST_PORT 7

extern void MX_FREERTOS_Init(void);
extern void SystemClock_Config(void);
extern void MPU_Config_wrapper(void);
void Error_Handler(void);

void MainTask(void *argument);

extern TIM_HandleTypeDef htim1;
COM_InitTypeDef BspCOMInit;
UART_HandleTypeDef huart_com;

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

  // Initialize Ethernet with MAC address filtering
  ETH_init(NULL);
  int mac1[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
  int mac2[6] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x67};
  int mac3[6] = {0x13, 0x24, 0x35, 0x46, 0x57, 0x68};
  ETH_setup_MAC_address_filtering(mac1, mac2, mac3);

  osThreadNew(MainTask, NULL, &mainTask_attributes);
  osKernelStart();

  while (1) {
  }
}

int main(void) { 
  init_board(); 
}

/**
 * @brief  Main application task - reads sensors and handles networking
 * @param  argument: Not used currently
 * @retval None
 */
void MainTask(void *argument) {
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  LOGI(TAG, "Sensor board taking off...");

  // Initialize sensors
  imu_data_t imu_data;
  LOGI(TAG, "Initializing IMU...");
  imu_sensor_init(&imu_data);

  ph_sensor_t ph_sensor;
  LOGI(TAG, "Initializing pH Sensor...");
  ph_sensor_init(&ph_sensor, 3.3f); // 3.3V reference voltage

  gps_data_t gps_data;
  LOGI(TAG, "Initializing GPS...");
  gps_sensor_init(&gps_data);

  BSP_LED_Toggle(LED_GREEN);

  uint32_t loop_count = 0;
  LOGI(TAG, "Starting main sensor loop...");

  // UDP destination (update when network params are finalized)
  uint8_t dest_ip[4] = {192, 168, 0, 100};

  static StaticQueue_t txStruct0;
  static uint8_t txStorage0[ETHERNET_SQ_LENGTH * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t tx0 = xQueueCreateStatic(ETHERNET_SQ_LENGTH,
                                         ETHERNET_SQ_ITEM_SIZE,
                                         txStorage0, &txStruct0);

  static StaticQueue_t txStruct1;
  static uint8_t txStorage1[ETHERNET_SQ_LENGTH * ETHERNET_SQ_ITEM_SIZE];
  QueueHandle_t tx1 = xQueueCreateStatic(ETHERNET_SQ_LENGTH,
                                         ETHERNET_SQ_ITEM_SIZE,
                                         txStorage1, &txStruct1);

  QueueHandle_t send_queues[ETHERNET_SQ_PRIORITY_BUFFERS] = {tx0, tx1};
  result_t udp_init = ETH_udp_init(ETHERNET_SQ_PRIORITY_BUFFERS, send_queues, NULL);
  if (udp_init != RESULT_OK) {
    LOGE(TAG, "UDP init failed: %s", result_to_short_str(udp_init));
  }

  while (1) {
    loop_count++;
    LOGI(TAG, "Loop iteration: %lu", loop_count);
    BSP_LED_Toggle(LED_GREEN);
    BSP_LED_Toggle(LED_BLUE);
    BSP_LED_Toggle(LED_RED);

    // Initialize sensor diagnostics message
    SensorBoardDiagnostics diagnostics = SensorBoardDiagnostics_init_zero;
    diagnostics.state = SensorBoardDiagnostics_State_OPERATING;

    LOGI(TAG, "========== Sensor Board Reading ==========");

    // Read and log pH sensor
    float ph_value, ph_voltage;
    result_t ph_poll_result = poll_ph_sensor(&ph_sensor);

    diagnostics.has_ph_sensor = true;
    if (ph_poll_result == RESULT_ERR_UNIMPLEMENTED || ph_poll_result == RESULT_ERR_COMMS) {
      // Sensor not connected
      LOGW(TAG, "pH Sensor - Not connected (result: %d)", ph_poll_result);
      diagnostics.ph_sensor.state = SensorState_SENSOR_IDLE;
      diagnostics.ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
      diagnostics.ph_sensor.ph_value = 0.0f;
      diagnostics.ph_sensor.voltage = 0.0f;
    } else if (ph_poll_result != RESULT_OK) {
      // Other error during polling
      LOGE(TAG, "pH Sensor - Poll error: %d", ph_poll_result);
      diagnostics.ph_sensor.state = SensorState_SENSOR_ERROR;
      diagnostics.ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
      diagnostics.ph_sensor.ph_value = 0.0f;
      diagnostics.ph_sensor.voltage = 0.0f;
    } else {
      // Poll succeeded, so now getting the actual sensor values
      if (ph_sensor_get_value(&ph_sensor, &ph_value) == RESULT_OK) {
        ph_sensor_get_voltage(&ph_sensor, &ph_voltage);
        if (validate_ph_value(ph_value) == RESULT_OK) {
          LOGI(TAG, "pH Sensor - Value: %.2f, Voltage: %.3fV", ph_value, ph_voltage);
          diagnostics.ph_sensor.ph_value = ph_value;
          diagnostics.ph_sensor.voltage = ph_voltage;
          diagnostics.ph_sensor.state = SensorState_SENSOR_OPERATING;
          diagnostics.ph_sensor.error_code = PHErrorCode_PH_NO_ERROR;
        } else {
          LOGI(TAG, "pH Sensor - Invalid value: %.2f", ph_value);
          diagnostics.ph_sensor.ph_value = ph_value;
          diagnostics.ph_sensor.voltage = ph_voltage;
          diagnostics.ph_sensor.state = SensorState_SENSOR_ERROR;
          diagnostics.ph_sensor.error_code = PHErrorCode_PH_INVALID_DATA;
        }
      } else {
        LOGE(TAG, "pH Sensor - Failed to read value");
        diagnostics.ph_sensor.state = SensorState_SENSOR_ERROR;
        diagnostics.ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
        diagnostics.ph_sensor.ph_value = 0.0f;
        diagnostics.ph_sensor.voltage = 0.0f;
      }
    }

    // Read and log GPS data
    result_t gps_poll_result = poll_gps_sensor(&gps_data);
    double lat, lon;
    float altitude, speed, heading;
    gps_fix_quality_t fix_quality;
    int32_t satellites;
    bool gps_valid;

    diagnostics.has_gps_sensor_1 = true;
    if (gps_poll_result == RESULT_ERR_UNIMPLEMENTED || gps_poll_result == RESULT_ERR_COMMS) {
      // Sensor not connected or hardware not available
      LOGW(TAG, "GPS - Not connected (result: %d)", gps_poll_result);
      diagnostics.gps_sensor_1.state = SensorState_SENSOR_IDLE;
      diagnostics.gps_sensor_1.error_code = GPSErrorCode_GPS_COMMUNICATION_FAILURE;
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
      // Other error during polling
      LOGE(TAG, "GPS - Poll error: %d", gps_poll_result);
      diagnostics.gps_sensor_1.state = SensorState_SENSOR_ERROR;
      diagnostics.gps_sensor_1.error_code = GPSErrorCode_GPS_COMMUNICATION_FAILURE;
    } else if (gps_sensor_is_valid(&gps_data, &gps_valid) == RESULT_OK && gps_valid) {
      diagnostics.gps_sensor_1.state = SensorState_SENSOR_OPERATING;
      diagnostics.gps_sensor_1.error_code = GPSErrorCode_GPS_NO_ERROR;

      if (gps_sensor_get_coordinates(&gps_data, &lat, &lon) == RESULT_OK) {
        LOGI(TAG, "GPS - Lat: %.6f°, Lon: %.6f°", lat, lon);
        diagnostics.gps_sensor_1.latitude = lat;
        diagnostics.gps_sensor_1.longitude = lon;
      }
      if (gps_sensor_get_altitude(&gps_data, &altitude) == RESULT_OK) {
        LOGI(TAG, "GPS - Altitude: %.2f m", altitude);
        diagnostics.gps_sensor_1.altitude = altitude;
      }
      if (gps_sensor_get_velocity(&gps_data, &speed, &heading) == RESULT_OK) {
        LOGI(TAG, "GPS - Speed: %.2f m/s, Heading: %.1f°", speed, heading);
        diagnostics.gps_sensor_1.speed = speed;
        diagnostics.gps_sensor_1.heading = heading;
      }
      if (gps_sensor_get_fix_info(&gps_data, &fix_quality, &satellites) == RESULT_OK) {
        LOGI(TAG, "GPS - Fix Quality: %d, Satellites: %ld", fix_quality, satellites);
        diagnostics.gps_sensor_1.fix_quality = (GPSFixQuality)fix_quality;
        diagnostics.gps_sensor_1.satellites = satellites;
      }
      LOGI(TAG, "GPS - HDOP: %.2f, VDOP: %.2f", gps_data.hdop, gps_data.vdop);
      diagnostics.gps_sensor_1.hdop = gps_data.hdop;
      diagnostics.gps_sensor_1.vdop = gps_data.vdop;
    } else {
      LOGW(TAG, "GPS - No valid data received");
      diagnostics.gps_sensor_1.state = SensorState_SENSOR_ERROR;
      diagnostics.gps_sensor_1.error_code = GPSErrorCode_GPS_INVALID_DATA;
    }

    // Read and log IMU data
    result_t imu_poll_result = poll_imu_sensor(&imu_data);

    diagnostics.has_imu_sensor = true;
    if (imu_poll_result == RESULT_ERR_UNIMPLEMENTED || imu_poll_result == RESULT_ERR_COMMS) {
      // Sensor not connected or hardware not available
      LOGW(TAG, "IMU - Not connected (result: %d)", imu_poll_result);
      diagnostics.imu_sensor.state = SensorState_SENSOR_IDLE;
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
    } else if (imu_poll_result != RESULT_OK) {
      // Other error during polling
      LOGE(TAG, "IMU - Poll error: %d", imu_poll_result);
      diagnostics.imu_sensor.state = SensorState_SENSOR_ERROR;
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
      // Sensor is operating normally
      LOGI(TAG, "IMU - Accel (m/s²): X=%.2f, Y=%.2f, Z=%.2f", 
           imu_data.accel[0], imu_data.accel[1], imu_data.accel[2]);
      LOGI(TAG, "IMU - Gyro (°/s): X=%.2f, Y=%.2f, Z=%.2f", 
           imu_data.gyro[0], imu_data.gyro[1], imu_data.gyro[2]);
      LOGI(TAG, "IMU - Mag (µT): X=%.2f, Y=%.2f, Z=%.2f", 
           imu_data.mag[0], imu_data.mag[1], imu_data.mag[2]);

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
      LOGW(TAG, "IMU - Accelerometer is out of range");
      diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_ACCELEROMETER_ERROR;
    }
    if (!imu_validate_gyroscope_range(&imu_data)) {
      LOGW(TAG, "IMU - Gyroscope is out of range");
      diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_GYROSCOPE_ERROR;
    }
    if (!imu_validate_magnetometer_range(&imu_data)) {
      LOGW(TAG, "IMU - Magnetometer is out of range");
      diagnostics.imu_sensor.error_code = IMUErrorCode_IMU_MAGNETOMETER_ERROR;
    }

    LOGI(TAG, "==========================================");

    // Encode protobuf message
    uint8_t *encoded_data = NULL;
    size_t encoded_size = 0;
    result_t encode_result = pb_message_encode(&diagnostics, SensorBoardDiagnostics_fields, 
                                                &encoded_data, &encoded_size);

        if (encode_result == RESULT_OK) {
          // Send encoded protobuf message via UDP
          result_t send_result = ETH_udp_send_binary(dest_ip, SENSOR_UDP_DEST_PORT,
                                                     encoded_data, encoded_size, 1);
          if (send_result == RESULT_OK) {
            LOGI(TAG, "Sensor diagnostics sent (%d bytes) to %u.%u.%u.%u:%u", 
                 encoded_size, dest_ip[0], dest_ip[1], dest_ip[2], dest_ip[3],
                 SENSOR_UDP_DEST_PORT);
          } else {
            LOGE(TAG, "UDP send failed: %s", result_to_short_str(send_result));
          }
          free(encoded_data);
        } else {
          LOGE(TAG, "Failed to encode sensor diagnostics: %d", encode_result);
        }

    LOGI(TAG, "Delaying for 5 seconds...");
    LOGI(TAG, " ");
    osDelay(MAIN_TASK_DELAY_MS);
  }
}

#endif //! PIO_UNIT_TESTING