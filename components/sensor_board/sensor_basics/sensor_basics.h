#ifndef SENSOR_BASICS_H
#define SENSOR_BASICS_H

#include "result.h"
#include <stdint.h>

/**
 * @brief Validates if a pH value is within the acceptable range (0-14).
 * @param ph_value The pH value to validate.
 * @return RESULT_OK if the value is valid, RESULT_ERR_INVALID_DATA otherwise.
 */
result_t validate_ph_value(float ph_value);

/**
 * @brief Converts temperature from Celsius to Fahrenheit.
 * @param celsius The temperature in Celsius.
 * @param fahrenheit Pointer to a float where the converted Fahrenheit temperature will be stored.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if fahrenheit is NULL.
 */
result_t celsius_to_fahrenheit(float celsius, float *fahrenheit);

/**
 * @brief Converts temperature from Fahrenheit to Celsius.
 * @param fahrenheit The temperature in Fahrenheit.
 * @param celsius Pointer to a float where the converted Celsius temperature will be stored.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if celsius is NULL.
 */
result_t fahrenheit_to_celsius(float fahrenheit, float *celsius);

/**
 * @brief Validates if an accelerometer value is within the typical range.
 * @param accel_value The accelerometer value to validate.
 * @return RESULT_OK if the value is valid, RESULT_ERR_INVALID_DATA otherwise.
 */
result_t validate_accelerometer_value(float accel_value);

/**
 * @brief Validates all three axes of accelerometer data.
 * @param accel_x The acceleration value for the X-axis.
 * @param accel_y The acceleration value for the Y-axis.
 * @param accel_z The acceleration value for the Z-axis.
 * @return RESULT_OK if all values are valid, RESULT_ERR_INVALID_DATA otherwise.
 */
result_t validate_imu_data(float accel_x, float accel_y, float accel_z);

/**
 * @brief Converts pressure from bar to psi.
 * @param bar The pressure in bar.
 * @param psi Pointer to a float where the converted psi pressure will be stored.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if psi is NULL.
 */
result_t bar_to_psi(float bar, float *psi);

/**
 * @brief Converts pressure from psi to bar.
 * @param psi The pressure in psi.
 * @param bar Pointer to a float where the converted bar pressure will be stored.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if bar is NULL.
 */
result_t psi_to_bar(float psi, float *bar);

/**
 * @brief Validates GPS latitude value.
 * @param latitude The latitude value to validate (-90 to +90 degrees).
 * @return RESULT_OK if the value is valid, RESULT_ERR_INVALID_DATA otherwise.
 */
result_t validate_gps_latitude(double latitude);

/**
 * @brief Validates GPS longitude value.
 * @param longitude The longitude value to validate (-180 to +180 degrees).
 * @return RESULT_OK if the value is valid, RESULT_ERR_INVALID_DATA otherwise.
 */
result_t validate_gps_longitude(double longitude);

/**
 * @brief Validates GPS HDOP value.
 * @param hdop The HDOP value to validate (0-50 typical range).
 * @return RESULT_OK if the value is valid, RESULT_ERR_INVALID_DATA otherwise.
 */
result_t validate_gps_hdop(float hdop);

/**
 * @brief Validates GPS satellite count.
 * @param satellites The number of satellites to validate.
 * @return RESULT_OK if the value is valid, RESULT_ERR_INVALID_DATA otherwise.
 */
result_t validate_gps_satellite_count(int32_t satellites);

#endif
