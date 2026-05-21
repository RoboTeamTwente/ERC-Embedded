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
#include "gps_sensor.h"
#include "imu_sensor.h"
#include "ph_sensor.h"
#include "sensor_basics.h"

/* ---- New drivers -------------------------------------------------------- */
#include "components/sensor_board/sampling/flow_sensor/flow_sensor.h"
#include "components/sensor_board/sampling/pump/pump.h"

// Protobuf includes
#include "components/sensor_board/diagnostics.pb.h"
#include "components/sensor_board/gps_sensor.pb.h"
#include "components/sensor_board/imu_sensor.pb.h"
#include "components/sensor_board/load_cell.pb.h"
#include "components/sensor_board/ph_sensor.pb.h"
#include "components/sensor_board/pressure_sensor.pb.h"
#include "components/sensor_board/pump.pb.h"
#include "components/sensor_board/sensor.pb.h"

// Packet dispatcher includes
#include "components/common/packet_dispatcher/packet_dispatcher.h"
#include "components/common/packet_dispatcher/packet_dispatcher_macros.h"

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
 * Sensor Status Helper Functions
 * ============================================================================
 */

/**
 * @brief Handle common sensor poll result and update sensor state.
 *        Sets state based on the result and logs appropriately.
 *        Error code handling is left to caller since they're different enum types.
 *
 * @param[out] state       Pointer to SensorState field to update
 * @param[in]  sensor_name Name of sensor for logging
 * @param[in]  poll_result Result from poll_*_sensor()
 * @return true if sensor is ready and has valid data, false otherwise
 */
static inline bool handle_sensor_poll_result(SensorState *state,
                                              const char *sensor_name,
                                              result_t poll_result) {
  if (poll_result == RESULT_ERR_UNIMPLEMENTED ||
      poll_result == RESULT_ERR_COMMS) {
    LOGW(TAG, "%s - Not connected (%s)", sensor_name,
         result_to_short_str(poll_result));
    *state = SensorState_SENSOR_IDLE;
    return false;
  } else if (poll_result != RESULT_OK) {
    LOGE(TAG, "%s - Poll error: %s (%s)", sensor_name,
         result_to_short_str(poll_result), result_to_desc_str(poll_result));
    *state = SensorState_SENSOR_ERROR;
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

static result_t init_gps_wrapper(gps_data_t *gps) {
  return gps_sensor_init(gps);
}

static result_t init_load_cells_wrapper(load_cell_data_t *load_cells) {
  for (size_t i = 0; i < 2; i++) {
    result_t result = load_cell_sensor_init(&load_cells[i]);
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

/* ============================================================================
 * Global pump handle
 * ============================================================================
 */
pump_data_t g_pump_data;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef
    htim3; /* PWM timer for pump — adjust to your CubeMX config */
extern COM_InitTypeDef BspCOMInit;
extern UART_HandleTypeDef huart_com;

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

  /* Initialize logging system */
  LOG_init(&huart_com);
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

  gps_data_t gps_data;
  LOGI(TAG, "Initializing GPS...");
  if (init_gps_wrapper(&gps_data) != RESULT_OK) {
    LOGW(TAG, "GPS may not be available, continuing...");
  } else {
    LOGI(TAG, "GPS init completed");
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
  /*
   * g_flow_data is made static so the EXTI ISR callback (in stm32h7xx_it.c)
   * can reach it via:  extern flow_sensor_data_t g_flow_data;
   */
  static flow_sensor_data_t g_flow_data;
  LOGI(TAG, "Initializing Flow Sensor...");
  if (init_flow_sensor_wrapper(&g_flow_data) != RESULT_OK) {
    LOGW(TAG, "Flow sensor may not be available, continuing...");
  } else {
    LOGI(TAG, "Flow sensor initialized");
  }

  /* ---- Pump init --------------------------------------------------------- */
  pump_hw_t pump_hw = {
      .htim = &htim3,
      .tim_channel = TIM_CHANNEL_3,
      .tim_period = htim3.Init.Period,
      .dir_port = GPIOB,
      .dir_pin = GPIO_PIN_0,
      .en_port = GPIOB,
      .en_pin = GPIO_PIN_14,
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

  BSP_LED_Toggle(LED_GREEN);

  /* ---- Main loop --------------------------------------------------------- */
  uint32_t loop_count = 0;
  LOGD(TAG, "========== ENTERING MAIN LOOP ==========");
  LOGD(TAG, "Starting main sensor loop...");
  const bool skip_sensor_polling = true; // need to make false to poll data

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
      diagnostics.has_gps_sensor_1 = false;
      diagnostics.has_imu_sensor = false;
    } else {

      /* ---------- pH sensor -------------------------------------------------
       */
      float ph_value, ph_voltage;
      result_t ph_poll_result = poll_ph_sensor(&ph_sensor);
      diagnostics.has_ph_sensor = true;

      if (!handle_sensor_poll_result(&diagnostics.ph_sensor.state,
                                      "pH Sensor", ph_poll_result)) {
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
        diagnostics.ph_sensor.error_code = PHErrorCode_PH_COMMUNICATION_FAILURE;
        diagnostics.ph_sensor.ph_value = 0.0f;
        diagnostics.ph_sensor.voltage = 0.0f;
      }

      /* ---------- GPS -------------------------------------------------------
       */
      result_t gps_poll_result = poll_gps_sensor(&gps_data);
      diagnostics.has_gps_sensor_1 = true;

      if (!handle_sensor_poll_result(&diagnostics.gps_sensor_1.state,
                                      "GPS", gps_poll_result)) {
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
      } else {
        bool gps_valid = false;
        if (gps_sensor_is_valid(&gps_data, &gps_valid) == RESULT_OK &&
            gps_valid) {
          diagnostics.gps_sensor_1.state = SensorState_SENSOR_OPERATING;
          diagnostics.gps_sensor_1.error_code = GPSErrorCode_GPS_NO_ERROR;

          double lat, lon;
          float altitude, speed, heading;
          gps_fix_quality_t fix_quality;
          int32_t satellites;

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
      }

      /* ---------- IMU -------------------------------------------------------
       */
      result_t imu_poll_result = poll_imu_sensor(&imu_data);
      diagnostics.has_imu_sensor = true;

      if (!handle_sensor_poll_result(&diagnostics.imu_sensor.state,
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

      /* ---------- Load cells + pressure (index loop) -----------------------
       */
      for (size_t i = 0; i < 2; i++) {
        /* Load cell */
        SensorBoardLoadCellInfo load_cell_info =
            SensorBoardLoadCellInfo_init_zero;
        result_t lc_result = poll_load_cell_sensor(&load_cell_data[i]);
        load_cell_info.sensor_index = (uint32_t)i;

        if (!handle_sensor_poll_result(&load_cell_info.state,
                                        "Load cell", lc_result)) {
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
            } else {
              LOGW(TAG, "Load cell %lu - Invalid data", (unsigned long)i);
              load_cell_info.state = SensorState_SENSOR_ERROR;
              load_cell_info.error_code = LoadCellErrorCode_LOAD_CELL_INVALID_DATA;
            }
            load_cell_info.force_newtons = lc_force;
            load_cell_info.mass_grams = lc_mass;
            load_cell_info.raw_counts = lc_counts;
            load_cell_info.scale_newtons_per_count = lc_scale;
            load_cell_info.tare_offset_counts = lc_tare;
          } else {
            LOGE(TAG, "Load cell %lu - Failed to read values", (unsigned long)i);
            load_cell_info.state = SensorState_SENSOR_ERROR;
            load_cell_info.error_code = LoadCellErrorCode_LOAD_CELL_COMMUNICATION_FAILURE;
            load_cell_info.force_newtons = 0.0f;
            load_cell_info.mass_grams = 0.0f;
          }
        }
        load_cell_info.is_calibrated = load_cell_data[i].is_calibrated;

        /* Pressure sensor */
        SensorBoardPressureInfo pressure_info =
            SensorBoardPressureInfo_init_zero;
        result_t pr_result = poll_pressure_sensor(&pressure_data[i]);
        pressure_info.sensor_index = (uint32_t)i;

        if (!handle_sensor_poll_result(&pressure_info.state,
                                        "Pressure", pr_result)) {
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
            } else {
              LOGW(TAG, "Pressure %lu - Invalid data", (unsigned long)i);
              pressure_info.state = SensorState_SENSOR_ERROR;
              pressure_info.error_code = PressureErrorCode_PRESSURE_INVALID_DATA;
            }
            pressure_info.pressure_kpa = pr_kpa;
            pressure_info.temperature_c = pr_temp;
            pressure_info.voltage = pr_voltage;
          } else {
            LOGE(TAG, "Pressure %lu - Failed to read values", (unsigned long)i);
            pressure_info.state = SensorState_SENSOR_ERROR;
            pressure_info.error_code = PressureErrorCode_PRESSURE_COMMUNICATION_FAILURE;
            pressure_info.pressure_kpa = 0.0f;
            pressure_info.temperature_c = 0.0f;
            pressure_info.voltage = 0.0f;
          }
        }
        pressure_info.is_calibrated = pressure_data[i].is_calibrated;
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

      if (!handle_sensor_poll_result(&flow_info.state,
                                      "Flow Sensor", flow_poll_result)) {
        flow_info.status = SensorStatus_STATUS_DISCONNECTED;
        flow_info.flow_rate_ml_min_x100 = 0U;
        flow_info.total_volume_ml = 0U;
        flow_info.pulse_count = 0U;
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
          LOGD(TAG, "Flow: %lu.%02lu ml/min, total: %lu ml, pulses: %lu",
               (unsigned long)(rate / 100U), (unsigned long)(rate % 100U),
               (unsigned long)vol_ml, (unsigned long)pulses);
        } else {
          /* First window not yet complete */
          flow_info.state = SensorState_SENSOR_IDLE;
          flow_info.status = SensorStatus_STATUS_INITIALIZING;
        }
      }
    }

    /* ==========================================================================
     * Pump — build proto from current state
     * ==========================================================================
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
    }


    LOGD(TAG, "Loop %lu complete - Heap: %lu bytes, LED toggle done", loop_count,
         free_heap);

    osDelay(MAIN_TASK_DELAY_MS);
  }
}

#endif //! PIO_UNIT_TESTING
