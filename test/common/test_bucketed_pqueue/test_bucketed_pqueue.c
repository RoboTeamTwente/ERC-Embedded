#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <unity.h>

#include "bucketed_pqueue.h"

static bucketed_pqueue_t pq;
static QueueHandle_t buckets[3];

static void setUp_pq(void) {
  buckets[0] = xQueueCreate(4, sizeof(uint32_t));
  buckets[1] = xQueueCreate(4, sizeof(uint32_t));
  buckets[2] = xQueueCreate(4, sizeof(uint32_t));
  TEST_ASSERT_NOT_NULL(buckets[0]);
  TEST_ASSERT_NOT_NULL(buckets[1]);
  TEST_ASSERT_NOT_NULL(buckets[2]);

  TEST_ASSERT_EQUAL(
      RESULT_OK,
      bucketed_pqueue_init(&pq, buckets, 3, xTaskGetCurrentTaskHandle()));
}

void setUp(void) { setUp_pq(); }

void tearDown(void) {
  vQueueDelete(buckets[0]);
  vQueueDelete(buckets[1]);
  vQueueDelete(buckets[2]);
}

void test_pop_prefers_highest_priority(void) {
  uint32_t a = 1, b = 2, c = 3;
  uint32_t out = 0;

  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_push(&pq, 0, &a, 0));

  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_push(&pq, 2, &c, 0));
  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_push(&pq, 1, &b, 0));
  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_pop(&pq, &out));
  TEST_ASSERT_EQUAL_UINT32(c, out);

  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_pop(&pq, &out));
  TEST_ASSERT_EQUAL_UINT32(b, out);

  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_pop(&pq, &out));
  TEST_ASSERT_EQUAL_UINT32(a, out);
}

void test_peek_does_not_remove(void) {
  uint32_t v = 0xDEADBEEF;
  uint32_t out = 0;

  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_push(&pq, 1, &v, 0));

  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_peek(&pq, &out));
  TEST_ASSERT_EQUAL_UINT32(v, out);

  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_pop(&pq, &out));
  TEST_ASSERT_EQUAL_UINT32(v, out);
}

void test_non_empty_flag(void) {
  uint32_t a = 1, b = 2, c = 3;
  uint32_t out = 0;
  uint8_t mask = 0;
  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_push(&pq, 0, &a, 0));
  mask = 0b00000001;
  TEST_ASSERT_EQUAL_UINT8(mask, pq.non_empty_mask);
  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_push(&pq, 2, &c, 0));
  mask = 0b00000101;
  TEST_ASSERT_EQUAL_UINT8(mask, pq.non_empty_mask);
  TEST_ASSERT_EQUAL(RESULT_OK, bucketed_pqueue_pop(&pq, &out));
  TEST_ASSERT_EQUAL_UINT32(c, out);
  mask = 0b00000001;
  TEST_ASSERT_EQUAL_UINT8(mask, pq.non_empty_mask);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_pop_prefers_highest_priority);
  RUN_TEST(test_peek_does_not_remove);
  RUN_TEST(test_non_empty_flag);

  return UNITY_END();
}
