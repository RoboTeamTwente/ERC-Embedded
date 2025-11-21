#include "scale.h" // The library we are testing
#include "logging.h"
#include "result.h"
#include <unity.h>
#include <stdbool.h>

float current_weight_on_scale = 0;
float FAKE_OFFSET = 5;
float FAKE_CALIBRATION_FACTOR = 1.2;
scale_t scale = {false, 0};
scale_t* scale_ptr = &scale;

// setUp and tearDown are optional functions that run before/after each test
void setUp(void) {
    // set stuff up here
    scale_init(scale_ptr);
}

void test_function_init(void) {
    TEST_ASSERT_EQUAL_INT(scale.isOn, true);
    //after init, tared weight should be 0 since we account for the offset
    float res_weight; //var to store result
    scale_read_tared(scale_ptr, &res_weight);
    TEST_ASSERT_EQUAL_INT(res_weight, 0);
}

void test_function_tare(void) {

}

void test_pipeline(void) {
    //after init, you should have to calibrate first before being able to read weight
    float res_weight; //var to store result
    TEST_ASSERT_EQUAL(scale_read_weight(scale_ptr, &res_weight), RESULT_ERR_NOT_INITIALIZED);

    //to calibrate place a known weight on the scale
    float known_weight = 10;
    float tared_weight; //var to store result
    scale.raw_data = known_weight*FAKE_CALIBRATION_FACTOR + FAKE_OFFSET;
    scale_read_tared(scale_ptr, &tared_weight);
    scale_calibrate(scale_ptr, tared_weight, known_weight);
}

void tearDown(void) {
    // clean stuff up here
}

// In native testing, the main function is the test runner
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_init);
    RUN_TEST(test_function_tare);
    RUN_TEST(test_pipeline);
    return UNITY_END();
}