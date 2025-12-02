#include "sensor_basics.h"
#include "result.h"

// Validate if PH sensor value is within acceptable range (0-14)
result_t validate_ph_value(float ph_value) {
    if (ph_value >= 0.0f && ph_value <= 14.0f) {
        return RESULT_OK;
    }
    return RESULT_ERR_INVALID_DATA;
}

// // Convert temperature from Celsius to Fahrenheit
// result_t celsius_to_fahrenheit(float celsius, float *fahrenheit) {
//     if (fahrenheit == NULL) {
//         return RESULT_ERR_INVALID_ARG;
//     }
//     *fahrenheit = (celsius * 9.0f / 5.0f) + 32.0f;
//     return RESULT_OK;
// }

// // Convert temperature from Fahrenheit to Celsius
// result_t fahrenheit_to_celsius(float fahrenheit, float *celsius) {
//     if (celsius == NULL) {
//         return RESULT_ERR_INVALID_ARG;
//     }
//     *celsius = (fahrenheit - 32.0f) * 5.0f / 9.0f;
//     return RESULT_OK;
// }

// Validate IMU accelerometer data (typical range: -16g to +16g in m/s^2)
// Using -160 to +160 m/s^2 as the acceptable range
result_t validate_accelerometer_value(float accel_value) {
    if (accel_value >= -160.0f && accel_value <= 160.0f) {
        return RESULT_OK;
    }
    return RESULT_ERR_INVALID_DATA;
}

// Validate all three axes of accelerometer data
result_t validate_imu_data(float accel_x, float accel_y, float accel_z) {
    if (validate_accelerometer_value(accel_x) == RESULT_OK &&
        validate_accelerometer_value(accel_y) == RESULT_OK &&
        validate_accelerometer_value(accel_z) == RESULT_OK) {
        return RESULT_OK;
    }
    return RESULT_ERR_INVALID_DATA;
}

// result_t bar_to_psi(float bar, float *psi) {
//     if (psi == NULL) {
//         return RESULT_ERR_INVALID_ARG;
//     }
//     *psi = bar * 14.5038f;
//     return RESULT_OK;
// }

// result_t psi_to_bar(float psi, float *bar) {
//     if (bar == NULL) {
//         return RESULT_ERR_INVALID_ARG;
//     }
//     *bar = psi / 14.5038f;
//     return RESULT_OK;
// }

// Validate GPS latitude value (-90 to +90 degrees)
result_t validate_gps_latitude(double latitude) {
    if (latitude >= -90.0 && latitude <= 90.0) {
        return RESULT_OK;
    }
    return RESULT_ERR_INVALID_DATA;
}

// Validate GPS longitude value (-180 to +180 degrees)
result_t validate_gps_longitude(double longitude) {
    if (longitude >= -180.0 && longitude <= 180.0) {
        return RESULT_OK;
    }
    return RESULT_ERR_INVALID_DATA;
}

// Validate GPS HDOP value (0-50 typical range)
result_t validate_gps_hdop(float hdop) {
    if (hdop >= 0.0f && hdop <= 50.0f) {
        return RESULT_OK;
    }
    return RESULT_ERR_INVALID_DATA;
}

// Validate GPS satellite count (0-30 typical range)
result_t validate_gps_satellite_count(int32_t satellites) {
    if (satellites >= 0 && satellites <= 30) {
        return RESULT_OK;
    }
    return RESULT_ERR_INVALID_DATA;
}
