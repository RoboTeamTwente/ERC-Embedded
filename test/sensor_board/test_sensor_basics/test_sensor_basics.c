#include "sensor_basics.h"
#include "unity.h"
#include "result.h"

void setUp(void) {}
void tearDown(void) {}

// void test_temperature_conversions(void) {
//     float fahrenheit, celsius;

//     result_t r1 = celsius_to_fahrenheit(0.0f, &fahrenheit);
//     TEST_ASSERT_EQUAL(RESULT_OK, r1);
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 32.0f, fahrenheit);

//     result_t r2 = celsius_to_fahrenheit(100.0f, &fahrenheit);
//     TEST_ASSERT_EQUAL(RESULT_OK, r2);
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 212.0f, fahrenheit);

//     result_t r3 = fahrenheit_to_celsius(32.0f, &celsius);
//     TEST_ASSERT_EQUAL(RESULT_OK, r3);
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, celsius);

//     result_t r4 = fahrenheit_to_celsius(212.0f, &celsius);
//     TEST_ASSERT_EQUAL(RESULT_OK, r4);
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, celsius);
// }

// void test_pressure_conversions(void) {
//     float psi, bar;

//     result_t r1 = bar_to_psi(1.0f, &psi);
//     TEST_ASSERT_EQUAL(RESULT_OK, r1);
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 14.5038f, psi);

//     result_t r2 = psi_to_bar(14.5038f, &bar);
//     TEST_ASSERT_EQUAL(RESULT_OK, r2);
//     TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, bar);
// //}

void test_validate_accelerometer_value_bounds(void) {
    result_t r1, r2, r3, r4;
    r1 = validate_accelerometer_value(160.0f);
    r2 = validate_accelerometer_value(-160.0f);
    r3 = validate_accelerometer_value(160.1f);
    r4 = validate_accelerometer_value(-160.1f);

    TEST_ASSERT_EQUAL(RESULT_OK, r1);
    TEST_ASSERT_EQUAL(RESULT_OK, r2);
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, r3);
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, r4);
}

void test_validate_imu_data_combines_axes(void) {
    result_t r1, r2;
    r1 = validate_imu_data(0.0f, 0.0f, 0.0f);
    r2 = validate_imu_data(0.0f, 0.0f, 200.0f);

    TEST_ASSERT_EQUAL(RESULT_OK, r1);
    TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_DATA, r2);
}

int main(void) {
    UNITY_BEGIN();
    // RUN_TEST(test_temperature_conversions);
    // RUN_TEST(test_pressure_conversions);
    RUN_TEST(test_validate_accelerometer_value_bounds);
    RUN_TEST(test_validate_imu_data_combines_axes);
    return UNITY_END();
}
