#include <assert.h>
#include <stdio.h>
#include "result.h"
#include "motor.h"
#include "logging.h"
#include <unity.h>

static motor_speed_t motors_speed[NUM_MOTORS_SPEED];
static motor_steering_t motors_steering[NUM_MOTORS_STEERING];
static driving_system_t driving_system;

void setUp(void) {
    driving_system_init(&driving_system);  // Unity calls setUp before each test
    motor_speed_init(motors_speed);
    motor_steering_init(motors_steering);
}

void tearDown(void) {
}

void test_driving_system_init(void) {
    driving_system_t ds = driving_system;
    TEST_ASSERT_EQUAL(0, ds.distance_to_go);
    TEST_ASSERT_EQUAL(0, ds.turning_radius);
    TEST_ASSERT_EQUAL(0, ds.turning_angle);
    TEST_ASSERT_EQUAL(0, ds.state);
}

void test_motor_speed_init(void) {
    for(int i=0; i < NUM_MOTORS_SPEED; i++){
        TEST_ASSERT_EQUAL(i, motors_speed[i].motor_id);
        TEST_ASSERT_EQUAL(0, motors_speed[i].actspeed);
        TEST_ASSERT_EQUAL(0, motors_speed[i].control_var);
    }
}

void test_motor_steering_init(void) {
    for(int i=0; i < NUM_MOTORS_STEERING; i++){
        TEST_ASSERT_EQUAL(i, motors_steering[i].motor_id);
        TEST_ASSERT_EQUAL(0, motors_steering[i].actangle);
        TEST_ASSERT_EQUAL(0, motors_steering[i].desang);
        TEST_ASSERT_EQUAL(0, motors_steering[i].pwnenable);
        TEST_ASSERT_EQUAL(0, motors_steering[i].pwmrev);
    }
}

void test_motor_speed_update_valid(void) {
    motor_speed_update(motors_speed, 1, 1.8f, 1.0f);

    TEST_ASSERT_EQUAL_FLOAT(1.8f, motors_speed[1].actspeed);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, motors_speed[1].control_var);
}

void test_motor_speed_update_invalid_index(void) {
    motor_speed_update(motors_speed, -1, 1.8f, 1.0f);// index below available
    motor_speed_update(motors_speed, 9, 2, 2); // index above what is available

    // Checks that motors remain initialized to 0
    for (int i = 0; i < NUM_MOTORS_STEERING; i++) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors_speed[i].actspeed);
    }
}

void test_motor_steering_update_valid(void) {
    motor_steering_update(motors_steering, 1, 1.8f, 1.0f, 0.5f, 1.2f);

    TEST_ASSERT_EQUAL_FLOAT(1.8f, motors_steering[1].actangle);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, motors_steering[1].desang);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, motors_steering[1].pwnenable);
    TEST_ASSERT_EQUAL_FLOAT(1.2f, motors_steering[1].pwmrev);
}

void test_motor_steering_update_invalid_index(void) {
    motor_steering_update(motors_steering, -1, 1.8f, 1.0f, 0.5f, 1.2f);// index below available
    motor_steering_update(motors_steering, 9, 2,2,2,2); // index above what is available

    // Checks that motors remain initialized to 0
    for (int i = 0; i < NUM_MOTORS_STEERING; i++) {
        TEST_ASSERT_EQUAL_FLOAT(0.0f, motors_steering[i].actangle);
    }
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_driving_system_init);
    RUN_TEST(test_motor_steering_update_invalid_index);
    RUN_TEST(test_motor_steering_update_valid);
    RUN_TEST(test_motor_speed_init);
    RUN_TEST(test_motor_steering_init);
    RUN_TEST(test_motor_speed_update_invalid_index);
    RUN_TEST(test_motor_speed_update_valid);
    return UNITY_END();
}
