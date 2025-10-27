#include <unity.h>

// setUp and tearDown are optional functions that run before/after each test
void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_function_add(void) { TEST_ASSERT_EQUAL_INT(5, 5); }

// In native testing, the main function is the test runner
int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_function_add);
  return UNITY_END();
}
