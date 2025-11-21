#include "calculator.h" // The library we are testing
#include "cubemx_main.h"
#include "logging.h"
#include "test/driving_board/unity_config.h"
#include <stdint.h>
#include <unity.h>

extern UART_HandleTypeDef huart_com;
extern void SystemClock_Config(void);
// setUp and tearDown are optional functions that run before/after each test
void setUp(void) {
  SystemClock_Config();
  LOG_init(&huart_com);
}

void tearDown(void) {
  // clean stuff up here
}

void test_function_add(void) {
  LOGI("TEST", "INFO");
  TEST_ASSERT_EQUAL_INT(6, add(2, 3));
  TEST_ASSERT_EQUAL_INT(-1, add(-4, 3)); 

  HAL_Delay(500);

}

void test_function_add2(void) {
  LOGI("TEST", "INFO2");
  TEST_ASSERT_EQUAL_INT(5, add(2, 3));
  TEST_ASSERT_EQUAL_INT(-1, add(-4, 3)); 

  HAL_Delay(500);

}

// In native testing, the main function is the test runner
int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_function_add);
  RUN_TEST(test_function_add2);

  return UNITY_END();
}
