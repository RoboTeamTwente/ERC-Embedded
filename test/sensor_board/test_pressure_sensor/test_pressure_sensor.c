#include "pressure_sensor.h"
#include "unity.h"
#include "result.h"

void setUp(void) {}
void tearDown(void) {}

void test_pressure_sensor_init(void) {
    pressure_sensor_data_t pressure_data = {0};
    result_t result = pressure_sensor_init(&pressure_data);
    
    TEST_ASSERT_EQUAL(RESULT_OK, result);
    TEST_ASSERT_EQUAL(0.0f, pressure_data.pressure_kpa);
    TEST_ASSERT_EQUAL(0.0f, pressure_data.temperature_c);
    TEST_ASSERT_EQUAL(0.0f, pressure_data.voltage);
}

void test_pressure_sensor_get_pressure_kpa(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_data.pressure_kpa = 101.325f;
    
    float pressure = pressure_sensor_get_pressure_kpa(&pressure_data);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 101.325f, pressure);
}

void test_pressure_sensor_get_temperature_c(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_data.temperature_c = 25.0f;
    
    float temperature = pressure_sensor_get_temperature_c(&pressure_data);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.0f, temperature);
}

void test_pressure_sensor_get_voltage(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_data.voltage = 2.5f;
    
    float voltage = pressure_sensor_get_voltage(&pressure_data);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.5f, voltage);
}

void test_pressure_sensor_is_valid_calibrated(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_data.is_calibrated = true;
    
    bool valid = pressure_sensor_is_valid(&pressure_data);
    TEST_ASSERT_TRUE(valid);
}

void test_pressure_sensor_is_valid_uncalibrated(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_data.is_calibrated = false;
    
    bool valid = pressure_sensor_is_valid(&pressure_data);
    TEST_ASSERT_FALSE(valid);
}

void test_pressure_sensor_poll_returns_unimplemented(void) {
    pressure_sensor_data_t pressure_data = {0};
    result_t result = poll_pressure_sensor(&pressure_data);
    
    TEST_ASSERT_EQUAL(RESULT_ERR_UNIMPLEMENTED, result);
}

void test_pressure_sensor_atmospheric_conditions(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_sensor_init(&pressure_data);
    
    pressure_data.pressure_kpa = 101.325f;
    pressure_data.temperature_c = 15.0f;
    pressure_data.is_calibrated = true;
    
    TEST_ASSERT_TRUE(pressure_sensor_is_valid(&pressure_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 101.325f, pressure_sensor_get_pressure_kpa(&pressure_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 15.0f, pressure_sensor_get_temperature_c(&pressure_data));
}

void test_pressure_sensor_high_altitude_conditions(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_sensor_init(&pressure_data);
    
    pressure_data.pressure_kpa = 79.5f;
    pressure_data.temperature_c = 5.0f;
    pressure_data.is_calibrated = true;
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 79.5f, pressure_sensor_get_pressure_kpa(&pressure_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.0f, pressure_sensor_get_temperature_c(&pressure_data));
}

void test_pressure_sensor_deep_pressure_conditions(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_sensor_init(&pressure_data);
    
    pressure_data.pressure_kpa = 200.0f;
    pressure_data.temperature_c = 20.0f;
    pressure_data.voltage = 4.5f;
    pressure_data.is_calibrated = true;
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 200.0f, pressure_sensor_get_pressure_kpa(&pressure_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, pressure_sensor_get_temperature_c(&pressure_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 4.5f, pressure_sensor_get_voltage(&pressure_data));
}

void test_pressure_sensor_negative_temperature(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_sensor_init(&pressure_data);
    
    pressure_data.temperature_c = -10.0f;
    pressure_data.pressure_kpa = 101.325f;
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -10.0f, pressure_sensor_get_temperature_c(&pressure_data));
}

void test_pressure_sensor_zero_readings(void) {
    pressure_sensor_data_t pressure_data = {0};
    pressure_sensor_init(&pressure_data);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, pressure_sensor_get_pressure_kpa(&pressure_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, pressure_sensor_get_temperature_c(&pressure_data));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, pressure_sensor_get_voltage(&pressure_data));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_pressure_sensor_init);
    RUN_TEST(test_pressure_sensor_get_pressure_kpa);
    RUN_TEST(test_pressure_sensor_get_temperature_c);
    RUN_TEST(test_pressure_sensor_get_voltage);
    RUN_TEST(test_pressure_sensor_is_valid_calibrated);
    RUN_TEST(test_pressure_sensor_is_valid_uncalibrated);
    RUN_TEST(test_pressure_sensor_poll_returns_unimplemented);
    RUN_TEST(test_pressure_sensor_atmospheric_conditions);
    RUN_TEST(test_pressure_sensor_high_altitude_conditions);
    RUN_TEST(test_pressure_sensor_deep_pressure_conditions);
    RUN_TEST(test_pressure_sensor_negative_temperature);
    RUN_TEST(test_pressure_sensor_zero_readings);
    return UNITY_END();
}
