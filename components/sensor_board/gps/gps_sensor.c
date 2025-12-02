/**
 * @file gps_sensor.c
 * @brief Implementation of GPS sensor data handling functions.
 * 
 * @note Target Hardware: GY-NEO6MV2 NEO-6M GPS
 */

#include "gps_sensor.h"
#include <math.h>
#include <string.h>

// Earth's radius in meters (mean radius)
#define EARTH_RADIUS_M 6371000.0

// GPS coordinate limits
#define GPS_LAT_MIN -90.0
#define GPS_LAT_MAX 90.0
#define GPS_LON_MIN -180.0
#define GPS_LON_MAX 180.0

// Degree to radian conversion
#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)

result_t gps_sensor_init(gps_data_t *gps) {
    if (gps == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    gps->latitude = 0.0; // Using memset and then re-assigning is redundant.
    gps->longitude = 0.0;
    gps->altitude = 0.0;
    gps->speed = 0.0;
    gps->heading = 0.0;
    gps->hdop = 99.9f;  // High HDOP indicates poor accuracy
    gps->vdop = 99.9f;
    gps->satellites = 0;
    gps->fix_quality = GPS_NO_FIX;
    gps->utc_timestamp = 0;
    gps->is_valid = false;
    gps->timestamp = 0;
    gps->last_timestamp = 0;
    
    return RESULT_OK;
}

result_t gps_sensor_update(gps_data_t *gps, 
                           double latitude,
                           double longitude,
                           float altitude,
                           float speed,
                           float heading,
                           float hdop,
                           float vdop,
                           int32_t satellites,
                           gps_fix_quality_t fix_quality,
                           int64_t utc_timestamp) {
    if (gps == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Validate coordinates
    result_t validation = validate_gps_coordinates(latitude, longitude);
    
    // Update timestamps
    gps->last_timestamp = gps->timestamp;
    // Note: timestamp should be updated with actual system time
    // gps->timestamp = get_system_time(); // Implement based on platform
    
    // Update GPS data
    gps->latitude = latitude;
    gps->longitude = longitude;
    gps->altitude = altitude;
    gps->speed = speed;
    gps->heading = heading;
    gps->hdop = hdop;
    gps->vdop = vdop;
    gps->satellites = satellites;
    gps->fix_quality = fix_quality;
    gps->utc_timestamp = utc_timestamp;
    
    // Determine if data is valid
    gps->is_valid = (validation == RESULT_OK) && 
                    (fix_quality >= GPS_GPS_FIX) && 
                    (satellites >= 4);
    
    return RESULT_OK;
}

result_t poll_gps_sensor(gps_data_t *gps) {
    if (gps == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }

    // TODO: Implement actual hardware communication to read from the GPS module.
    // This typically involves reading NMEA sentences from a UART and parsing them.

    // For now, we use hardcoded data for testing and development.
    return gps_sensor_update(gps, 52.237049, 6.850537, 30.0f, 0.0f, 0.0f, 1.0f, 1.0f, 8, GPS_GPS_FIX, 1672531200000);
}


result_t gps_sensor_read(gps_data_t *gps, gps_data_t *data) {
    if (gps == NULL || data == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    memcpy(data, gps, sizeof(gps_data_t));
    return RESULT_OK;
}

result_t gps_sensor_get_coordinates(gps_data_t *gps, double *latitude, double *longitude) {
    if (gps == NULL || latitude == NULL || longitude == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    *latitude = gps->latitude;
    *longitude = gps->longitude;
    
    return RESULT_OK;
}

result_t gps_sensor_get_altitude(gps_data_t *gps, float *altitude) {
    if (gps == NULL || altitude == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    *altitude = gps->altitude;
    return RESULT_OK;
}

result_t gps_sensor_get_velocity(gps_data_t *gps, float *speed, float *heading) {
    if (gps == NULL || speed == NULL || heading == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    *speed = gps->speed;
    *heading = gps->heading;
    
    return RESULT_OK;
}

result_t gps_sensor_get_fix_info(gps_data_t *gps, gps_fix_quality_t *fix_quality, int32_t *satellites) {
    if (gps == NULL || fix_quality == NULL || satellites == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    *fix_quality = gps->fix_quality;
    *satellites = gps->satellites;
    
    return RESULT_OK;
}

result_t gps_sensor_is_valid(gps_data_t *gps, bool *is_valid) {
    if (gps == NULL || is_valid == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    *is_valid = gps->is_valid;
    return RESULT_OK;
}

result_t validate_gps_coordinates(double latitude, double longitude) {
    if (latitude < GPS_LAT_MIN || latitude > GPS_LAT_MAX ||
        longitude < GPS_LON_MIN || longitude > GPS_LON_MAX) {
        return RESULT_ERR_INVALID_DATA;
    }
    
    return RESULT_OK;
}

result_t gps_calculate_distance(double lat1, double lon1, double lat2, double lon2, float *distance) {
    if (distance == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Validate coordinates
    if (validate_gps_coordinates(lat1, lon1) != RESULT_OK ||
        validate_gps_coordinates(lat2, lon2) != RESULT_OK) {
        return RESULT_ERR_INVALID_DATA;
    }
    
    // Haversine formula(to calculate distances on a sphere)
    double lat1_rad = lat1 * DEG_TO_RAD;
    double lat2_rad = lat2 * DEG_TO_RAD;
    double dlat = (lat2 - lat1) * DEG_TO_RAD;
    double dlon = (lon2 - lon1) * DEG_TO_RAD;
    
    double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
               cos(lat1_rad) * cos(lat2_rad) *
               sin(dlon / 2.0) * sin(dlon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    
    *distance = (float)(EARTH_RADIUS_M * c);
    
    return RESULT_OK;
}

result_t gps_calculate_bearing(double lat1, double lon1, double lat2, double lon2, float *bearing) {
    if (bearing == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Validate coordinates
    if (validate_gps_coordinates(lat1, lon1) != RESULT_OK ||
        validate_gps_coordinates(lat2, lon2) != RESULT_OK) {
        return RESULT_ERR_INVALID_DATA;
    }
    
    double lat1_rad = lat1 * DEG_TO_RAD;
    double lat2_rad = lat2 * DEG_TO_RAD;
    double dlon = (lon2 - lon1) * DEG_TO_RAD;
    
    double y = sin(dlon) * cos(lat2_rad);
    double x = cos(lat1_rad) * sin(lat2_rad) -
               sin(lat1_rad) * cos(lat2_rad) * cos(dlon);
    
    double bearing_rad = atan2(y, x);
    double bearing_deg = bearing_rad * RAD_TO_DEG;
    
    // Normalize to 0-360 degrees
    *bearing = (float)fmod((bearing_deg + 360.0), 360.0);
    
    return RESULT_OK;
}

result_t gps_sensor_merge_dual(gps_data_t *gps1, gps_data_t *gps2, gps_data_t *merged) {
    if (gps1 == NULL || gps2 == NULL || merged == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Initialize merged data
    gps_sensor_init(merged);
    
    // If only one sensor has valid data, use that one
    if (gps1->is_valid && !gps2->is_valid) {
        memcpy(merged, gps1, sizeof(gps_data_t));
        return RESULT_OK;
    }
    
    if (!gps1->is_valid && gps2->is_valid) {
        memcpy(merged, gps2, sizeof(gps_data_t));
        return RESULT_OK;
    }
    
    // If neither sensor has valid data, return invalid merged data
    if (!gps1->is_valid && !gps2->is_valid) {
        merged->is_valid = false;
        return RESULT_OK;
    }
    
    // Both sensors have valid data - use weighted average based on HDOP
    // Lower HDOP = better accuracy = higher weight
    float weight1 = 1.0f / (gps1->hdop + 0.1f);  // Add small value to avoid division by zero(don't want undef error)
    float weight2 = 1.0f / (gps2->hdop + 0.1f);
    float total_weight = weight1 + weight2;
    
    // Normalize weights
    weight1 /= total_weight;
    weight2 /= total_weight;
    
    // Weighted average of coordinates
    merged->latitude = gps1->latitude * weight1 + gps2->latitude * weight2;
    merged->longitude = gps1->longitude * weight1 + gps2->longitude * weight2;
    merged->altitude = gps1->altitude * weight1 + gps2->altitude * weight2;
    
    // Weighted average of velocity data
    merged->speed = gps1->speed * weight1 + gps2->speed * weight2;
    
    // For heading, handle circular averaging (0 and 360 are the same)
    double heading1_rad = gps1->heading * DEG_TO_RAD;
    double heading2_rad = gps2->heading * DEG_TO_RAD;
    double avg_x = cos(heading1_rad) * weight1 + cos(heading2_rad) * weight2;
    double avg_y = sin(heading1_rad) * weight1 + sin(heading2_rad) * weight2;
    merged->heading = (float)(atan2(avg_y, avg_x) * RAD_TO_DEG);
    if (merged->heading < 0) {
        merged->heading += 360.0f;
    }
    
    // Use better accuracy values (lower is better for DOP)
    merged->hdop = (gps1->hdop < gps2->hdop) ? gps1->hdop : gps2->hdop;
    merged->vdop = (gps1->vdop < gps2->vdop) ? gps1->vdop : gps2->vdop;
    
    // Use higher satellite count
    merged->satellites = (gps1->satellites > gps2->satellites) ? gps1->satellites : gps2->satellites;
    
    // Use better fix quality
    merged->fix_quality = (gps1->fix_quality > gps2->fix_quality) ? gps1->fix_quality : gps2->fix_quality;
    
    // Use more recent timestamp
    merged->utc_timestamp = (gps1->utc_timestamp > gps2->utc_timestamp) ? 
                            gps1->utc_timestamp : gps2->utc_timestamp;
    merged->timestamp = (gps1->timestamp > gps2->timestamp) ? 
                        gps1->timestamp : gps2->timestamp;
    
    merged->is_valid = true;
    
    return RESULT_OK;
}
