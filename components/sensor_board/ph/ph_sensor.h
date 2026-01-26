#ifndef PH_SENSOR_H
#define PH_SENSOR_H

#include "result.h"
#include <stdint.h>

/**
 * @brief Number of samples for averaging (noise filtering).
 * @note Based on DFRobot SEN0161 sample code recommendation.
 */
#define PH_SAMPLE_COUNT 40

/**
 * @brief Default pH slope for DFRobot SEN0161.
 * @note pH = 3.5 * voltage + offset (at 25°C)
 */
#define PH_DEFAULT_SLOPE 3.5f

/**
 * @brief Structure to hold pH sensor calibration data.
 * using PH meter SKU SEN0161
 */
typedef struct {
    float offset; /**< pH offset for calibration (added to calculated pH). */
    float slope;  /**< Slope for pH calculation (default 3.5 for SEN0161). */
} ph_calibration_t;

/**
 * @brief Structure to hold data for a pH sensor.
 */
typedef struct {
    uint16_t raw_value;              /**< Raw ADC value from the sensor. */
    float voltage;                   /**< Voltage calculated from the raw ADC value. */
    float ph_value;                  /**< Calculated pH value. */
    float reference_voltage;         /**< Reference voltage of the ADC (should be close to 5.0V). */
    ph_calibration_t calibration;    /**< Calibration data for the sensor. */
    uint16_t sample_buffer[PH_SAMPLE_COUNT]; /**< Buffer for averaging samples. */
    uint8_t sample_index;            /**< Current index in the sample buffer. */
    uint8_t samples_collected;       /**< Number of samples collected so far. */
} ph_sensor_t;

/**
 * @brief Initializes the pH sensor data structure with default values.
 * @param sensor Pointer to the ph_sensor_t structure to initialize.
 * @param reference_voltage The reference voltage used by the ADC (should be close to 5.0V for SEN0161).
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if sensor is NULL.
 * @note For best accuracy with SEN0161, use a stable 5.0V reference.
 */
result_t ph_sensor_init(ph_sensor_t *sensor, float reference_voltage);

/**
 * @brief Adds a single ADC sample to the averaging buffer.
 * @param sensor Pointer to the ph_sensor_t structure.
 * @param adc_value The ADC value to add to the buffer.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if sensor is NULL.
 * @note Call this function at regular intervals (e.g., every 20ms) for best results.
 */
result_t ph_sensor_add_sample(ph_sensor_t *sensor, uint16_t adc_value);

/**
 * @brief Updates the pH sensor data with a new ADC reading.
 * @param sensor Pointer to the ph_sensor_t structure to update.
 * @param adc_value The new ADC value.
 * @param adc_max The maximum possible ADC value.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if sensor is NULL or adc_max is 0.
 */
result_t ph_sensor_update(ph_sensor_t *sensor, uint16_t adc_value, uint16_t adc_max);

/**
 * @brief Polls the pH sensor for a new reading and updates the data structure.
 * 
 * This function should contain the hardware-specific code to read the ADC value
 * for the pH sensor.
 * 
 * @param sensor Pointer to the ph_sensor_t structure to update.
 * @return RESULT_OK on success, or an error code if fetching data fails.
 */
result_t poll_ph_sensor(ph_sensor_t *sensor);

/**
 * @brief Gets the last calculated pH value.
 * @param sensor Pointer to the ph_sensor_t structure.
 * @param ph_value Pointer to a float where the pH value will be stored.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if sensor or ph_value is NULL.
 */
result_t ph_sensor_get_value(ph_sensor_t *sensor, float *ph_value);

/**
 * @brief Gets the last calculated voltage.
 * @param sensor Pointer to the ph_sensor_t structure.
 * @param voltage Pointer to a float where the voltage will be stored.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if sensor or voltage is NULL.
 */
result_t ph_sensor_get_voltage(ph_sensor_t *sensor, float *voltage);

/**
 * @brief Calibrates the pH sensor with a given offset and slope.
 * @param sensor Pointer to the ph_sensor_t structure to calibrate.
 * @param offset The pH offset for calibration (difference between measured and actual pH at 7.0).
 * @param slope The slope for pH calculation (default 3.5 for SEN0161).
 * @return RESULT_OK on success.
 * @note Calibration procedure:
 *       1. Short BNC input or use pH 7.0 buffer, record pH value.
 *       2. offset = 7.0 - recorded_value
 *       3. Use pH 4.0 buffer, adjust gain potentiometer until reading is 4.0.
 */
result_t ph_sensor_calibrate(ph_sensor_t *sensor, float offset, float slope);

/**
 * @brief Resets the pH sensor calibration to its default values.
 * @param sensor Pointer to the ph_sensor_t structure to reset.
 * @return RESULT_OK on success.
 */
result_t ph_sensor_reset_calibration(ph_sensor_t *sensor);

#endif // PH_SENSOR_H
