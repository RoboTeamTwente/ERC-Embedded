#include <unity.h>
#include "calculator.h" // The library we are testing

// setUp and tearDown are optional functions that run before/after each test
void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_function_add(void) {
    TEST_ASSERT_EQUAL_INT(5, add(2, 3));
    TEST_ASSERT_EQUAL_INT(-1, add(-4, 3));
}


// In native testing, the main function is the test runner
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_function_add);
    return UNITY_END();
}
