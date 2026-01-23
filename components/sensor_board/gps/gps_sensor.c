/**
 * @file gps_sensor.c
 * @brief Implementation of GPS sensor data handling functions.
 * 
 * @note Target Hardware: GY-NEO6MV2 NEO-6M GPS
 * @note Default UART: 9600 baud, 8N1
 * @note NMEA sentences parsed: GGA, RMC, GSA
 * 
 * NMEA Sentence Formats:
 * GGA - Global Positioning System Fix Data (position, altitude, fix quality)
 * RMC - Recommended Minimum Specific GPS Data (position, speed, heading)
 * GSA - GPS DOP and Active Satellites (HDOP, VDOP, fix mode)
 */

#include "gps_sensor.h"
#include <math.h>
#include <stdlib.h>
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

// Knots to meters per second conversion
#define KNOTS_TO_MPS 0.514444f

// Forward declarations for the internal NMEA parsing functions
static result_t parse_gga(gps_data_t *gps, const char *sentence);
static result_t parse_rmc(gps_data_t *gps, const char *sentence);
static result_t parse_gsa(gps_data_t *gps, const char *sentence);
static bool verify_checksum(const char *sentence);
static double parse_coordinate(const char *str, char direction);
static const char* get_field(const char *sentence, int field_num);
static float parse_float(const char *str);
static int parse_int(const char *str);

result_t gps_sensor_init(gps_data_t *gps) {
    if (gps == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    gps->latitude = 0.0;
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
    
    // Initialize NMEA parsing state
    memset(gps->rx_buffer, 0, GPS_RX_BUFFER_SIZE);
    gps->rx_index = 0;
    gps->sentence_ready = false;
    
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
    // TODO soon: Update gps->timestamp with HAL_GetTick() or similar
    
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

    // Process any complete NMEA sentences in the buffer
    // Data is received via UART interrupt (see gps_receive_byte)
    return gps_process_buffer(gps);
}

// =============================================================================
// NMEA Parsing Implementation for NEO-6M GPS
// =============================================================================

/**
 * @brief Parses an integer from a string.
 */
static int parse_int(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    return atoi(str);
}

/**
 * @brief Parses a float from a string.
 */
static float parse_float(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0.0f;
    }
    return (float)atof(str);
}

/**
 * @brief Gets a pointer to a specific field in an NMEA sentence.
 * @param sentence The NMEA sentence.
 * @param field_num The field number (0-based, where 0 is the sentence type).
 * @return Pointer to the start of the field, or NULL if not found.
 */
static const char* get_field(const char *sentence, int field_num) {
    if (sentence == NULL) {
        return NULL;
    }
    
    const char *ptr = sentence;
    int current_field = 0;
    
    // Skip the '$' if present
    if (*ptr == '$') {
        ptr++;
    }
    
    while (*ptr != '\0' && current_field < field_num) {
        if (*ptr == ',') {
            current_field++;
        }
        ptr++;
    }
    
    if (current_field == field_num) {
        return ptr;
    }
    
    return NULL;
}

/**
 * @brief Copies a field from an NMEA sentence to a buffer.
 */
static void copy_field(const char *sentence, int field_num, char *buffer, size_t buffer_size) {
    const char *field = get_field(sentence, field_num);
    if (field == NULL || buffer_size == 0) {
        if (buffer_size > 0) buffer[0] = '\0';
        return;
    }
    
    size_t i = 0;
    while (*field != ',' && *field != '*' && *field != '\r' && *field != '\n' && *field != '\0' && i < buffer_size - 1) {
        buffer[i++] = *field++;
    }
    buffer[i] = '\0';
}

/**
 * @brief Verifies the checksum of an NMEA sentence.
 * @param sentence The NMEA sentence (starting with '$').
 * @return true if checksum is valid, false otherwise.
 */
static bool verify_checksum(const char *sentence) {
    if (sentence == NULL || sentence[0] != '$') {
        return false;
    }
    
    uint8_t checksum = 0;
    const char *ptr = sentence + 1;  // Skip '$'
    
    // XOR all characters between '$' and '*'
    while (*ptr != '\0' && *ptr != '*') {
        checksum ^= (uint8_t)*ptr;
        ptr++;
    }
    
    // Check if we found '*'
    if (*ptr != '*') {
        return false;
    }
    
    // Parse the checksum from the sentence
    ptr++;  // Skip '*'
    char checksum_str[3] = {0};
    if (ptr[0] != '\0' && ptr[1] != '\0') {
        checksum_str[0] = ptr[0];
        checksum_str[1] = ptr[1];
    } else {
        return false;
    }
    
    uint8_t expected = (uint8_t)strtol(checksum_str, NULL, 16);
    return checksum == expected;
}

/**
 * @brief Parses NMEA coordinate format (DDDMM.MMMM) to decimal degrees.
 * @param str The coordinate string.
 * @param direction 'N', 'S', 'E', or 'W'.
 * @return Coordinate in decimal degrees.
 */
static double parse_coordinate(const char *str, char direction) {
    if (str == NULL || *str == '\0') {
        return 0.0;
    }
    
    double raw = atof(str);
    
    // Extract degrees and minutes
    // Format: DDDMM.MMMM (longitude) or DDMM.MMMM (latitude)
    int degrees = (int)(raw / 100);
    double minutes = raw - (degrees * 100);
    
    double decimal = degrees + (minutes / 60.0);
    
    // Apply direction
    if (direction == 'S' || direction == 'W') {
        decimal = -decimal;
    }
    
    return decimal;
}

/**
 * @brief Parses a GGA sentence (Global Positioning System Fix Data).
 * 
 * Format: $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
 * Fields:
 *   1: UTC time (hhmmss.ss)
 *   2: Latitude (llll.ll)
 *   3: N/S indicator
 *   4: Longitude (yyyyy.yy)
 *   5: E/W indicator
 *   6: Fix quality (0=invalid, 1=GPS fix, 2=DGPS fix)
 *   7: Number of satellites
 *   8: HDOP
 *   9: Altitude above sea level
 *   10: Altitude units (M)
 *   11: Geoidal separation
 *   12: Geoidal separation units
 */
static result_t parse_gga(gps_data_t *gps, const char *sentence) {
    char field[20];
    
    // Field 2: Latitude
    copy_field(sentence, 2, field, sizeof(field));
    char lat_str[20];
    strncpy(lat_str, field, sizeof(lat_str) - 1);
    
    // Field 3: N/S
    copy_field(sentence, 3, field, sizeof(field));
    char lat_dir = (field[0] != '\0') ? field[0] : 'N';
    
    // Field 4: Longitude
    copy_field(sentence, 4, field, sizeof(field));
    char lon_str[20];
    strncpy(lon_str, field, sizeof(lon_str) - 1);
    
    // Field 5: E/W
    copy_field(sentence, 5, field, sizeof(field));
    char lon_dir = (field[0] != '\0') ? field[0] : 'E';
    
    // Parse coordinates
    if (lat_str[0] != '\0' && lon_str[0] != '\0') {
        gps->latitude = parse_coordinate(lat_str, lat_dir);
        gps->longitude = parse_coordinate(lon_str, lon_dir);
    }
    
    // Field 6: Fix quality
    copy_field(sentence, 6, field, sizeof(field));
    int fix = parse_int(field);
    gps->fix_quality = (gps_fix_quality_t)fix;
    
    // Field 7: Number of satellites
    copy_field(sentence, 7, field, sizeof(field));
    gps->satellites = parse_int(field);
    
    // Field 8: HDOP
    copy_field(sentence, 8, field, sizeof(field));
    if (field[0] != '\0') {
        gps->hdop = parse_float(field);
    }
    
    // Field 9: Altitude
    copy_field(sentence, 9, field, sizeof(field));
    if (field[0] != '\0') {
        gps->altitude = parse_float(field);
    }
    
    // Update validity
    gps->is_valid = (gps->fix_quality >= GPS_GPS_FIX) && (gps->satellites >= 4);
    
    return RESULT_OK;
}

/**
 * @brief Parses an RMC sentence (Recommended Minimum Specific GPS Data).
 * 
 * Format: $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
 * Fields:
 *   1: UTC time
 *   2: Status (A=active, V=void)
 *   3: Latitude
 *   4: N/S
 *   5: Longitude
 *   6: E/W
 *   7: Speed over ground (knots)
 *   8: Course over ground (degrees)
 *   9: Date (ddmmyy)
 */
static result_t parse_rmc(gps_data_t *gps, const char *sentence) {
    char field[20];
    
    // Field 2: Status
    copy_field(sentence, 2, field, sizeof(field));
    if (field[0] != 'A') {
        // Data not valid
        gps->is_valid = false;
        return RESULT_OK;
    }
    
    // Field 3: Latitude
    copy_field(sentence, 3, field, sizeof(field));
    char lat_str[20];
    strncpy(lat_str, field, sizeof(lat_str) - 1);
    
    // Field 4: N/S
    copy_field(sentence, 4, field, sizeof(field));
    char lat_dir = (field[0] != '\0') ? field[0] : 'N';
    
    // Field 5: Longitude
    copy_field(sentence, 5, field, sizeof(field));
    char lon_str[20];
    strncpy(lon_str, field, sizeof(lon_str) - 1);
    
    // Field 6: E/W
    copy_field(sentence, 6, field, sizeof(field));
    char lon_dir = (field[0] != '\0') ? field[0] : 'E';
    
    // Parse coordinates
    if (lat_str[0] != '\0' && lon_str[0] != '\0') {
        gps->latitude = parse_coordinate(lat_str, lat_dir);
        gps->longitude = parse_coordinate(lon_str, lon_dir);
    }
    
    // Field 7: Speed (knots -> m/s)
    copy_field(sentence, 7, field, sizeof(field));
    if (field[0] != '\0') {
        gps->speed = parse_float(field) * KNOTS_TO_MPS;
    }
    
    // Field 8: Course/heading
    copy_field(sentence, 8, field, sizeof(field));
    if (field[0] != '\0') {
        gps->heading = parse_float(field);
    }
    
    return RESULT_OK;
}

/**
 * @brief Parses a GSA sentence (GPS DOP and Active Satellites).
 * 
 * Format: $GPGSA,a,x,xx,xx,...,xx,x.x,x.x,x.x*hh
 * Fields:
 *   1: Mode (M=manual, A=automatic)
 *   2: Fix type (1=no fix, 2=2D, 3=3D)
 *   3-14: PRN numbers of satellites used
 *   15: PDOP
 *   16: HDOP
 *   17: VDOP
 */
static result_t parse_gsa(gps_data_t *gps, const char *sentence) {
    char field[20];
    
    // Field 16: HDOP
    copy_field(sentence, 16, field, sizeof(field));
    if (field[0] != '\0') {
        gps->hdop = parse_float(field);
    }
    
    // Field 17: VDOP
    copy_field(sentence, 17, field, sizeof(field));
    if (field[0] != '\0') {
        gps->vdop = parse_float(field);
    }
    
    return RESULT_OK;
}

result_t gps_receive_byte(gps_data_t *gps, uint8_t byte) {
    if (gps == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Start of new sentence
    if (byte == '$') {
        gps->rx_index = 0;
        gps->rx_buffer[gps->rx_index++] = byte;
        return RESULT_OK;
    }
    
    // End of sentence
    if (byte == '\n') {
        if (gps->rx_index > 0 && gps->rx_index < GPS_RX_BUFFER_SIZE - 1) {
            gps->rx_buffer[gps->rx_index++] = byte;
            gps->rx_buffer[gps->rx_index] = '\0';
            gps->sentence_ready = true;
        }
        return RESULT_OK;
    }
    
    // Add byte to buffer
    if (gps->rx_index < GPS_RX_BUFFER_SIZE - 1) {
        gps->rx_buffer[gps->rx_index++] = byte;
        return RESULT_OK;
    }
    
    // Buffer overflow - reset
    gps->rx_index = 0;
    return RESULT_ERR_OVERFLOW;
}

result_t gps_parse_nmea(gps_data_t *gps, const char *sentence) {
    if (gps == NULL || sentence == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    // Verify sentence starts with '$'
    if (sentence[0] != '$') {
        return RESULT_ERR_BAD_FORMAT;
    }
    
    // Verify checksum (optional but recommended)
    if (!verify_checksum(sentence)) {
        return RESULT_ERR_BAD_FORMAT;
    }
    
    // Determine sentence type and parse
    if (strncmp(sentence + 3, "GGA", 3) == 0) {
        return parse_gga(gps, sentence);
    } else if (strncmp(sentence + 3, "RMC", 3) == 0) {
        return parse_rmc(gps, sentence);
    } else if (strncmp(sentence + 3, "GSA", 3) == 0) {
        return parse_gsa(gps, sentence);
    }
    
    // Unknown sentence type - not an error, just ignore
    return RESULT_OK;
}

result_t gps_process_buffer(gps_data_t *gps) {
    if (gps == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    if (gps->sentence_ready) {
        gps_parse_nmea(gps, gps->rx_buffer);
        gps->sentence_ready = false;
        gps->rx_index = 0;
    }
    
    return RESULT_OK;
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
    TRY(validate_gps_coordinates(lat1, lon1));
    TRY(validate_gps_coordinates(lat2, lon2));
    
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
    TRY(validate_gps_coordinates(lat1, lon1));
    TRY(validate_gps_coordinates(lat2, lon2));
    
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
