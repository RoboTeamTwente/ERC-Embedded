#ifndef PUMP_H
#define PUMP_H

/**
 * @file pump.h
 * @brief DC 12V Mini Peristaltic Pump Driver (2×4mm hose, controlled current)
 *
 * Hardware assumptions
 * ====================
 * Speed control : PWM output on a TIM channel driving a motor driver
 *                 (e.g. DRV8833, L298N half-bridge, or MOSFET gate).
 *                 Duty cycle 0–100 % maps directly to speed_percent 0–100.
 *
 * Direction     : Single GPIO output to the motor driver IN2/DIR pin.
 *                   GPIO HIGH → forward (default)
 *                   GPIO LOW  → reverse
 *
 * Enable        : Optional second GPIO for driver SLEEP/EN pin.
 *                 If your driver has no enable pin, tie it high in hardware
 *                 and the enable GPIO handle can be left NULL.
 *
 * Peripheral handles must be initialised by CubeMX-generated MX_TIMx_Init()
 * and MX_GPIO_Init() before pump_init() is called.
 *
 * Caller must supply concrete handles at init time (see pump_hw_t).
 */

#include "result.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* ---- Hardware binding ---------------------------------------------------- */

/**
 * @brief Hardware resource handles for one pump instance.
 *        Fill this struct before calling pump_init().
 */
typedef struct {
    TIM_HandleTypeDef *htim;        /**< PWM timer handle                   */
    uint32_t           tim_channel; /**< TIM_CHANNEL_1 … TIM_CHANNEL_4      */
    uint32_t           tim_period;  /**< htim->Init.Period (auto-reload val) */

    GPIO_TypeDef      *dir_port;    /**< Direction GPIO port (e.g. GPIOB)   */
    uint16_t           dir_pin;     /**< Direction GPIO pin  (e.g. GPIO_PIN_0) */

    /* Optional enable pin — set both to NULL/0 if not used */
    GPIO_TypeDef      *en_port;     /**< Enable GPIO port, or NULL          */
    uint16_t           en_pin;      /**< Enable GPIO pin,  or 0             */
} pump_hw_t;

/* ---- Data structure ------------------------------------------------------ */

typedef struct {
    pump_hw_t hw;               /**< Hardware handles (copied at init)      */

    bool     enabled;           /**< Current on/off state                   */
    bool     direction;         /**< true = forward, false = reverse        */
    uint32_t speed_percent;     /**< Requested speed 0–100 %                */
    uint32_t speed_rpm;         /**< Estimated RPM (0 if unknown)           */
    bool     is_initialised;
} pump_data_t;

/* ---- Estimated RPM conversion ------------------------------------------- */
/*
 * The DC 12V mini peristaltic pump is unloaded at ~100 RPM @ 12 V.
 * Without encoder feedback we linearly interpolate from duty cycle:
 *   rpm ≈ speed_percent × PUMP_MAX_RPM_EST / 100
 * Override PUMP_MAX_RPM_EST in your build flags if you have a better figure.
 */
#ifndef PUMP_MAX_RPM_EST
#define PUMP_MAX_RPM_EST  100U
#endif

/* ---- Public API ---------------------------------------------------------- */

/**
 * @brief Initialise the pump, start PWM at 0 % duty, direction = forward.
 * @param data  Caller-allocated pump_data_t
 * @param hw    Pointer to filled pump_hw_t (contents are copied in)
 * @return RESULT_OK | RESULT_ERR_INVALID_ARG | RESULT_ERR_HAL
 */
result_t pump_init(pump_data_t *data, const pump_hw_t *hw);

/**
 * @brief Enable or disable the pump output.
 *        Disable sets PWM duty to 0 % and de-asserts the enable pin.
 */
result_t pump_set_enabled(pump_data_t *data, bool enabled);

/**
 * @brief Set pump direction (true = forward, false = reverse).
 *        The pump is briefly stopped (duty = 0) before direction changes
 *        to protect the motor driver.
 */
result_t pump_set_direction(pump_data_t *data, bool forward);

/**
 * @brief Set pump speed as a percentage (0–100).
 *        Clamps silently to [0, 100].
 */
result_t pump_set_speed_percent(pump_data_t *data, uint32_t speed_percent);

/* --- Getters (for filling the proto) ------------------------------------- */

result_t pump_get_enabled(const pump_data_t *data, bool *enabled);
result_t pump_get_direction(const pump_data_t *data, bool *direction);
result_t pump_get_speed_percent(const pump_data_t *data, uint32_t *speed_percent);
result_t pump_get_speed_rpm(const pump_data_t *data, uint32_t *speed_rpm);

#endif /* PUMP_H */