#ifndef PUMP_H
#define PUMP_H

/**
 * @file pump.h
 * @brief 12V DC Mini Peristaltic Dosing Pump Driver — single MOSFET switch
 *
 * Target pump
 * ===========
 * Grothen 12 V DC mini peristaltic dosing pump (3×5 mm), a plain 2-wire DC
 * motor. No internal driver, no direction control — apply 12 V, it pumps.
 *
 * Hardware (single low-side MOSFET)
 * =================================
 *   STM32 PWM (TIM ch) --[gate resistor]--> N-MOSFET gate
 *   12V  --> pump +
 *   pump - --> MOSFET drain
 *   MOSFET source --> GND   (common with STM32 GND)
 *   Flyback diode across the pump motor (cathode to 12V) — inductive load.
 *
 *   PWM duty 0..100 % = speed 0..100 %. Duty 0 % = motor off.
 *   Use a logic-level MOSFET (e.g. AO3400, IRLZ44N) so 3.3 V fully turns it on.
 *
 * UNIDIRECTIONAL
 * ==============
 * A single MOSFET cannot reverse the motor. pump_set_direction() therefore
 * only stores the requested flag (so the network/proto field round-trips) and
 * has NO hardware effect. The pump always runs forward.
 *
 * NO FEEDBACK
 * ===========
 * No current-sense or fault line is wired (no ADC enabled in CubeMX). This
 * driver is OPEN-LOOP: it cannot detect whether a pump is physically
 * connected, stalled, or dry. speed_rpm is an ESTIMATE from duty cycle only.
 * Connection/efficacy must be inferred externally (the inline flow sensor) —
 * see main.c.
 *
 * Peripheral handles must be initialised by CubeMX-generated MX_TIMx_Init()
 * before pump_init() is called.
 */

#include "result.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* ---- Hardware binding ---------------------------------------------------- */

/**
 * @brief Hardware resource handles for one pump instance.
 *        The MOSFET gate is driven by a single PWM channel.
 *        Fill this struct before calling pump_init().
 */
typedef struct {
    TIM_HandleTypeDef *htim;        /**< PWM timer handle -> MOSFET gate     */
    uint32_t           tim_channel; /**< TIM_CHANNEL_1 … TIM_CHANNEL_4       */
    uint32_t           tim_period;  /**< htim->Init.Period (auto-reload val) */
} pump_hw_t;

/* ---- Data structure ------------------------------------------------------ */

typedef struct {
    pump_hw_t hw;               /**< Hardware handles (copied at init)      */

    bool     enabled;           /**< Current on/off state                   */
    bool     direction;         /**< Stored only — NO hardware effect       */
    uint32_t speed_percent;     /**< Requested speed 0–100 %                */
    uint32_t speed_rpm;         /**< ESTIMATED RPM from duty (no encoder)   */
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
 * @brief Initialise the pump: start PWM (gate) at 0 % duty = motor off.
 * @param data  Caller-allocated pump_data_t
 * @param hw    Pointer to filled pump_hw_t (contents are copied in)
 * @return RESULT_OK | RESULT_ERR_INVALID_ARG | RESULT_ERR_IO
 */
result_t pump_init(pump_data_t *data, const pump_hw_t *hw);

/**
 * @brief Enable or disable the pump output.
 *        Enable  -> PWM duty = speed_percent.
 *        Disable -> PWM duty = 0 % (motor off).
 */
result_t pump_set_enabled(pump_data_t *data, bool enabled);

/**
 * @brief Set requested direction flag. NO hardware effect (single MOSFET,
 *        unidirectional). Stored only so the proto/network field round-trips.
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
