/**
 * @file flow_sensor.c
 * @brief FM-PS2216 Water Flow Sensor Driver Implementation
 *
 * HOW PULSE COUNTING WORKS
 * ========================
 * The FM-PS2216 outputs an open-collector pulse train on its signal wire.
 * Each rising edge = 1/5.5 ml of fluid has passed through.
 *
 * Two hardware options (choose one, wire the HAL glue accordingly):
 *
 * Option A – EXTI (simplest):
 *   Configure the signal GPIO as EXTI rising-edge.
 *   In HAL_GPIO_EXTI_Callback(), call flow_sensor_pulse_isr(&flow_data).
 *
 * Option B – TIM Input Capture (more accurate at high flow):
 *   Configure TIMx CH1 in input-capture mode; enable the CC interrupt.
 *   In HAL_TIM_IC_CaptureCallback(), call flow_sensor_pulse_isr(&flow_data).
 *
 * poll_flow_sensor() is called from the FreeRTOS main task every
 * MAIN_TASK_DELAY_MS (5000 ms).  It snapshots the pulse counter accumulated
 * since the last call, computes flow rate, and accumulates total volume.
 *
 * FIXED-POINT ARITHMETIC
 * ======================
 * To stay integer-only (no floats, no strings):
 *   flow_rate_ml_min_x100 = (pulses_in_window × 100 × 1000)
 *                           / (FLOW_SENSOR_PULSES_PER_ML_X10 × elapsed_ms / 10)
 * Simplified:
 *   = (pulses × 100 × 10000) / (PULSES_PER_ML_X10 × elapsed_ms)
 *
 * Volume per window (µl):
 *   delta_ul = (pulses × 1000 × 10) / FLOW_SENSOR_PULSES_PER_ML_X10
 *            = (pulses × 10000)      / PULSES_PER_ML_X10
 */

#include "flow_sensor.h"

#include <string.h>
#include "stm32h7xx_hal.h"   /* HAL_GetTick(), __disable_irq/__enable_irq */

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/**
 * @brief Atomically snapshot and reset the per-window pulse counter.
 *        Uses a brief critical section so the ISR cannot fire mid-read.
 */
static uint32_t snapshot_and_reset_pulses(flow_sensor_data_t *data) {
    uint32_t count;
    __disable_irq();
    count = data->pulse_count_window;
    data->pulse_count_window = 0U;
    __enable_irq();
    return count;
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

result_t flow_sensor_init(flow_sensor_data_t *data) {
    if (data == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    memset(data, 0, sizeof(*data));
    data->last_sample_tick_ms = HAL_GetTick();
    data->is_valid = false;

    /*
     * NOTE: GPIO EXTI or TIM Input Capture peripheral init must be performed
     * by MX_GPIO_Init() / MX_TIMx_Init() (CubeMX-generated) BEFORE this
     * function is called.  This driver does not touch registers directly so
     * it stays board-agnostic.
     */

    return RESULT_OK;
}

result_t poll_flow_sensor(flow_sensor_data_t *data) {
    if (data == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    uint32_t now_ms      = HAL_GetTick();
    uint32_t elapsed_ms  = now_ms - data->last_sample_tick_ms;

    /* Guard against zero elapsed (called too quickly) or wrap-around */
    if (elapsed_ms == 0U) {
        return RESULT_OK;
    }

    /* Snapshot pulses counted by the ISR since last poll */
    uint32_t pulses = snapshot_and_reset_pulses(data);
    data->pulse_count_total += pulses;
    data->last_sample_tick_ms = now_ms;

    /*
     * Flow rate (fixed-point ×100, unit: ml/min):
     *
     *   rate [ml/min] = (pulses / PULSES_PER_ML) × (60000 / elapsed_ms)
     *
     * Keeping ×100 precision and using PULSES_PER_ML_X10 = 55:
     *
     *   rate_x100 = (pulses × 100 × 60000 × 10) / (55 × elapsed_ms)
     *             = (pulses × 60_000_000)         / (55 × elapsed_ms)
     *
     * Maximum before overflow (uint32): pulses ≈ 150 pulses/s × 5 s = 750
     *   750 × 60_000_000 = 45_000_000_000 → needs uint64 intermediate.
     */
    uint64_t numerator   = (uint64_t)pulses * 60000000ULL;
    uint32_t denominator = FLOW_SENSOR_PULSES_PER_ML_X10 * elapsed_ms;

    data->flow_rate_ml_min_x100 = (denominator > 0U)
        ? (uint32_t)(numerator / denominator)
        : 0U;

    /*
     * Accumulate total volume (µl):
     *   delta_ul = (pulses × 10000) / PULSES_PER_ML_X10
     *            = (pulses × 10000) / 55
     */
    uint32_t delta_ul = (pulses * 10000U) / FLOW_SENSOR_PULSES_PER_ML_X10;
    data->total_volume_ul += delta_ul;

    data->is_valid = true;
    return RESULT_OK;
}

result_t flow_sensor_get_flow_rate(const flow_sensor_data_t *data,
                                   uint32_t *flow_rate_ml_min_x100) {
    if (data == NULL || flow_rate_ml_min_x100 == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *flow_rate_ml_min_x100 = data->flow_rate_ml_min_x100;
    return RESULT_OK;
}

result_t flow_sensor_get_total_volume_ml(const flow_sensor_data_t *data,
                                         uint32_t *total_volume_ml) {
    if (data == NULL || total_volume_ml == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    /* Convert µl → ml (integer, floor) */
    *total_volume_ml = data->total_volume_ul / 1000U;
    return RESULT_OK;
}

result_t flow_sensor_get_pulse_count(const flow_sensor_data_t *data,
                                     uint32_t *pulse_count) {
    if (data == NULL || pulse_count == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *pulse_count = data->pulse_count_total;
    return RESULT_OK;
}

result_t flow_sensor_is_valid(const flow_sensor_data_t *data, bool *is_valid) {
    if (data == NULL || is_valid == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *is_valid = data->is_valid;
    return RESULT_OK;
}

/* Called from EXTI or TIM IC ISR — must be fast and side-effect-free */
void flow_sensor_pulse_isr(flow_sensor_data_t *data) {
    if (data == NULL) {
        return;
    }
    data->pulse_count_window++;
}