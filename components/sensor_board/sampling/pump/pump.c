/**
 * @file pump.c
 * @brief 12V DC Mini Peristaltic Pump Driver — single MOSFET switch
 *
 * PWM DUTY CYCLE CALCULATION (MOSFET gate)
 * ========================================
 * STM32 TIM compare register value for a given duty:
 *
 *   CCR = (speed_percent × (ARR + 1)) / 100
 *
 * where ARR = htim->Init.Period (auto-reload register value).
 * HAL_TIM_PWM_Start() must have been called once (done in pump_init).
 * Subsequent speed changes go through __HAL_TIM_SET_COMPARE().
 *
 * The motor is a plain 2-wire DC motor switched low-side by one MOSFET, so
 * there is no direction or brake control: duty 0 % = off, duty 100 % = full.
 */

#include "pump.h"

#include <string.h>

/* --------------------------------------------------------------------------
 * Internal helper: apply speed_percent to the gate PWM compare register
 * -------------------------------------------------------------------------- */
static result_t apply_pwm_duty(pump_data_t *data, uint32_t speed_percent) {
    if (speed_percent > 100U) {
        speed_percent = 100U;
    }

    /* CCR = speed_percent × (ARR+1) / 100 */
    uint32_t ccr = (speed_percent * (data->hw.tim_period + 1U)) / 100U;

    __HAL_TIM_SET_COMPARE(data->hw.htim, data->hw.tim_channel, ccr);
    return RESULT_OK;
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

result_t pump_init(pump_data_t *data, const pump_hw_t *hw) {
    if (data == NULL || hw == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    if (hw->htim == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    memset(data, 0, sizeof(*data));
    data->hw = *hw;          /* Copy hardware handles */

    data->enabled        = false;
    data->direction      = true;   /* Stored only — no hardware effect */
    data->speed_percent  = 0U;
    data->speed_rpm      = 0U;

    /* Start gate PWM output at 0 % duty (motor off) */
    if (HAL_TIM_PWM_Start(data->hw.htim, data->hw.tim_channel) != HAL_OK) {
        return RESULT_ERR_IO;
    }
    apply_pwm_duty(data, 0U);

    data->is_initialised = true;
    return RESULT_OK;
}

result_t pump_set_enabled(pump_data_t *data, bool enabled) {
    if (data == NULL || !data->is_initialised) {
        return RESULT_ERR_INVALID_ARG;
    }

    data->enabled = enabled;

    if (enabled) {
        apply_pwm_duty(data, data->speed_percent);
    } else {
        apply_pwm_duty(data, 0U);
        data->speed_rpm = 0U;
    }

    return RESULT_OK;
}

result_t pump_set_direction(pump_data_t *data, bool forward) {
    if (data == NULL || !data->is_initialised) {
        return RESULT_ERR_INVALID_ARG;
    }

    /* Single MOSFET = unidirectional. Store the flag for proto round-trip;
     * there is no GPIO to drive and no effect on the motor. */
    data->direction = forward;
    return RESULT_OK;
}

result_t pump_set_speed_percent(pump_data_t *data, uint32_t speed_percent) {
    if (data == NULL || !data->is_initialised) {
        return RESULT_ERR_INVALID_ARG;
    }

    if (speed_percent > 100U) {
        speed_percent = 100U;
    }

    data->speed_percent = speed_percent;
    data->speed_rpm     = (speed_percent * PUMP_MAX_RPM_EST) / 100U;

    if (data->enabled) {
        apply_pwm_duty(data, speed_percent);
    }

    return RESULT_OK;
}

result_t pump_get_enabled(const pump_data_t *data, bool *enabled) {
    if (data == NULL || enabled == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *enabled = data->enabled;
    return RESULT_OK;
}

result_t pump_get_direction(const pump_data_t *data, bool *direction) {
    if (data == NULL || direction == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *direction = data->direction;
    return RESULT_OK;
}

result_t pump_get_speed_percent(const pump_data_t *data, uint32_t *speed_percent) {
    if (data == NULL || speed_percent == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *speed_percent = data->speed_percent;
    return RESULT_OK;
}

result_t pump_get_speed_rpm(const pump_data_t *data, uint32_t *speed_rpm) {
    if (data == NULL || speed_rpm == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *speed_rpm = data->speed_rpm;
    return RESULT_OK;
}
