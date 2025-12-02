#include "imu_sensor.h"
#include "logging.h"
#include "result.h"
#include <math.h>
#include <string.h>

void imu_sensor_init(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to init");
        return;
    }
    
    LOGI("IMU", "Initializing IMU sensor");
    
    // Initializing sensor data to zero
    memset(imu, 0, sizeof(imu_data_t));
}

void imu_sensor_read(imu_data_t *imu, imu_data_t *data) {
    if (imu == NULL || data == NULL) {
        LOGE("IMU", "NULL pointer provided");
        return;
    }
    *data = *imu;     // Copying the current IMU data
}

result_t imu_sensor_update(imu_data_t *imu, float ax, float ay, float az,
                        float gx, float gy, float gz,
                        float mx, float my, float mz,
                        uint32_t timestamp) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to update");
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Updating accelerometer data (m/s²)
    imu->accel[0] = ax;
    imu->accel[1] = ay;
    imu->accel[2] = az;

    // Updating gyroscope data (degrees/second) (ANGULAR VELOCITY)
    imu->gyro[0] = gx;
    imu->gyro[1] = gy;
    imu->gyro[2] = gz;

    // Updating magnetometer data (in µT)
    imu->mag[0] = mx;
    imu->mag[1] = my;
    imu->mag[2] = mz;

    // Updating the timestamp
    imu->timestamp = timestamp;
    imu->last_timestamp = timestamp;

    return RESULT_OK;
}

result_t poll_imu_sensor(imu_data_t *imu) {
    if (imu == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    // TODO: Implement actual hardware communication to read from the IMU.
    // This typically involves reading sensor registers via I2C or SPI.

    // For now, we use hardcoded data for testing and development.
    float ax = 1.0f, ay = 2.0f, az = 9.8f; // accel values
    float gx = 0.1f, gy = 0.2f, gz = 0.3f; // gyro values
    float mx = 10.0f, my = 20.0f, mz = 30.0f; // magnetometer values
    uint32_t timestamp = 0; // Should be replaced with HAL_GetTick() or similar

    return imu_sensor_update(imu, ax, ay, az, gx, gy, gz, mx, my, mz, timestamp);
}


float imu_get_acceleration_magnitude(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to get_acceleration_magnitude");
        return 0.0f;
    }
    
    return sqrtf(imu->accel[0] * imu->accel[0] + 
                 imu->accel[1] * imu->accel[1] + 
                 imu->accel[2] * imu->accel[2]);
}

// using atan2f because its useful for calculating with floats directly
float imu_get_pitch(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to get_pitch");
        return 0.0f;
    }
    
    return atan2f(imu->accel[1], 
                  sqrtf(imu->accel[0] * imu->accel[0] + 
                        imu->accel[2] * imu->accel[2])) * 180.0f / M_PI;
}

float imu_get_roll(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to get_roll");
        return 0.0f;
    }
    
    return atan2f(imu->accel[0],
                  sqrtf(imu->accel[1] * imu->accel[1] +
                        imu->accel[2] * imu->accel[2])) * 180.0f / M_PI;
}

bool imu_validate_accelerometer_range(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to validate_accelerometer_range");
        return false;
    }

    const float max_accel = 16.0f * 9.80665f; // ±16g in m/s²
    
    return (fabsf(imu->accel[0]) <= max_accel &&
            fabsf(imu->accel[1]) <= max_accel &&
            fabsf(imu->accel[2]) <= max_accel);
}

bool imu_validate_gyroscope_range(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to validate_gyroscope_range");
        return false;
    }
    
    const float max_gyro = 2000.0f; // ±2000 deg/s
    
    return (fabsf(imu->gyro[0]) <= max_gyro &&
            fabsf(imu->gyro[1]) <= max_gyro &&
            fabsf(imu->gyro[2]) <= max_gyro);
}

bool imu_validate_magnetometer_range(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to validate_magnetometer_range");
        return false;
    }
    
    const float max_mag = 4900.0f; // ±4900 µT
    
    return (fabsf(imu->mag[0]) <= max_mag &&
            fabsf(imu->mag[1]) <= max_mag &&
            fabsf(imu->mag[2]) <= max_mag);
}

bool imu_check_gyroscope_drift(imu_data_t *imu, float drift_threshold) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to check_gyroscope_drift");
        return false;
    }
    
    return (fabsf(imu->gyro[0]) < drift_threshold &&
            fabsf(imu->gyro[1]) < drift_threshold &&
            fabsf(imu->gyro[2]) < drift_threshold);
}

uint32_t imu_get_timestamp(imu_data_t *imu) {
    if (imu == NULL) {
        LOGE("IMU", "NULL pointer provided to get_timestamp");
        return 0;
    }
    
    return imu->timestamp;
}






// This takes me back to school in Physics class!

// Mathematical formulas used in this file:
//
// 1. Acceleration Magnitude:
//    |a| = √(ax² + ay² + az²)
//    where ax, ay, az are acceleration components in m/s²
//
// 2. Pitch (rotation around Y-axis):
//    pitch = arctan2(ay, √(ax² + az²)) × (180/π)
//    Converts from radians to degrees
//
// 3. Roll (rotation around X-axis):
//    roll = arctan2(ax, √(ay² + az²)) × (180/π)
//    Converts from radians to degrees
//
// Note: Pitch and Roll calculations assume the device is relatively stationary and use the gravity vector for orientation estimation.