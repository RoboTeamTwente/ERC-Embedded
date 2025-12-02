/**
 * @file gps_sensor.h
 * @brief Header file for GPS sensor data handling.
 *
 * This file defines the data structures for holding GPS sensor data and declares
 * functions for initializing, reading, and processing GPS sensor values.
 * Supports dual GPS module configuration for redundancy and accuracy.
 * 
 * @note Hardware: GY-NEO6MV2 NEO-6M GPS
 */

#ifndef GPS_SENSOR_H
#define GPS_SENSOR_H

#include "result.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief GPS fix quality enumeration.
 */
typedef enum {
    GPS_NO_FIX = 0,        /** No GPS fix available. */
    GPS_GPS_FIX = 1,       /** Standard GPS fix. */
    GPS_DGPS_FIX = 2,      /** Differential GPS fix. */
    GPS_PPS_FIX = 3,       /** PPS (Pulse Per Second) fix. */
    GPS_RTK_FIX = 4,       /** Real-Time Kinematic fix. */
    GPS_RTK_FLOAT = 5      /** RTK float solution. */
} gps_fix_quality_t;

/**
 * @brief Structure to hold GPS sensor data.
 */
typedef struct {
    // GPS coordinates
    double latitude;           /**< Latitude in degrees (positive = North, negative = South). */
    double longitude;          /**< Longitude in degrees (positive = East, negative = West). */
    float altitude;            /**< Altitude in meters above sea level. */
    
    // Velocity data
    float speed;               /**< Speed in meters per second. */
    float heading;             /**< Course over ground in degrees (0-360). */
    
    // Position accuracy and quality
    float hdop;                /**< Horizontal dilution of precision. */
    float vdop;                /**< Vertical dilution of precision. */
    int32_t satellites;        /**< Number of satellites in view. */
    gps_fix_quality_t fix_quality; /**< GPS fix quality indicator. */
    
    // Timestamp
    int64_t utc_timestamp;     /**< UTC timestamp in milliseconds since Unix epoch. */
    
    // Sensor status
    bool is_valid;             /**< Whether the GPS data is valid. */
    uint32_t timestamp;        /**< System timestamp of the last sensor reading. */
    uint32_t last_timestamp;   /**< System timestamp of the previous sensor reading. */
} gps_data_t;

/**
 * @brief Initializes the GPS sensor data structure with default values.
 * @param gps Pointer to the gps_data_t structure to initialize.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if gps is NULL.
 */
result_t gps_sensor_init(gps_data_t *gps);

/**
 * @brief Updates the GPS sensor data with new raw values.
 *
 * @param gps Pointer to the GPS data structure.
 * @param latitude Latitude in degrees.
 * @param longitude Longitude in degrees.
 * @param altitude Altitude in meters.
 * @param speed Speed in meters per second.
 * @param heading Course over ground in degrees.
 * @param hdop Horizontal dilution of precision.
 * @param vdop Vertical dilution of precision.
 * @param satellites Number of satellites in view.
 * @param fix_quality GPS fix quality.
 * @param utc_timestamp UTC timestamp in milliseconds.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if gps is NULL.
 */
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
                           int64_t utc_timestamp);

/**
 * @brief Polls the GPS sensor for new data and updates the data structure.
 * 
 * This function should contain the hardware-specific code to read from
 * the GPS module (e.g., via UART) and parse the data (e.g., NMEA sentences).
 * 
 * @param gps Pointer to the gps_data_t structure to update.
 * @return RESULT_OK on success, or an error code if fetching data fails.
 */
result_t poll_gps_sensor(gps_data_t *gps);

/**
 * @brief Reads the current GPS data.
 * @param gps Pointer to the source gps_data_t structure.
 * @param data Pointer to the destination gps_data_t structure to copy data into.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if any pointer is NULL.
 */
result_t gps_sensor_read(gps_data_t *gps, gps_data_t *data);

/**
 * @brief Gets the current GPS coordinates.
 * @param gps Pointer to the gps_data_t structure.
 * @param latitude Pointer to store the latitude value.
 * @param longitude Pointer to store the longitude value.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if any pointer is NULL.
 */
result_t gps_sensor_get_coordinates(gps_data_t *gps, double *latitude, double *longitude);

/**
 * @brief Gets the GPS altitude.
 * @param gps Pointer to the gps_data_t structure.
 * @param altitude Pointer to store the altitude value.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if any pointer is NULL.
 */
result_t gps_sensor_get_altitude(gps_data_t *gps, float *altitude);

/**
 * @brief Gets the GPS velocity data.
 * @param gps Pointer to the gps_data_t structure.
 * @param speed Pointer to store the speed value.
 * @param heading Pointer to store the heading value.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if any pointer is NULL.
 */
result_t gps_sensor_get_velocity(gps_data_t *gps, float *speed, float *heading);

/**
 * @brief Gets the GPS fix quality and satellite count.
 * @param gps Pointer to the gps_data_t structure.
 * @param fix_quality Pointer to store the fix quality value.
 * @param satellites Pointer to store the satellite count.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if any pointer is NULL.
 */
result_t gps_sensor_get_fix_info(gps_data_t *gps, gps_fix_quality_t *fix_quality, int32_t *satellites);

/**
 * @brief Checks if the GPS data is valid and has a fix.
 * @param gps Pointer to the gps_data_t structure.
 * @param is_valid Pointer to store the validity status.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if any pointer is NULL.
 */
result_t gps_sensor_is_valid(gps_data_t *gps, bool *is_valid);

/**
 * @brief Validates GPS coordinates (basic range check).
 * @param latitude Latitude to validate.
 * @param longitude Longitude to validate.
 * @return RESULT_OK if coordinates are valid, RESULT_ERR_INVALID_DATA otherwise.
 */
result_t validate_gps_coordinates(double latitude, double longitude);

/**
 * @brief Calculates the distance between two GPS coordinates using Haversine formula.
 * @param lat1 Latitude of the first point in degrees.
 * @param lon1 Longitude of the first point in degrees.
 * @param lat2 Latitude of the second point in degrees.
 * @param lon2 Longitude of the second point in degrees.
 * @param distance Pointer to store the calculated distance in meters.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if distance is NULL.
 */
result_t gps_calculate_distance(double lat1, double lon1, double lat2, double lon2, float *distance);

/**
 * @brief Calculates the bearing between two GPS coordinates.
 * @param lat1 Latitude of the first point in degrees.
 * @param lon1 Longitude of the first point in degrees.
 * @param lat2 Latitude of the second point in degrees.
 * @param lon2 Longitude of the second point in degrees.
 * @param bearing Pointer to store the calculated bearing in degrees (0-360).
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if bearing is NULL.
 */
result_t gps_calculate_bearing(double lat1, double lon1, double lat2, double lon2, float *bearing);

/**
 * @brief Merges data from two GPS sensors for improved accuracy.
 * 
 * Uses weighted average based on HDOP values and fix quality to combine
 * data from two GPS modules for better position accuracy.
 * 
 * @param gps1 Pointer to the first GPS data structure.
 * @param gps2 Pointer to the second GPS data structure.
 * @param merged Pointer to store the merged GPS data.
 * @return RESULT_OK on success, or RESULT_ERR_INVALID_ARG if any pointer is NULL.
 */
result_t gps_sensor_merge_dual(gps_data_t *gps1, gps_data_t *gps2, gps_data_t *merged);

#endif // GPS_SENSOR_H
