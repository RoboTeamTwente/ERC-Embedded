/**
 * @file load_cell_sensor.c
 * @brief HX711 load-cell driver (GPIO bit-bang) implementation.
 *
 * HX711 read protocol:
 *   - DOUT goes low when a conversion is ready.
 *   - Pulse SCK 24 times; read DOUT MSB-first after each rising edge -> 24-bit
 *     two's-complement sample.
 *   - 1–3 extra SCK pulses select gain/channel for the NEXT conversion.
 *   - SCK must not stay HIGH > 60 µs (else the chip powers down), so the bit
 *     loop runs with interrupts disabled and short DWT-based µs delays.
 */

#include "load_cell_sensor.h"

#include <string.h>

/* --------------------------------------------------------------------------
 * Microsecond busy-delay via the Cortex-M DWT cycle counter.
 * -------------------------------------------------------------------------- */
static void dwt_init(void) {
  /* TRCENA may already be set by an attached debugger/probe, so enable the
   * cycle counter unconditionally — otherwise CYCCNT never increments and
   * delay_us() spins forever (watchdog reset / reboot loop). */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void delay_us(uint32_t us) {
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (SystemCoreClock / 1000000U);
  while ((DWT->CYCCNT - start) < ticks) {
    /* busy wait */
  }
}

/* --------------------------------------------------------------------------
 * Low-level HX711 read. Returns sign-extended 24-bit sample.
 * -------------------------------------------------------------------------- */
static result_t hx711_read_raw(load_cell_data_t *data, int32_t *out) {
  /* Wait for DOUT low = data ready */
  uint32_t t0 = HAL_GetTick();
  while (HAL_GPIO_ReadPin(data->dout_port, data->dout_pin) == GPIO_PIN_SET) {
    if ((HAL_GetTick() - t0) > LOAD_CELL_READY_TIMEOUT_MS) {
      return RESULT_ERR_COMMS; /* not connected / not converting */
    }
  }

  uint32_t value = 0U;

  __disable_irq(); /* timing-critical: keep SCK high pulses < 60 µs */
  for (int i = 0; i < 24; i++) {
    HAL_GPIO_WritePin(data->sck_port, data->sck_pin, GPIO_PIN_SET);
    delay_us(1);
    value = (value << 1) |
            (HAL_GPIO_ReadPin(data->dout_port, data->dout_pin) == GPIO_PIN_SET
                 ? 1U
                 : 0U);
    HAL_GPIO_WritePin(data->sck_port, data->sck_pin, GPIO_PIN_RESET);
    delay_us(1);
  }
  /* Gain/channel selection pulses for the next conversion */
  for (uint8_t i = 0; i < data->gain_pulses; i++) {
    HAL_GPIO_WritePin(data->sck_port, data->sck_pin, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(data->sck_port, data->sck_pin, GPIO_PIN_RESET);
    delay_us(1);
  }
  __enable_irq();

  /* Sign-extend 24-bit two's complement to 32-bit */
  if (value & 0x800000U) {
    value |= 0xFF000000U;
  }
  *out = (int32_t)value;
  return RESULT_OK;
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

result_t load_cell_sensor_init(load_cell_data_t *data) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  memset(data, 0, sizeof(*data));
  data->scale_newtons_per_count = LOAD_CELL_DEFAULT_N_PER_COUNT;
  data->gain_pulses = LOAD_CELL_GAIN_PULSES;
  data->is_calibrated = false;
  data->read_ok = false;
  return RESULT_OK;
}

result_t load_cell_sensor_init_hw(load_cell_data_t *data,
                                  GPIO_TypeDef *dout_port, uint16_t dout_pin,
                                  GPIO_TypeDef *sck_port, uint16_t sck_pin) {
  if (data == NULL || dout_port == NULL || sck_port == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  TRY(load_cell_sensor_init(data));
  data->dout_port = dout_port;
  data->dout_pin = dout_pin;
  data->sck_port = sck_port;
  data->sck_pin = sck_pin;

  dwt_init();

  /* Pull-up on DOUT so an ABSENT HX711 floats HIGH (never "data ready") and is
   * reported DISCONNECTED instead of reading a floating line as valid data. A
   * present chip drives DOUT push-pull, overriding the weak pull-up. */
  GPIO_InitTypeDef dout_cfg = {0};
  dout_cfg.Pin = dout_pin;
  dout_cfg.Mode = GPIO_MODE_INPUT;
  dout_cfg.Pull = GPIO_PULLUP;
  dout_cfg.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(dout_port, &dout_cfg);

  /* Power-up: SCK low (a >60 µs high pulse powers the chip down). */
  HAL_GPIO_WritePin(sck_port, sck_pin, GPIO_PIN_RESET);

  /* Best-effort auto-tare; ignore failure (sensor may be absent at boot). */
  (void)load_cell_tare(data, 8);
  return RESULT_OK;
}

result_t load_cell_tare(load_cell_data_t *data, uint8_t samples) {
  if (data == NULL || data->sck_port == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (samples == 0U) {
    samples = 1U;
  }

  int64_t sum = 0;
  for (uint8_t i = 0; i < samples; i++) {
    int32_t raw = 0;
    TRY(hx711_read_raw(data, &raw));
    sum += raw;
  }
  data->tare_offset_counts = (int32_t)(sum / (int64_t)samples);
  return RESULT_OK;
}

result_t load_cell_set_scale(load_cell_data_t *data, float newtons_per_count) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  data->scale_newtons_per_count = newtons_per_count;
  data->is_calibrated = true;
  return RESULT_OK;
}

result_t poll_load_cell_sensor(load_cell_data_t *data) {
  if (data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (data->sck_port == NULL) {
    return RESULT_ERR_UNIMPLEMENTED; /* no hardware binding */
  }

  int32_t raw = 0;
  result_t r = hx711_read_raw(data, &raw);
  if (r != RESULT_OK) {
    data->read_ok = false;
    return r;
  }

  data->raw_counts = raw;
  float counts = (float)(raw - data->tare_offset_counts);
  data->force_newtons = counts * data->scale_newtons_per_count;
  data->mass_grams = data->force_newtons / 9.80665f * 1000.0f;
  data->read_ok = true;
  return RESULT_OK;
}

result_t load_cell_get_force_newtons(const load_cell_data_t *data,
                                     float *force_newtons) {
  if (data == NULL || force_newtons == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *force_newtons = data->force_newtons;
  return RESULT_OK;
}

result_t load_cell_get_mass_grams(const load_cell_data_t *data,
                                  float *mass_grams) {
  if (data == NULL || mass_grams == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *mass_grams = data->mass_grams;
  return RESULT_OK;
}

result_t load_cell_get_raw_counts(const load_cell_data_t *data,
                                  int32_t *raw_counts) {
  if (data == NULL || raw_counts == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *raw_counts = data->raw_counts;
  return RESULT_OK;
}

result_t load_cell_get_calibration(const load_cell_data_t *data,
                                   float *scale_newtons_per_count,
                                   int32_t *tare_offset_counts) {
  if (data == NULL || scale_newtons_per_count == NULL ||
      tare_offset_counts == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  *scale_newtons_per_count = data->scale_newtons_per_count;
  *tare_offset_counts = data->tare_offset_counts;
  return RESULT_OK;
}

result_t load_cell_sensor_is_valid(const load_cell_data_t *data,
                                   bool *is_valid) {
  if (data == NULL || is_valid == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  /* "valid" = last read succeeded. Calibration is reported separately via
   * is_calibrated, so an uncalibrated-but-reading cell still shows OPERATING. */
  *is_valid = data->read_ok;
  return RESULT_OK;
}
