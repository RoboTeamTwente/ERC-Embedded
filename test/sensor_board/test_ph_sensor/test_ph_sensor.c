/**
 * Sensor used- PH meter SKU SEN0161
 * @file test_ph_sensor.c
 * @brief Unit tests for pH sensor functionality using Unity framework.
 */

#include "ph_sensor.h"
#include "logging.h"
#include <math.h>
#include <unity.h>

#define TAG "PH_TEST"

static ph_sensor_t sensor;
static const float reference_voltage = 3.3f;

void setUp(void) {
    ph_sensor_init(&sensor, reference_voltage);
}

void tearDown(void) {
}

void test_ph_sensor_init_sets_defaults(void) {
    LOGI(TAG, "Starting test: %s", __func__);
    TEST_ASSERT_EQUAL_UINT16(0u, sensor.raw_value);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sensor.voltage);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, sensor.ph_value);
    TEST_ASSERT_EQUAL_FLOAT(reference_voltage, sensor.reference_voltage);
}

void test_ph_sensor_update_converts_voltage_to_ph(void) {
    LOGI(TAG, "Starting test: %s", __func__);
    // This ADC value should result in a pH around 4.0
    const uint16_t adc_value = 2150u; 
    const uint16_t adc_max = 4095u;

    ph_sensor_update(&sensor, adc_value, adc_max);

    const float expected_voltage = (float)adc_value * reference_voltage / (float)adc_max;
    const float neutral_voltage = reference_voltage / 2.0f;
    const float voltage_diff_mv = (expected_voltage - neutral_voltage) * 1000.0f;
    const float expected_ph = 7.0f - (voltage_diff_mv / 59.16f);

    TEST_ASSERT_EQUAL_UINT16(adc_value, sensor.raw_value);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, expected_voltage, sensor.voltage);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_ph, sensor.ph_value);

    float ph_val;
    TEST_ASSERT_EQUAL(RESULT_OK, ph_sensor_get_value(&sensor, &ph_val));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_ph, ph_val);

    float voltage_val;
    TEST_ASSERT_EQUAL(RESULT_OK, ph_sensor_get_voltage(&sensor, &voltage_val));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, expected_voltage, voltage_val);
}

void test_ph_sensor_update_clamps_extremes(void) {
    const uint16_t adc_max = 4095u;

    ph_sensor_update(&sensor, 0u, adc_max);
    TEST_ASSERT_EQUAL_FLOAT(14.0f, sensor.ph_value);

    ph_sensor_update(&sensor, adc_max, adc_max);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, sensor.ph_value);
}

void test_ph_sensor_calibration_adjusts_measurement(void) {
    // This ADC value should result in a pH around 10.0
    const uint16_t adc_value = 1000u;
    const uint16_t adc_max = 4095u;
    // We expect a voltage of ~0.806V. Neutral is 1.65V. Difference is -0.844V.
    // To get a pH of 10, we need a voltage difference of (7-10) * 59.16mV = -177.48mV.
    // So we need an offset of -177.48 - (-844) = 666.52mV = 0.666V
    const float offset = 0.666f;
    const float slope = 59.16f;  // Nernstian slope

    ph_sensor_update(&sensor, adc_value, adc_max);
    const float baseline_ph = sensor.ph_value;

    ph_sensor_calibrate(&sensor, offset, slope);

    ph_sensor_update(&sensor, adc_value, adc_max);

    const float voltage = (float)adc_value * reference_voltage / (float)adc_max;
    const float neutral_voltage = reference_voltage / 2.0f;
    const float voltage_diff_mv = (voltage - neutral_voltage + offset) * 1000.0f;
    const float expected_ph = 7.0f - (voltage_diff_mv / slope);

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, sensor.ph_value);
    TEST_ASSERT_TRUE(fabsf(baseline_ph - sensor.ph_value) > 0.1f);
}

int main(void) {
    LOG_init(NULL);
    UNITY_BEGIN();
    RUN_TEST(test_ph_sensor_init_sets_defaults);
    RUN_TEST(test_ph_sensor_update_converts_voltage_to_ph);
    RUN_TEST(test_ph_sensor_update_clamps_extremes);
    RUN_TEST(test_ph_sensor_calibration_adjusts_measurement);
    return UNITY_END();
}
