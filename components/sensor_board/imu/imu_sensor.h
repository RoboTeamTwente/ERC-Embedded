/**
 * @file imu_sensor.h
 * @brief Header file for the IMU sensor data handling.
 *
 * This file defines the data structure for holding IMU data and declares
 * functions for initializing, reading, and processing IMU sensor values.
 */

#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

#include "result.h"
#include <stdbool.h>
#include <stdint.h>
/**
 * @brief Structure to hold data from the Inertial Measurement Unit (IMU).
 */
typedef struct {
  float accel[3];          /**< Acceleration along X, Y, Z axes. */
  float gyro[3];           /**< Angular velocity around X, Y, Z axes. */
  float mag[3];            /**< Magnetic field strength along X, Y, Z axes. */
  uint32_t timestamp;      /**< Timestamp of the current sensor reading. */
  uint32_t last_timestamp; /**< Timestamp of the previous sensor reading. */
} imu_data_t;

/**
 * @brief Initializes the IMU data structure.
 * @param imu Pointer to the imu_data_t structure to initialize.
 */
void imu_sensor_init(imu_data_t *imu);

/**
 * @brief Polls the IMU sensor for a new reading and updates the data structure.
 *
 * This function should contain the hardware-specific code to read from the IMU
 * (e.g., via I2C or SPI).
 *
 * @param imu Pointer to the imu_data_t structure to update.
 * @return RESULT_OK on success, or an error code if fetching data fails.
 */
result_t poll_imu_sensor(imu_data_t *imu);

/**
 * @brief Reads the current data from the IMU sensor structure.
 * @param imu Pointer to the source imu_data_t structure.
 * @param data Pointer to the destination imu_data_t structure to copy data
 * into.
 */
void imu_sensor_read(imu_data_t *imu, imu_data_t *data);

#include "result.h"

/**
 * @brief Updates the IMU sensor data with new raw values.
 *
 * @param imu Pointer to the IMU data structure.
 * @param ax Raw accelerometer X-axis data.
 * @param ay Raw accelerometer Y-axis data.
 * @param az Raw accelerometer Z-axis data.
 * @param gx Raw gyroscope X-axis data.
 * @param gy Raw gyroscope Y-axis data.
 * @param gz Raw gyroscope Z-axis data.
 * @param mx Raw magnetometer X-axis data.
 * @param my Raw magnetometer Y-axis data.
 * @param mz Raw magnetometer Z-axis data.
 * @param timestamp The timestamp of the reading.
 * @return result_t RESULT_OK if successful, otherwise an error code.
 */
result_t imu_sensor_update(imu_data_t *imu, float ax, float ay, float az,
                           float gx, float gy, float gz, float mx, float my,
                           float mz, uint32_t timestamp);

/**
 * @brief Calculates the magnitude of the acceleration vector.
 * @param imu Pointer to the imu_data_t structure.
 * @return The magnitude of the acceleration.
 */
float imu_get_acceleration_magnitude(imu_data_t *imu);

/**
 * @brief Calculates the pitch angle from accelerometer data.
 * @param imu Pointer to the imu_data_t structure.
 * @return The pitch angle in degrees.
 */
float imu_get_pitch(imu_data_t *imu);

/**
 * @brief Calculates the roll angle from accelerometer data.
 * @param imu Pointer to the imu_data_t structure.
 * @return The roll angle in degrees.
 */
float imu_get_roll(imu_data_t *imu);

/**
 * @brief Validates if the accelerometer readings are within a plausible range.
 * @param imu Pointer to the imu_data_t structure.
 * @return True if the readings are valid, false otherwise.
 */
bool imu_validate_accelerometer_range(imu_data_t *imu);

/**
 * @brief Validates if the gyroscope readings are within a plausible range.
 * @param imu Pointer to the imu_data_t structure.
 * @return True if the readings are valid, false otherwise.
 */
bool imu_validate_gyroscope_range(imu_data_t *imu);

/**
 * @brief Validates if the magnetometer readings are within a plausible range.
 * @param imu Pointer to the imu_data_t structure.
 * @return True if the readings are valid, false otherwise.
 */
bool imu_validate_magnetometer_range(imu_data_t *imu);

/**
 * @brief Checks for significant gyroscope drift.
 * @param imu Pointer to the imu_data_t structure.
 * @param drift_threshold The maximum allowed drift.
 * @return True if drift is within the threshold, false otherwise.
 */
bool imu_check_gyroscope_drift(imu_data_t *imu, float drift_threshold);

/**
 * @brief Gets the timestamp of the last IMU reading.
 * @param imu Pointer to the imu_data_t structure.
 * @return The timestamp.
 */
uint32_t imu_get_timestamp(imu_data_t *imu);

#endif
