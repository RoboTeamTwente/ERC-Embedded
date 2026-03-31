#include "load_cell_sensor.h"
#include "unity.h"
#include "result.h"

void setUp(void) {}
void tearDown(void) {}

void test_load_cell_sensor_init(void) {
    load_cell_data_t load_data = {0};
    result_t result = load_cell_sensor_init(&load_data);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_EQUAL(0, load_data.raw_adc_counts);
    TEST_ASSERT_EQUAL(0.0f, load_data.force_newtons);
    TEST_ASSERT_EQUAL(0.0f, load_data.mass_grams);
}

void test_load_cell_get_raw_counts(void) {
    load_cell_data_t load_data = {0};
    load_data.raw_adc_counts = 1024;
    
    uint32_t counts = load_cell_get_raw_counts(&load_data);
    TEST_ASSERT_EQUAL(1024, counts);
}

void test_load_cell_get_force_newtons(void) {
    load_cell_data_t load_data = {0};
    load_data.force_newtons = 98.1f;
    
    float force = load_cell_get_force_newtons(&load_data);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 98.1f, force);
}

void test_load_cell_get_mass_grams(void) {
    load_cell_data_t load_data = {0};
    load_data.mass_grams = 1000.0f;
    
    float mass = load_cell_get_mass_grams(&load_data);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1000.0f, mass);
}

void test_load_cell_get_calibration(void) {
    load_cell_data_t load_data = {0};
    load_data.calibration_scale = 0.00488f;
    load_data.tare_offset = 512;
    
    float scale = load_cell_get_calibration(&load_data, NULL);
    TEST_ASSERT_FLOAT_WITHIN(0.00001f, 0.00488f, scale);
    
    uint32_t tare = 0;
    load_cell_get_calibration(&load_data, &tare);
    TEST_ASSERT_EQUAL(512, tare);
}

void test_load_cell_sensor_is_valid_calibrated(void) {
    load_cell_data_t load_data = {0};
    load_data.is_calibrated = true;
    
    bool valid = load_cell_sensor_is_valid(&load_data);
    TEST_ASSERT_TRUE(valid);
}

void test_load_cell_sensor_is_valid_uncalibrated(void) {
    load_cell_data_t load_data = {0};
    load_data.is_calibrated = false;
    
    bool valid = load_cell_sensor_is_valid(&load_data);
    TEST_ASSERT_FALSE(valid);
}

void test_load_cell_poll_sensor_returns_unimplemented(void) {
    load_cell_data_t load_data = {0};
    result_t result = poll_load_cell_sensor(&load_data);
    
    TEST_ASSERT_EQUAL(RESULT_ERR_UNIMPLEMENTED, result);
}

void test_load_cell_multiple_measurements(void) {
    load_cell_data_t load_data = {0};
    
    load_cell_sensor_init(&load_data);
    
    load_data.force_newtons = 50.0f;
    load_data.mass_grams = 500.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, load_cell_get_force_newtons(&load_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 500.0f, load_cell_get_mass_grams(&load_data));
    
    load_data.force_newtons = 100.0f;
    load_data.mass_grams = 1000.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, load_cell_get_force_newtons(&load_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1000.0f, load_cell_get_mass_grams(&load_data));
}

void test_load_cell_zero_force_and_mass(void) {
    load_cell_data_t load_data = {0};
    
    load_cell_sensor_init(&load_data);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, load_cell_get_force_newtons(&load_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, load_cell_get_mass_grams(&load_data));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_load_cell_sensor_init);
    RUN_TEST(test_load_cell_get_raw_counts);
    RUN_TEST(test_load_cell_get_force_newtons);
    RUN_TEST(test_load_cell_get_mass_grams);
    RUN_TEST(test_load_cell_get_calibration);
    RUN_TEST(test_load_cell_sensor_is_valid_calibrated);
    RUN_TEST(test_load_cell_sensor_is_valid_uncalibrated);
    RUN_TEST(test_load_cell_poll_sensor_returns_unimplemented);
    RUN_TEST(test_load_cell_multiple_measurements);
    RUN_TEST(test_load_cell_zero_force_and_mass);
    return UNITY_END();
}
