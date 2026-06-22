/**
 * @file pressure_sensor.c
 * @brief Analog force/pressure (FSR) sensor read via ADC.
 *
 * The ADC access is compile-gated by PRESSURE_USE_ADC (no ADC enabled in
 * CubeMX yet). When off, poll_pressure_sensor() returns
 * RESULT_ERR_UNIMPLEMENTED and the firmware still links. See header for the
 * CubeMX changes needed to enable it.
 */

#include "pressure_sensor.h"

#include <string.h>

#ifdef PRESSURE_USE_ADC
#include "stm32h7xx_hal.h"
#ifndef PRESSURE_ADC_TIMEOUT_MS
#define PRESSURE_ADC_TIMEOUT_MS 100U
#endif
#ifndef PRESSURE_ADC_SAMPLETIME
#define PRESSURE_ADC_SAMPLETIME ADC_SAMPLETIME_64CYCLES_5
#endif
#endif /* PRESSURE_USE_ADC */

result_t pressure_sensor_init(pressure_sensor_data_t *data) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  memset(data, 0, sizeof(*data));
  data->is_calibrated = false;
  data->read_ok = false;
  data->adc_max = 65535U; /* 16-bit default */
  data->reference_voltage = 3.3f;
  data->scale_kpa_per_volt = 1.0f; /* passthrough until calibrated */
  data->offset_kpa = 0.0f;
  return RESULT_OK;
}

result_t pressure_sensor_init_hw(pressure_sensor_data_t *data, void *adc_handle,
                                 uint32_t adc_channel, uint32_t adc_max,
                                 float reference_voltage) {
  if (data == NULL || adc_handle == NULL || adc_max == 0U) {
    return RESULT_ERR_INVALID_ARG;
  }
  TRY(pressure_sensor_init(data));
  data->adc_handle = adc_handle;
  data->adc_channel = adc_channel;
  data->adc_max = adc_max;
  data->reference_voltage = reference_voltage;
  return RESULT_OK;
}

result_t pressure_sensor_set_calibration(pressure_sensor_data_t *data,
                                         float scale_kpa_per_volt,
                                         float offset_kpa) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  data->scale_kpa_per_volt = scale_kpa_per_volt;
  data->offset_kpa = offset_kpa;
  data->is_calibrated = true;
  return RESULT_OK;
}

result_t poll_pressure_sensor(pressure_sensor_data_t *data) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

#ifdef PRESSURE_USE_ADC
  if (data->adc_handle == NULL) {
    return RESULT_ERR_UNIMPLEMENTED; /* no ADC binding */
  }
  ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)data->adc_handle;

  /* Select this sensor's channel (shared ADC may serve several channels). */
  ADC_ChannelConfTypeDef ch = {0};
  ch.Channel = data->adc_channel;
  ch.Rank = ADC_REGULAR_RANK_1;
  ch.SamplingTime = PRESSURE_ADC_SAMPLETIME;
  ch.SingleDiff = ADC_SINGLE_ENDED;
  ch.OffsetNumber = ADC_OFFSET_NONE;
  ch.Offset = 0;
  if (HAL_ADC_ConfigChannel(hadc, &ch) != HAL_OK) {
    data->read_ok = false;
    return RESULT_ERR_COMMS;
  }

  if (HAL_ADC_Start(hadc) != HAL_OK) {
    data->read_ok = false;
    return RESULT_ERR_COMMS;
  }
  if (HAL_ADC_PollForConversion(hadc, PRESSURE_ADC_TIMEOUT_MS) != HAL_OK) {
    HAL_ADC_Stop(hadc);
    data->read_ok = false;
    return RESULT_ERR_COMMS;
  }
  uint32_t raw = HAL_ADC_GetValue(hadc);
  HAL_ADC_Stop(hadc);

  data->voltage = (float)raw / (float)data->adc_max * data->reference_voltage;
  data->pressure_kpa =
      data->voltage * data->scale_kpa_per_volt + data->offset_kpa;
  data->temperature_c = 0.0f; /* FSR has no temperature channel */
  data->read_ok = true;
  return RESULT_OK;
#else
  (void)data;
  return RESULT_ERR_UNIMPLEMENTED; /* ADC not wired — see header notes */
#endif
}

result_t pressure_sensor_get_pressure_kpa(const pressure_sensor_data_t *data,
                                          float *pressure_kpa) {
  if (data == NULL || pressure_kpa == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *pressure_kpa = data->pressure_kpa;
  return RESULT_OK;
}

result_t pressure_sensor_get_temperature_c(const pressure_sensor_data_t *data,
                                           float *temperature_c) {
  if (data == NULL || temperature_c == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *temperature_c = data->temperature_c;
  return RESULT_OK;
}

result_t pressure_sensor_get_voltage(const pressure_sensor_data_t *data,
                                     float *voltage) {
  if (data == NULL || voltage == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *voltage = data->voltage;
  return RESULT_OK;
}

result_t pressure_sensor_is_valid(const pressure_sensor_data_t *data,
                                  bool *is_valid) {
  if (data == NULL || is_valid == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *is_valid = data->read_ok;
  return RESULT_OK;
}
