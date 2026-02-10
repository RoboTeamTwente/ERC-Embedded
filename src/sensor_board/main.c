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
#include "tim.h"
#include "string.h"
#include <stdint.h>

#include "imu_sensor.h"
#include "ph_sensor.h"
#include "gps_sensor.h"
#include "sensor_basics.h"

#define TAG "MAIN"
#define MAIN_TASK_DELAY_MS 5000

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
  ETH_init(NULL, NULL);
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

  // Initialize networking
  ETH_udp_init();

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

  while (1) {
    loop_count++;
    LOGI(TAG, "Loop iteration: %lu", loop_count);
    BSP_LED_Toggle(LED_GREEN);
    BSP_LED_Toggle(LED_BLUE);
    BSP_LED_Toggle(LED_RED);

    LOGI(TAG, "========== Sensor Board Reading ==========");

    // Read and log pH sensor
    float ph_value, ph_voltage;
    poll_ph_sensor(&ph_sensor);
    if (ph_sensor_get_value(&ph_sensor, &ph_value) == RESULT_OK) {
      ph_sensor_get_voltage(&ph_sensor, &ph_voltage);
      if (validate_ph_value(ph_value) == RESULT_OK) {
        LOGI(TAG, "pH Sensor - Value: %.2f, Voltage: %.3fV", ph_value, ph_voltage);
      } else {
        LOGW(TAG, "pH Sensor - Invalid value: %.2f", ph_value);
      }
    } else {
      LOGE(TAG, "pH Sensor - Failed to read");
    }

    // Read and log GPS data
    poll_gps_sensor(&gps_data);
    double lat, lon;
    float altitude, speed, heading;
    gps_fix_quality_t fix_quality;
    int32_t satellites;
    bool gps_valid;
    
    if (gps_sensor_is_valid(&gps_data, &gps_valid) == RESULT_OK && gps_valid) {
      if (gps_sensor_get_coordinates(&gps_data, &lat, &lon) == RESULT_OK) {
        LOGI(TAG, "GPS - Lat: %.6f°, Lon: %.6f°", lat, lon);
      }
      if (gps_sensor_get_altitude(&gps_data, &altitude) == RESULT_OK) {
        LOGI(TAG, "GPS - Altitude: %.2f m", altitude);
      }
      if (gps_sensor_get_velocity(&gps_data, &speed, &heading) == RESULT_OK) {
        LOGI(TAG, "GPS - Speed: %.2f m/s, Heading: %.1f°", speed, heading);
      }
      if (gps_sensor_get_fix_info(&gps_data, &fix_quality, &satellites) == RESULT_OK) {
        LOGI(TAG, "GPS - Fix Quality: %d, Satellites: %ld", fix_quality, satellites);
      }
      LOGI(TAG, "GPS - HDOP: %.2f, VDOP: %.2f", gps_data.hdop, gps_data.vdop);
    } else {
      LOGW(TAG, "GPS - No valid values...cooked");
    }
    
    // Read and log IMU data
    poll_imu_sensor(&imu_data);
    LOGI(TAG, "IMU - Accel (m/s²): X=%.2f, Y=%.2f, Z=%.2f", 
         imu_data.accel[0], imu_data.accel[1], imu_data.accel[2]);
    LOGI(TAG, "IMU - Gyro (°/s): X=%.2f, Y=%.2f, Z=%.2f", 
         imu_data.gyro[0], imu_data.gyro[1], imu_data.gyro[2]);
    LOGI(TAG, "IMU - Mag (µT): X=%.2f, Y=%.2f, Z=%.2f", 
         imu_data.mag[0], imu_data.mag[1], imu_data.mag[2]);
    
    float pitch = imu_get_pitch(&imu_data);
    float roll = imu_get_roll(&imu_data);
    float accel_mag = imu_get_acceleration_magnitude(&imu_data);
    LOGI(TAG, "IMU - Pitch: %.2f°, Roll: %.2f°, Accel Mag: %.2f m/s²", 
         pitch, roll, accel_mag);
    
    if (!imu_validate_accelerometer_range(&imu_data)) {
      LOGW(TAG, "IMU - Accelerometer is out of range");
    }
    if (!imu_validate_gyroscope_range(&imu_data)) {
      LOGW(TAG, "IMU - Gyroscope is out of range");
    }
    if (!imu_validate_magnetometer_range(&imu_data)) {
      LOGW(TAG, "IMU - Magnetometer is out of range");
    }

    LOGI(TAG, "==========================================");
    LOGI(TAG, "Delaying for 5 seconds...");
    LOGI(TAG, " ");
    osDelay(MAIN_TASK_DELAY_MS);
  }
}

#endif //! PIO_UNIT_TESTING
