#ifndef LOAD_CELL_SENSOR_H
#define LOAD_CELL_SENSOR_H

/**
 * @file load_cell_sensor.h
 * @brief Load-cell driver via HX711 24-bit ADC (GPIO bit-bang).
 *
 * Wiring (one HX711 per load cell):
 *   HX711 DOUT (data, low = ready) -> STM32 GPIO input
 *   HX711 SCK  (clock)             -> STM32 GPIO output
 *   HX711 VCC 2.7–5.5 V, GND common with STM32.
 *
 * Sensor board mapping (see firmware.ioc):
 *   Unit 0: DOUT = PA5 (WEIGHT_INPUT_1), SCK = PC7 (WEIGHT_CLOCK_1)
 *   Unit 1: DOUT = PA6 (WEIGHT_INPUT_2), SCK = PB5 (WEIGHT_CLOCK_2)
 *
 * Reading is 24-bit two's-complement. force/mass are only meaningful after
 * calibration (load_cell_tare + load_cell_set_scale). Until then raw_counts is
 * valid but force_newtons/mass_grams use the default passthrough scale.
 */

#include "result.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/** Default Newtons-per-count before calibration (passthrough = raw is trusted,
 *  force/mass are not). Override with load_cell_set_scale(). */
#ifndef LOAD_CELL_DEFAULT_N_PER_COUNT
#define LOAD_CELL_DEFAULT_N_PER_COUNT 1.0f
#endif

/** HX711 extra SCK pulses after the 24 data bits select gain/channel for the
 *  NEXT conversion: 1 = ch A gain 128, 2 = ch B gain 32, 3 = ch A gain 64. */
#ifndef LOAD_CELL_GAIN_PULSES
#define LOAD_CELL_GAIN_PULSES 1U
#endif

/** Max wait for the HX711 to signal "data ready" (DOUT low), milliseconds. */
#ifndef LOAD_CELL_READY_TIMEOUT_MS
#define LOAD_CELL_READY_TIMEOUT_MS 200U
#endif

typedef struct {
  int32_t raw_counts;
  float force_newtons;
  float mass_grams;
  float scale_newtons_per_count;
  int32_t tare_offset_counts;
  bool is_calibrated; /**< true once load_cell_set_scale() called   */
  bool read_ok;       /**< true if the last poll read succeeded      */

  /* ---- HX711 hardware binding (set by load_cell_sensor_init_hw) ---------- */
  GPIO_TypeDef *dout_port;
  uint16_t dout_pin;
  GPIO_TypeDef *sck_port;
  uint16_t sck_pin;
  uint8_t gain_pulses;
} load_cell_data_t;

/**
 * @brief Zero-initialise without a hardware binding. poll() returns
 *        RESULT_ERR_UNIMPLEMENTED until load_cell_sensor_init_hw() is used.
 */
result_t load_cell_sensor_init(load_cell_data_t *data);

/**
 * @brief Initialise an HX711-backed load cell on the given GPIOs, power it up,
 *        and auto-tare. force/mass use the default scale until calibrated.
 */
result_t load_cell_sensor_init_hw(load_cell_data_t *data,
                                  GPIO_TypeDef *dout_port, uint16_t dout_pin,
                                  GPIO_TypeDef *sck_port, uint16_t sck_pin);

/** @brief Average `samples` raw reads and store as the tare (zero) offset. */
result_t load_cell_tare(load_cell_data_t *data, uint8_t samples);

/** @brief Set the Newtons-per-count scale and mark the cell calibrated. */
result_t load_cell_set_scale(load_cell_data_t *data, float newtons_per_count);

result_t poll_load_cell_sensor(load_cell_data_t *data);

result_t load_cell_get_force_newtons(const load_cell_data_t *data,
                                     float *force_newtons);
result_t load_cell_get_mass_grams(const load_cell_data_t *data,
                                  float *mass_grams);
result_t load_cell_get_raw_counts(const load_cell_data_t *data,
                                  int32_t *raw_counts);
result_t load_cell_get_calibration(const load_cell_data_t *data,
                                   float *scale_newtons_per_count,
                                   int32_t *tare_offset_counts);
result_t load_cell_sensor_is_valid(const load_cell_data_t *data, bool *is_valid);

#endif
