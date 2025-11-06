#include "lock_logic.h" // The library we are testing
#include "logging.h"
#include <unity.h>

lock_t* lock;
static uint16_t angle_close = 0; 
static uint16_t angle_open = 180; 
static uint16_t min_pw = 1000; 
static uint16_t max_pw = 2000; 

// setUp and tearDown are optional functions that run before/after each test
void setUp(void) {
    // set stuff up here
    lock_init(lock, angle_close, angle_open, min_pw, max_pw);
}

void tearDown(void) {
    // clean stuff up here
}

void test_function_open_lock(void) {
    lock_open(lock);
    TEST_ASSERT_EQUAL_INT(lock->servo.pos, 180);
    TEST_ASSERT_EQUAL_INT(lock->servo.pulse_width, 2000);
}

void test_function_close_lock(void) {
    lock_open(lock);
    lock_close(lock);
    TEST_ASSERT_EQUAL_INT(lock->servo.pos, 0);
    TEST_ASSERT_EQUAL_INT(lock->servo.pulse_width, 1000);
}

// In native testing, the main function is the test runner
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_open_lock);
    RUN_TEST(test_function_close_lock);
    return UNITY_END();
}