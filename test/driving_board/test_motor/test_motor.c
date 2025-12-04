#include <assert.h>
#include <stdio.h>
#include "unity.h"
#include "logging.h"
#include "motor.h"

static motor_t motors[NUM_MOTORS];

void setUp(void) {
    motor_init();  // Unity calls setUp before each test
}

void tearDown(void) {
}

void test_motor_init(void) {
    for (int i = 0; i < NUM_MOTORS; i++) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors[i].voltage);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors[i].angle_of_body_frame);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors[i].angular_momentum);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors[i].current);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors[i].rpm);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors[i].direction_vector_x);
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors[i].direction_vector_y);
    }
}

void test_motor_update_valid(void) {
    motor_update(motors, 1, 24.0f, 30.0f, 0.5f, 1.2f, 3000.0f, 0.1f, 0.2f);

    TEST_ASSERT_EQUAL_FLOAT(24.0f, motors[1].voltage);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, motors[1].angle_of_body_frame);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, motors[1].angular_momentum);
    TEST_ASSERT_EQUAL_FLOAT(1.2f, motors[1].current);
    TEST_ASSERT_EQUAL_FLOAT(3000.0f, motors[1].rpm);
    TEST_ASSERT_EQUAL_FLOAT(0.1f, motors[1].direction_vector_x);
    TEST_ASSERT_EQUAL_FLOAT(0.2f, motors[1].direction_vector_y);
}

void test_motor_update_invalid_index(void) {
    motor_update(motors, -1, 1,1,1,1,1,1,1);// index below available
    motor_update(motors, NUM_MOTORS, 2,2,2,2,2,2,2); // index above what is available

    // Checks that motors remain initialized to 0
    for (int i = 0; i < NUM_MOTORS; i++) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors[i].voltage);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_motor_init);
    RUN_TEST(test_motor_update_valid);
    RUN_TEST(test_motor_update_invalid_index);
    return UNITY_END();
}
