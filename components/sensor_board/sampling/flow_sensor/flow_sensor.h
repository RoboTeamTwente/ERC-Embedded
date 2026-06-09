#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

/**
 * @file flow_sensor.h
 * @brief FM-PS2216 Water Flow Sensor Driver
 *
 * Hardware: FM-PS2216, range 40–150 ml/min, 2×4mm tubing
 * Pulse factor: 5.5 pulses/ml (per datasheet)
 *
 * Wiring assumption:
 *   - Signal pin → GPIO EXTI (e.g. PC6, TIM3_CH1 for hardware counting)
 *   - VCC 5V, GND
 *
 * flow_rate_ml_min_x100 is fixed-point: value / 100 = ml/min
 * e.g. 7350 → 73.50 ml/min
 */

#include "result.h"
#include <stdbool.h>
#include <stdint.h>

/* ---- Tuning constants ---------------------------------------------------- */

/** Pulses per millilitre (FM-PS2216 datasheet: 5.5 pulses/ml) scaled ×10 */
#define FLOW_SENSOR_PULSES_PER_ML_X10   55U   /* 5.5 × 10 */

/** Sampling window in milliseconds used for flow-rate calculation */
#define FLOW_SENSOR_SAMPLE_WINDOW_MS    1000U

/** Maximum believable flow rate in ml/min (sensor max = 150 ml/min) */
#define FLOW_SENSOR_MAX_FLOW_ML_MIN     150U

/* ---- Data structure ------------------------------------------------------ */

typedef struct {
    /* Fixed-point flow rate: divide by 100 to get ml/min                    */
    uint32_t flow_rate_ml_min_x100;

    /* Cumulative volume in microlitres (µl) to keep integer precision        */
    uint32_t total_volume_ul;

    /* Raw pulse counter, reset each sampling window                          */
    uint32_t pulse_count_window;

    /* Lifetime pulse counter (never reset, wraps at UINT32_MAX)              */
    uint32_t pulse_count_total;

    /* Timestamp of last sample window start (HAL_GetTick ms)                 */
    uint32_t last_sample_tick_ms;

    /* True once first full sample window has completed                       */
    bool is_valid;
} flow_sensor_data_t;

/* ---- Public API ---------------------------------------------------------- */

/**
 * @brief Initialise the flow sensor data struct and start EXTI pulse counting.
 *        Call once before the main loop.
 * @param data  Pointer to caller-allocated flow_sensor_data_t
 * @return RESULT_OK on success, RESULT_ERR_INVALID_ARG if data is NULL
 */
result_t flow_sensor_init(flow_sensor_data_t *data);

/**
 * @brief Poll the flow sensor: compute flow rate and accumulate volume.
 *        Call periodically (e.g. every MAIN_TASK_DELAY_MS).
 *        Returns RESULT_ERR_UNIMPLEMENTED if HAL timer/EXTI not yet wired.
 * @param data  Pointer to flow_sensor_data_t
 * @return RESULT_OK | RESULT_ERR_INVALID_ARG | RESULT_ERR_UNIMPLEMENTED
 */
result_t poll_flow_sensor(flow_sensor_data_t *data);

/**
 * @brief Get the current flow rate (fixed-point ×100, divide by 100 → ml/min)
 */
result_t flow_sensor_get_flow_rate(const flow_sensor_data_t *data,
                                   uint32_t *flow_rate_ml_min_x100);

/**
 * @brief Get total accumulated volume in millilitres (integer, rounded)
 */
result_t flow_sensor_get_total_volume_ml(const flow_sensor_data_t *data,
                                         uint32_t *total_volume_ml);

/**
 * @brief Get the raw pulse count from the last sampling window
 */
result_t flow_sensor_get_pulse_count(const flow_sensor_data_t *data,
                                     uint32_t *pulse_count);

/**
 * @brief Check whether at least one full sample window has completed
 */
result_t flow_sensor_is_valid(const flow_sensor_data_t *data, bool *is_valid);

/**
 * @brief ISR-safe callback — call from HAL_GPIO_EXTI_Callback or TIM IC ISR
 *        for the flow sensor GPIO line.
 * @param data  Pointer to the active flow_sensor_data_t
 */
void flow_sensor_pulse_isr(flow_sensor_data_t *data);

#endif /* FLOW_SENSOR_H */