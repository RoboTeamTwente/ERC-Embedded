#include "imu_sensor.h"
#include "logging.h"
#include "unity.h"
#include <math.h>
#include <string.h>

#define TAG "IMU_TEST"

static imu_data_t imu;

void setUp(void) {
    // Initialize the IMU sensor before each test
    imu_sensor_init(&imu);
}

void tearDown(void) {
    // Clean up after each test
}

void test_imu_sensor_init_sets_defaults(void) {
    LOGI(TAG, "Starting test: %s", __func__);
    imu_data_t imu_captured;
    imu_sensor_read(&imu, &imu_captured);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.accel[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.accel[1]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.accel[2]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.gyro[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.gyro[1]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.gyro[2]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.mag[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.mag[1]);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, imu_captured.mag[2]);
    TEST_ASSERT_EQUAL_UINT32(0, imu_captured.timestamp);

    TEST_ASSERT_TRUE(imu_validate_accelerometer_range(&imu));
    TEST_ASSERT_TRUE(imu_validate_gyroscope_range(&imu));
    TEST_ASSERT_TRUE(imu_validate_magnetometer_range(&imu));
}

void test_imu_sensor_update_and_read(void) {
    LOGI(TAG, "Starting test: %s", __func__);
    float ax = 1.0f, ay = 2.0f, az = 3.0f;
    float gx = 0.1f, gy = 0.2f, gz = 0.3f;
    float mx = 10.0f, my = 20.0f, mz = 30.0f;
    uint32_t timestamp = 12345;

    imu_sensor_update(&imu, ax, ay, az, gx, gy, gz, mx, my, mz, timestamp);

    imu_data_t imu_captured;
    imu_sensor_read(&imu, &imu_captured);

    TEST_ASSERT_EQUAL_FLOAT(ax, imu_captured.accel[0]);
    TEST_ASSERT_EQUAL_FLOAT(ay, imu_captured.accel[1]);
    TEST_ASSERT_EQUAL_FLOAT(az, imu_captured.accel[2]);
    TEST_ASSERT_EQUAL_FLOAT(gx, imu_captured.gyro[0]);
    TEST_ASSERT_EQUAL_FLOAT(gy, imu_captured.gyro[1]);
    TEST_ASSERT_EQUAL_FLOAT(gz, imu_captured.gyro[2]);
    TEST_ASSERT_EQUAL_FLOAT(mx, imu_captured.mag[0]);
    TEST_ASSERT_EQUAL_FLOAT(my, imu_captured.mag[1]);
    TEST_ASSERT_EQUAL_FLOAT(mz, imu_captured.mag[2]);
    TEST_ASSERT_EQUAL_UINT32(timestamp, imu_get_timestamp(&imu));
}

void test_imu_get_acceleration_magnitude(void) {
    imu_sensor_update(&imu, 3.0f, 4.0f, 12.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u);

    // sqrt(3^2 + 4^2 + 12^2) = sqrt(9 + 16 + 144) = sqrt(169) = 13
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 13.0f, imu_get_acceleration_magnitude(&imu));
}

void test_imu_orientation_helpers(void) {
    
    // Test case 1: Pitch 0, Roll 45
    imu_sensor_update(&imu, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u);
    // Pitch: atan2(0, sqrt(1*1 + 1*1)) = 0
    // Roll: atan2(1, 1) * 180/PI = 45 degrees
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, imu_get_pitch(&imu));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 45.0f, imu_get_roll(&imu));

    // Test case 2: Pitch 90, Roll 0
    imu_sensor_update(&imu, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u);
    // Pitch: atan2(1, 0) * 180/PI = 90 degrees
    // Roll: atan2(0, 1) = 0
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 90.0f, imu_get_pitch(&imu));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, imu_get_roll(&imu));
}
void test_imu_range_validators(void) {
    LOGI(TAG, "Starting test: %s", __func__);
    // Test valid ranges
    imu_sensor_update(&imu, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u);
    imu_sensor_update(&imu, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u);
    TEST_ASSERT_TRUE(imu_validate_accelerometer_range(&imu));
    TEST_ASSERT_TRUE(imu_validate_gyroscope_range(&imu));
    TEST_ASSERT_TRUE(imu_validate_magnetometer_range(&imu));

    // Test invalid accelerometer
    imu_sensor_update(&imu, 200.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u);
    TEST_ASSERT_FALSE(imu_validate_accelerometer_range(&imu));

    // Test invalid gyroscope
    imu_sensor_update(&imu, 0.0f, 0.0f, 0.0f, 2100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u);
    TEST_ASSERT_FALSE(imu_validate_gyroscope_range(&imu));

    // Test invalid magnetometer
    imu_sensor_update(&imu, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5000.0f, 0.0f, 0.0f, 0u);
    TEST_ASSERT_FALSE(imu_validate_magnetometer_range(&imu));
}

void test_imu_gyroscope_drift_check(void) {
    // Test drift below threshold
    imu_sensor_update(&imu, 0.0f, 0.0f, 0.0f, 0.5f, -0.4f, 0.3f, 0.0f, 0.0f, 0.0f, 0u);
    TEST_ASSERT_TRUE(imu_check_gyroscope_drift(&imu, 1.0f));

    // Testing drift above threshold
    imu_sensor_update(&imu, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0u);
    TEST_ASSERT_FALSE(imu_check_gyroscope_drift(&imu, 1.0f));
}

int main(void) {
    LOG_init(NULL);
    UNITY_BEGIN();
    RUN_TEST(test_imu_sensor_init_sets_defaults);
    RUN_TEST(test_imu_sensor_update_and_read);
    RUN_TEST(test_imu_get_acceleration_magnitude);
    RUN_TEST(test_imu_orientation_helpers);
    RUN_TEST(test_imu_range_validators);
    RUN_TEST(test_imu_gyroscope_drift_check);
    return UNITY_END();
}