/**
 * @file pump.c
 * @brief DC 12V Mini Peristaltic Pump Driver Implementation
 *
 * PWM DUTY CYCLE CALCULATION
 * ==========================
 * STM32 TIM compare register value for a given duty:
 *
 *   CCR = (speed_percent × (ARR + 1)) / 100
 *
 * where ARR = htim->Init.Period (auto-reload register value).
 * HAL_TIM_PWM_Start() must have been called once (done in pump_init).
 * Subsequent speed changes go through __HAL_TIM_SET_COMPARE().
 *
 * DIRECTION CHANGE SAFETY
 * =======================
 * Reversing polarity under load can spike current on cheap motor drivers.
 * pump_set_direction() sets duty to 0, waits PUMP_DIR_CHANGE_DELAY_MS,
 * flips the GPIO, then restores the previous duty.
 */

#include "pump.h"

#include <string.h>
#include "cmsis_os2.h"   /* osDelay */

/* Milliseconds to coast at 0 % before reversing direction */
#define PUMP_DIR_CHANGE_DELAY_MS   50U

/* --------------------------------------------------------------------------
 * Internal helper: apply speed_percent to the PWM compare register
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
 * Internal helper: assert / de-assert the enable pin (if wired)
 * -------------------------------------------------------------------------- */
static void set_enable_pin(const pump_data_t *data, bool active) {
    if (data->hw.en_port == NULL) {
        return; /* No enable pin — nothing to do */
    }
    HAL_GPIO_WritePin(data->hw.en_port, data->hw.en_pin,
                      active ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

result_t pump_init(pump_data_t *data, const pump_hw_t *hw) {
    if (data == NULL || hw == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    if (hw->htim == NULL || hw->dir_port == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    memset(data, 0, sizeof(*data));
    data->hw = *hw;          /* Copy hardware handles */

    data->enabled        = false;
    data->direction      = true;   /* Forward */
    data->speed_percent  = 0U;
    data->speed_rpm      = 0U;

    /* Start PWM output at 0 % duty */
    if (HAL_TIM_PWM_Start(data->hw.htim, data->hw.tim_channel) != HAL_OK) {
        return RESULT_ERR_HAL;
    }
    apply_pwm_duty(data, 0U);

    /* Direction pin: default forward (HIGH) */
    HAL_GPIO_WritePin(data->hw.dir_port, data->hw.dir_pin, GPIO_PIN_SET);

    /* Enable pin: de-asserted until explicitly enabled */
    set_enable_pin(data, false);

    data->is_initialised = true;
    return RESULT_OK;
}

result_t pump_set_enabled(pump_data_t *data, bool enabled) {
    if (data == NULL || !data->is_initialised) {
        return RESULT_ERR_INVALID_ARG;
    }

    data->enabled = enabled;

    if (enabled) {
        set_enable_pin(data, true);
        apply_pwm_duty(data, data->speed_percent);
    } else {
        apply_pwm_duty(data, 0U);
        set_enable_pin(data, false);
        data->speed_rpm = 0U;
    }

    return RESULT_OK;
}

result_t pump_set_direction(pump_data_t *data, bool forward) {
    if (data == NULL || !data->is_initialised) {
        return RESULT_ERR_INVALID_ARG;
    }

    if (data->direction == forward) {
        return RESULT_OK; /* Already at requested direction */
    }

    /* Coast to zero before reversing to protect driver */
    uint32_t saved_speed = data->speed_percent;
    apply_pwm_duty(data, 0U);
    osDelay(PUMP_DIR_CHANGE_DELAY_MS);

    data->direction = forward;
    HAL_GPIO_WritePin(data->hw.dir_port, data->hw.dir_pin,
                      forward ? GPIO_PIN_SET : GPIO_PIN_RESET);

    /* Restore speed only if still enabled */
    if (data->enabled) {
        apply_pwm_duty(data, saved_speed);
    }

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