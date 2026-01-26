/**
 * @file ph_sensor.c
 * @brief Implementation of pH sensor handling for DFRobot SEN0161.
 * 
 * @note Target Hardware: DFRobot Analog pH Meter (SKU: SEN0161)
 * @see https://wiki.dfrobot.com/PH_meter_SKU__SEN0161_
 * 
 * The pH calculation uses the formula from DFRobot:
 *   pH = slope * voltage + offset
 * where slope = 3.5 (default) and offset is calibrated.
 */

#include "ph_sensor.h"
#include "result.h"
#include <string.h>

// Forward declaration for internal averaging function
static float ph_calculate_average(ph_sensor_t *sensor);

result_t ph_sensor_init(ph_sensor_t *sensor, float reference_voltage) {
    if (sensor == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    sensor->raw_value = 0;
    sensor->voltage = 0.0f;
    sensor->ph_value = 7.0f;
    sensor->reference_voltage = reference_voltage;
    sensor->sample_index = 0;
    sensor->samples_collected = 0;
    memset(sensor->sample_buffer, 0, sizeof(sensor->sample_buffer));
    
    TRY(ph_sensor_reset_calibration(sensor));

    return RESULT_OK;
}

result_t ph_sensor_add_sample(ph_sensor_t *sensor, uint16_t adc_value) {
    if (sensor == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    sensor->sample_buffer[sensor->sample_index] = adc_value;
    sensor->sample_index = (sensor->sample_index + 1) % PH_SAMPLE_COUNT;
    
    if (sensor->samples_collected < PH_SAMPLE_COUNT) {
        sensor->samples_collected++;
    }
    
    return RESULT_OK;
}

/**
 * @brief Calculates the average of samples, excluding min and max values for noise filtering.
 * @param sensor Pointer to the ph_sensor_t structure.
 * @return The filtered average ADC value.
 * @note Based on DFRobot sample code averaging algorithm.
 */
static float ph_calculate_average(ph_sensor_t *sensor) {
    if (sensor->samples_collected < 5) {
        // Not enough samples for filtered average, use simple average
        uint32_t sum = 0;
        for (uint8_t i = 0; i < sensor->samples_collected; i++) {
            sum += sensor->sample_buffer[i];
        }
        return (float)sum / (float)sensor->samples_collected;
    }
    
    // Find min and max, exclude them from average (noise filtering)
    uint16_t min_val = sensor->sample_buffer[0];
    uint16_t max_val = sensor->sample_buffer[0];
    uint32_t sum = 0;
    
    for (uint8_t i = 0; i < sensor->samples_collected; i++) {
        uint16_t val = sensor->sample_buffer[i];
        if (val < min_val) {
            sum += min_val;
            min_val = val;
        } else if (val > max_val) {
            sum += max_val;
            max_val = val;
        } else {
            sum += val;
        }
    }
    
    // Exclude min and max from average
    return (float)sum / (float)(sensor->samples_collected - 2);
}

result_t ph_sensor_update(ph_sensor_t *sensor, uint16_t adc_value, uint16_t adc_max) {
    if (sensor == NULL || adc_max == 0) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Add sample to buffer for averaging
    TRY(ph_sensor_add_sample(sensor, adc_value));
    
    // Calculate averaged ADC value
    float avg_adc = ph_calculate_average(sensor);
    sensor->raw_value = (uint16_t)avg_adc;
    
    // Convert ADC to voltage
    // For STM32H7 with 16-bit ADC: voltage = (adc_value / adc_max) * reference_voltage
    sensor->voltage = avg_adc * sensor->reference_voltage / (float)adc_max;
    
    // DFRobot SEN0161 formula: pH = slope * voltage + offset
    // Default slope = 3.5 (at 25°C)
    sensor->ph_value = sensor->calibration.slope * sensor->voltage + sensor->calibration.offset;
    
    // Restrict pH value to valid range (0-14)
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

    // TODO SOon: Implement ADC read once ADC peripheral is configured in CubeMX.
    // 
    // Example for STM32H7 with HAL:
    // extern ADC_HandleTypeDef hadc1;
    // 
    // HAL_ADC_Start(&hadc1);
    // if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
    //     return RESULT_ERR_TIMEOUT;
    // }
    // uint16_t raw_adc_value = HAL_ADC_GetValue(&hadc1);
    // HAL_ADC_Stop(&hadc1);
    //
    // return ph_sensor_update(sensor, raw_adc_value, 4095);  // 12-bit ADC

    return RESULT_ERR_UNIMPLEMENTED;
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
    sensor->calibration.offset = 0.0f;  // No offset by default
    sensor->calibration.slope = PH_DEFAULT_SLOPE;  // 3.5 for DFRobot SEN0161
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