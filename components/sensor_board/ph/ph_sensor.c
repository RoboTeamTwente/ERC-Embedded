#include "ph_sensor.h"
#include "result.h"
#include <stdlib.h>

result_t ph_sensor_init(ph_sensor_t *sensor, float reference_voltage) {
    if (sensor == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    sensor->raw_value = 0;
    sensor->voltage = 0.0f;
    sensor->ph_value = 7.0f;
    sensor->reference_voltage = reference_voltage;
    ph_sensor_reset_calibration(sensor);

    return RESULT_OK;
}

result_t ph_sensor_update(ph_sensor_t *sensor, uint16_t adc_value, uint16_t adc_max) {
    if (sensor == NULL || adc_max == 0) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    sensor->raw_value = adc_value;
    sensor->voltage = (float)adc_value * sensor->reference_voltage / (float)adc_max;
    
    // Convert voltage to pH using the linear relationship
    // pH = 7.0 - (voltage - neutral_voltage) / slope
    float neutral_voltage = sensor->reference_voltage / 2.0f; // Neutral pH at mid-point
    float voltage_diff_mv = (sensor->voltage - neutral_voltage + sensor->calibration.offset) * 1000.0f;
    sensor->ph_value = 7.0f - (voltage_diff_mv / sensor->calibration.slope);
    
    // restricts pH value to valid range (0-14)
    if (sensor->ph_value < 0.0f) {
        sensor->ph_value = 0.0f;
    } else if (sensor->ph_value > 14.0f) {
        sensor->ph_value = 14.0f;
    }

    return RESULT_OK;
}

result_t poll_ph_sensor(ph_sensor_t *sensor) {
    if (sensor == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    // TODO: Implement actual hardware communication to read from the ADC.
    // This typically involves:
    // 1. Selecting the correct ADC channel for the pH sensor.
    // 2. Starting an ADC conversion.
    // 3. Waiting for the conversion to complete.
    // 4. Reading the raw ADC value.

    // For now, we use hardcoded data for testing and development.
    uint16_t raw_adc_value = 2048; // Simulated ADC reading
    uint16_t adc_max_value = 4095; // For a 12-bit ADC

    return ph_sensor_update(sensor, raw_adc_value, adc_max_value);
}


result_t ph_sensor_get_value(ph_sensor_t *sensor, float *ph_value) {
    if (sensor == NULL || ph_value == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *ph_value = sensor->ph_value;
    return RESULT_OK;
}

result_t ph_sensor_get_voltage(ph_sensor_t *sensor, float *voltage) {
    if (sensor == NULL || voltage == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    *voltage = sensor->voltage;
    return RESULT_OK;
}

result_t ph_sensor_reset_calibration(ph_sensor_t *sensor) {
    if (sensor == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    sensor->calibration.offset = 0.0f;
    sensor->calibration.slope = 59.16f; // Default slope (mV per pH unit at 25°C)
    return RESULT_OK;
}

result_t ph_sensor_calibrate(ph_sensor_t *sensor, float offset, float slope) {
    if (sensor == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    sensor->calibration.offset = offset;
    sensor->calibration.slope = slope;
    return RESULT_OK;
}