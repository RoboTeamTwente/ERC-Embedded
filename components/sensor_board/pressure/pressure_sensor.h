#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

/**
 * @file pressure_sensor.h
 * @brief Analog force/pressure (FSR) sensor read via ADC.
 *
 * The board labels these FORCE_ANALOG_DATA_1 (PD15) / FORCE_ANALOG_DATA_2
 * (PF3). The sensor is read on an ADC channel:
 *     voltage      = raw / adc_max * reference_voltage
 *     pressure_kpa = voltage * scale_kpa_per_volt + offset_kpa
 *
 * The ADC path is compile-gated by PRESSURE_USE_ADC because no ADC is enabled
 * in CubeMX yet; until then poll_pressure_sensor() returns
 * RESULT_ERR_UNIMPLEMENTED (firmware still links).
 *
 * To enable, in CubeMX:
 *   - Enable an ADC + the channel(s) for the force pins. NOTE: on the
 *     STM32H753, PD15 has NO ADC function and PF3 = ADC3_INP5 — move
 *     FORCE_ANALOG_DATA_1 to an ADC-capable pin.
 * Then build with -D PRESSURE_USE_ADC and bind each unit with
 * pressure_sensor_init_hw().
 */

#include "result.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float pressure_kpa;
  float temperature_c;
  float voltage;
  bool is_calibrated;
  bool read_ok; /**< true if the last poll read succeeded */

  /* ---- ADC binding (set by pressure_sensor_init_hw) --------------------- */
  void *adc_handle;          /**< ADC_HandleTypeDef* (void* keeps header HAL-free) */
  uint32_t adc_channel;      /**< ADC_CHANNEL_x */
  uint32_t adc_max;          /**< full-scale count (e.g. 65535 for 16-bit)   */
  float reference_voltage;   /**< ADC Vref+ in volts                          */
  float scale_kpa_per_volt;  /**< linear gain  (default 1.0)                  */
  float offset_kpa;          /**< linear offset (default 0.0)                 */
} pressure_sensor_data_t;

result_t pressure_sensor_init(pressure_sensor_data_t *data);

/**
 * @brief Bind an ADC handle + channel for this force sensor.
 * @param adc_handle  Pointer to the ADC_HandleTypeDef (cast to void*).
 */
result_t pressure_sensor_init_hw(pressure_sensor_data_t *data, void *adc_handle,
                                 uint32_t adc_channel, uint32_t adc_max,
                                 float reference_voltage);

/** @brief Set linear conversion (kPa = V*scale + offset) and mark calibrated. */
result_t pressure_sensor_set_calibration(pressure_sensor_data_t *data,
                                         float scale_kpa_per_volt,
                                         float offset_kpa);

result_t poll_pressure_sensor(pressure_sensor_data_t *data);

result_t pressure_sensor_get_pressure_kpa(const pressure_sensor_data_t *data,
                                          float *pressure_kpa);
result_t pressure_sensor_get_temperature_c(const pressure_sensor_data_t *data,
                                           float *temperature_c);
result_t pressure_sensor_get_voltage(const pressure_sensor_data_t *data,
                                     float *voltage);
result_t pressure_sensor_is_valid(const pressure_sensor_data_t *data,
                                  bool *is_valid);

#endif
