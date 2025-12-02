/**
 * Sensor used- GY-NEO6MV2 NEO-6M GPS
 * @file test_gps_sensor.c
 * @brief Unit tests for GPS sensor functionality using Unity framework.
 */

#include "gps_sensor.h"
#include "sensor_basics.h"
#include <unity.h>
#include <math.h>
#include <string.h>

// Test GPS sensors
static gps_data_t gps1, gps2;

void setUp(void) {
    gps_sensor_init(&gps1);
    gps_sensor_init(&gps2);
}

void tearDown(void) {
}

// Test GPS sensor initialization
void test_gps_sensor_init_sets_defaults(void) {
    gps_data_t gps;
    result_t result = gps_sensor_init(&gps);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, gps.latitude);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, gps.longitude);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, gps.altitude);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, gps.speed);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, gps.heading);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 99.9f, gps.hdop);    // Horizontal dilution of precision
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 99.9f, gps.vdop);    // Vertical dilution of precision
    TEST_ASSERT_EQUAL_INT32(0, gps.satellites);
    TEST_ASSERT_EQUAL(GPS_NO_FIX, gps.fix_quality);
    TEST_ASSERT_FALSE(gps.is_valid);
}

// Test GPS sensor update with valid data
void test_gps_sensor_update_valid_data(void) {
    // Update with valid GPS data (Enschede, Netherlands coordinates)
    result_t result = gps_sensor_update(&gps1, 
                                       52.2215,    // latitude
                                       6.8937,     // longitude
                                       65.0f,      // altitude
                                       5.5f,       // speed
                                       90.0f,      // heading
                                       1.2f,       // hdop
                                       1.5f,       // vdop
                                       8,          // satellites
                                       GPS_GPS_FIX,
                                       1700000000000LL);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 52.2215, gps1.latitude);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 6.8937, gps1.longitude);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 65.0f, gps1.altitude);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 5.5f, gps1.speed);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 90.0f, gps1.heading);
    TEST_ASSERT_EQUAL_INT32(8, gps1.satellites);
    TEST_ASSERT_EQUAL(GPS_GPS_FIX, gps1.fix_quality);
    TEST_ASSERT_TRUE(gps1.is_valid);
}

// Test GPS coordinate validation
void test_validate_gps_coordinates(void) {
    // Valid coordinates
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_coordinates(52.2215, 6.8937));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_coordinates(0.0, 0.0));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_coordinates(-90.0, -180.0));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_coordinates(90.0, 180.0));
    
    // Invalid coordinates
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_coordinates(91.0, 0.0));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_coordinates(-91.0, 0.0));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_coordinates(0.0, 181.0));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_coordinates(0.0, -181.0));
}

// Test GPS coordinate getters
void test_gps_sensor_get_coordinates(void) {
    gps_sensor_update(&gps1, 52.2215, 6.8937, 65.0f, 5.5f, 90.0f, 1.2f, 1.5f, 8, GPS_GPS_FIX, 0LL);
    
    double lat, lon;
    result_t result = gps_sensor_get_coordinates(&gps1, &lat, &lon);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 52.2215, lat);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, 6.8937, lon);
}

// Test GPS distance calculation (Haversine formula)
void test_gps_calculate_distance(void) {
    float distance;
    result_t result;
    
    // Distance from Enschede to Amsterdam (approximately 136 km)
    result = gps_calculate_distance(52.2215, 6.8937,  // Enschede
                                   52.3676, 4.9041,   // Amsterdam
                                   &distance);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    // Haversine formula gives straight-line distance
    TEST_ASSERT_FLOAT_WITHIN(2000.0f, 136274.0f, distance);
    
    // Distance from same point should be 0
    result = gps_calculate_distance(52.2215, 6.8937,
                                   52.2215, 6.8937,
                                   &distance);
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, distance);
}

// Test GPS bearing calculation
void test_gps_calculate_bearing(void) {
    float bearing;
    result_t result;
    
    // Bearing from Enschede to Amsterdam (approximately west-northwest)
    result = gps_calculate_bearing(52.2215, 6.8937,  // Enschede
                                  52.3676, 4.9041,   // Amsterdam
                                  &bearing);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    // Should be roughly 280-290 degrees (west-northwest)
    TEST_ASSERT_TRUE(bearing > 270.0f && bearing < 300.0f);
    
    // Bearing due east
    result = gps_calculate_bearing(0.0, 0.0, 0.0, 1.0, &bearing);
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 90.0f, bearing);
}

// Test GPS sensor read
void test_gps_sensor_read(void) {
    gps_data_t copy;
    gps_sensor_update(&gps1, 52.2215, 6.8937, 65.0f, 5.5f, 90.0f, 1.2f, 1.5f, 8, GPS_GPS_FIX, 123456789LL);
    
    result_t result = gps_sensor_read(&gps1, &copy);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, gps1.latitude, copy.latitude);
    TEST_ASSERT_DOUBLE_WITHIN(0.0001, gps1.longitude, copy.longitude);
    TEST_ASSERT_EQUAL_INT32(gps1.satellites, copy.satellites);
    TEST_ASSERT_EQUAL(gps1.is_valid, copy.is_valid);
}

// Test dual GPS sensor merging
void test_gps_sensor_merge_dual(void) {
    gps_data_t merged;
    gps_sensor_init(&merged);
    
    // Both sensors have valid data with slightly different readings
    gps_sensor_update(&gps1, 52.2215, 6.8937, 65.0f, 5.5f, 90.0f, 1.0f, 1.5f, 8, GPS_GPS_FIX, 0LL);
    gps_sensor_update(&gps2, 52.2218, 6.8940, 66.0f, 5.3f, 92.0f, 1.5f, 1.8f, 7, GPS_DGPS_FIX, 0LL);
    
    result_t result = gps_sensor_merge_dual(&gps1, &gps2, &merged);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_TRUE(merged.is_valid);
    
    // Merged coordinates should be between the two readings
    TEST_ASSERT_TRUE(merged.latitude > 52.2215 && merged.latitude < 52.2218);
    TEST_ASSERT_TRUE(merged.longitude > 6.8937 && merged.longitude < 6.8940);
    
    // Should use better HDOP (lower is better)
    TEST_ASSERT_EQUAL_FLOAT(1.0f, merged.hdop);
    
    // Should use better fix quality
    TEST_ASSERT_EQUAL(GPS_DGPS_FIX, merged.fix_quality);
    
    // Should use higher satellite count
    TEST_ASSERT_EQUAL_INT32(8, merged.satellites);
}

// Test GPS validity check
void test_gps_sensor_is_valid(void) {
    bool is_valid;
    
    // Should be invalid after init
    result_t result = gps_sensor_is_valid(&gps1, &is_valid);
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_FALSE(is_valid);
    
    // Should be valid after good update
    gps_sensor_update(&gps1, 52.2215, 6.8937, 65.0f, 5.5f, 90.0f, 1.0f, 1.5f, 8, GPS_GPS_FIX, 0LL);
    result = gps_sensor_is_valid(&gps1, &is_valid);
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_TRUE(is_valid);
}

// Test sensor_basics GPS validation functions
void test_sensor_basics_gps_validation(void) {
    // Latitude validation
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_latitude(52.2215));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_latitude(0.0));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_latitude(-90.0));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_latitude(90.0));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_latitude(91.0));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_latitude(-91.0));
    
    // Longitude validation
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_longitude(6.8937));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_longitude(0.0));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_longitude(-180.0));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_longitude(180.0));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_longitude(181.0));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_longitude(-181.0));
    
    // HDOP validation
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_hdop(1.0f));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_hdop(0.0f));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_hdop(50.0f));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_hdop(-1.0f));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_hdop(51.0f));
    
    // Satellite count validation
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_satellite_count(8));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_satellite_count(0));
    TEST_ASSERT_EQUAL(RESULT_OK, validate_gps_satellite_count(30));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_satellite_count(-1));
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, validate_gps_satellite_count(31));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_gps_sensor_init_sets_defaults);
    RUN_TEST(test_gps_sensor_update_valid_data);
    RUN_TEST(test_validate_gps_coordinates);
    RUN_TEST(test_gps_sensor_get_coordinates);
    RUN_TEST(test_gps_calculate_distance);
    RUN_TEST(test_gps_calculate_bearing);
    RUN_TEST(test_gps_sensor_read);
    RUN_TEST(test_gps_sensor_merge_dual);
    RUN_TEST(test_gps_sensor_is_valid);
    RUN_TEST(test_sensor_basics_gps_validation);
    
    return UNITY_END();
}
