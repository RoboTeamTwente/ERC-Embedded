#include <unity.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "input_handler.h" /* your header */
#include "result.h"

/* ---------------- Compile-time checks (X-macro correctness) -------------- */

_Static_assert(INPUT_HANDLER_KEY_COUNT == 7, "Key count must match key list");

_Static_assert(INPUT_HANDLER_KEY_MAS(INPUT_KEY_UP) == (1u << 0),
               "Mask must match enum index");
_Static_assert(INPUT_HANDLER_KEY_MAS(INPUT_KEY_RESET) == (1u << 6),
               "Mask must match enum index");

/* ---------------- Helpers ------------------------------------------------ */

static QueueHandle_t g_evt_q;
static input_handler_t g_handler;

static void expect_no_event(void) {
  input_handler_event_t evt;
  result_t r = input_handler_recv(&g_handler, &evt, 0);
  TEST_ASSERT_EQUAL(RESULT_ERR_TIMEOUT, r);
}

static input_handler_event_t expect_event(input_handler_event_type_t type,
                                          input_handler_key_t key) {
  input_handler_event_t evt;
  result_t r = input_handler_recv(&g_handler, &evt, 0);
  TEST_ASSERT_EQUAL(RESULT_OK, r);
  TEST_ASSERT_EQUAL(type, evt.event_type);
  TEST_ASSERT_EQUAL(key, evt.key);
  return evt;
}

static void setup_handler(uint8_t debounce_samples, uint8_t enable_repeat,
                          uint16_t initial_delay_ms, uint16_t interval_ms,
                          uint8_t repeat_mask) {
  g_evt_q = xQueueCreate(16, sizeof(input_handler_event_t));
  TEST_ASSERT_NOT_NULL(g_evt_q);

  input_handler_config_t cfg = {
      .debounce_samples = debounce_samples,
      .enable_repeat = enable_repeat,
      .repeat_initial_delay_ms = initial_delay_ms,
      .repeat_interval_ms = interval_ms,
      .repeat_keys_mask = repeat_mask,
      .event_queue = g_evt_q,
  };

  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_init(&g_handler, &cfg));
  (void)input_handler_empty_event_queue(&g_handler);
}

void setUp(void) {
  /* Default: no repeat, debounce 2 samples */
  setup_handler(2, 0, 0, 0, 0);
}

void tearDown(void) {
  if (g_evt_q != NULL) {
    vQueueDelete(g_evt_q);
    g_evt_q = NULL;
  }
}

/* ---------------- Tests -------------------------------------------------- */

void test_debounce_press_requires_consecutive_samples(void) {
  const uint8_t up = (uint8_t)INPUT_HANDLER_KEY_MAS(INPUT_KEY_UP);

  /* initial: not pressed */
  TEST_ASSERT_EQUAL_UINT8(0u, input_handler_get_stable_mask(&g_handler));

  /* 1st sample says "pressed" -> should NOT accept yet (debounce=2) */
  TEST_ASSERT_EQUAL(RESULT_OK,
                    input_handler_process_sample(&g_handler, up, 10));
  TEST_ASSERT_EQUAL_UINT8(0u, input_handler_get_stable_mask(&g_handler));
  expect_no_event();

  /* 2nd consecutive sample still pressed -> accept press */
  TEST_ASSERT_EQUAL(RESULT_OK,
                    input_handler_process_sample(&g_handler, up, 20));
  TEST_ASSERT_EQUAL_UINT8(up, input_handler_get_stable_mask(&g_handler));
  (void)expect_event(INPUT_HANDLER_EVENT_KEY_PRESSED, INPUT_KEY_UP);

  /* No extra events if held and repeat disabled */
  TEST_ASSERT_EQUAL(RESULT_OK,
                    input_handler_process_sample(&g_handler, up, 30));
  expect_no_event();
}

void test_debounce_release_requires_consecutive_samples(void) {
  const uint8_t up = (uint8_t)INPUT_HANDLER_KEY_MAS(INPUT_KEY_UP);

  /* Get into pressed stable state */
  (void)input_handler_process_sample(&g_handler, up, 10);
  (void)input_handler_process_sample(&g_handler, up, 20);
  (void)expect_event(INPUT_HANDLER_EVENT_KEY_PRESSED, INPUT_KEY_UP);

  /* 1st sample says "released" -> should NOT accept yet */
  TEST_ASSERT_EQUAL(RESULT_OK,
                    input_handler_process_sample(&g_handler, 0u, 30));
  TEST_ASSERT_EQUAL_UINT8(up, input_handler_get_stable_mask(&g_handler));
  expect_no_event();

  /* 2nd consecutive release -> accept release */
  TEST_ASSERT_EQUAL(RESULT_OK,
                    input_handler_process_sample(&g_handler, 0u, 40));
  TEST_ASSERT_EQUAL_UINT8(0u, input_handler_get_stable_mask(&g_handler));
  (void)expect_event(INPUT_HANDLER_EVENT_KEY_RELEASED, INPUT_KEY_UP);
}

void test_bounce_does_not_trigger_press(void) {
  const uint8_t up = (uint8_t)INPUT_HANDLER_KEY_MAS(INPUT_KEY_UP);

  /* press (1st), bounce back (release), press again (1st) -> never 2
   * consecutive */
  TEST_ASSERT_EQUAL(RESULT_OK,
                    input_handler_process_sample(&g_handler, up, 10));
  TEST_ASSERT_EQUAL(RESULT_OK,
                    input_handler_process_sample(&g_handler, 0u, 20));
  TEST_ASSERT_EQUAL(RESULT_OK,
                    input_handler_process_sample(&g_handler, up, 30));

  TEST_ASSERT_EQUAL_UINT8(0u, input_handler_get_stable_mask(&g_handler));
  expect_no_event();
}

void test_repeat_generates_held_events_after_delay_and_interval(void) {
  /* Reconfigure: debounce=1 for easier timing, repeat enabled for UP only */
  tearDown();
  setup_handler(1, 1, 30, /* initial delay ms */
                20,       /* interval ms */
                (uint8_t)INPUT_HANDLER_KEY_MAS(INPUT_KEY_UP));

  const uint8_t up = (uint8_t)INPUT_HANDLER_KEY_MAS(INPUT_KEY_UP);

  /* Tick units: we assume tick values passed in are consistent with
   * pdMS_TO_TICKS inside impl. To avoid depending on configTICK_RATE_HZ here,
   * pass ticks in ms *and* in your impl you should be using pdMS_TO_TICKS(). If
   * your impl uses raw ticks directly, adjust these test tick values
   * accordingly. */

  /* Press: debounce=1 => immediate press event */
  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(&g_handler, up,
                                                            pdMS_TO_TICKS(0)));
  (void)expect_event(INPUT_HANDLER_EVENT_KEY_PRESSED, INPUT_KEY_UP);

  /* Before initial delay: no held */
  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(&g_handler, up,
                                                            pdMS_TO_TICKS(29)));
  expect_no_event();

  /* At/after initial delay: expect first HELD */
  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(&g_handler, up,
                                                            pdMS_TO_TICKS(30)));
  (void)expect_event(INPUT_HANDLER_EVENT_KEY_HELD, INPUT_KEY_UP);

  /* Next repeat at +20ms */
  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(&g_handler, up,
                                                            pdMS_TO_TICKS(49)));
  expect_no_event();

  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(&g_handler, up,
                                                            pdMS_TO_TICKS(50)));
  (void)expect_event(INPUT_HANDLER_EVENT_KEY_HELD, INPUT_KEY_UP);

  /* Release should stop repeat and emit release */
  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(&g_handler, 0u,
                                                            pdMS_TO_TICKS(60)));
  (void)expect_event(INPUT_HANDLER_EVENT_KEY_RELEASED, INPUT_KEY_UP);

  /* After release: no held */
  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(&g_handler, 0u,
                                                            pdMS_TO_TICKS(80)));
  expect_no_event();
}

void test_repeat_mask_blocks_nonrepeat_keys(void) {
  /* Repeat enabled, but only UP repeats (CENTER should NOT repeat) */
  tearDown();
  setup_handler(1, 1, 10, 10, (uint8_t)INPUT_HANDLER_KEY_MAS(INPUT_KEY_UP));

  const uint8_t mid = (uint8_t)INPUT_HANDLER_KEY_MAS(INPUT_KEY_MID);

  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(&g_handler, mid,
                                                            pdMS_TO_TICKS(0)));
  (void)expect_event(INPUT_HANDLER_EVENT_KEY_PRESSED, INPUT_KEY_MID);

  /* Even after long time, should not get HELD for MID */
  TEST_ASSERT_EQUAL(RESULT_OK, input_handler_process_sample(
                                   &g_handler, mid, pdMS_TO_TICKS(100)));
  expect_no_event();
}

void test_empty_event_queue_drains_all_events(void) {
  const uint8_t up = (uint8_t)INPUT_HANDLER_KEY_MAS(INPUT_KEY_UP);

  /* Create a press event */
  (void)input_handler_process_sample(&g_handler, up, 10);
  (void)input_handler_process_sample(&g_handler, up, 20);

  /* Drain should remove it */
  uint16_t n = input_handler_empty_event_queue(&g_handler);
  TEST_ASSERT_EQUAL_UINT16(1u, n);

  /* And queue is now empty */
  expect_no_event();
}

/* ---------------- Unity runner ------------------------------------------ */

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_debounce_press_requires_consecutive_samples);
  RUN_TEST(test_debounce_release_requires_consecutive_samples);
  RUN_TEST(test_bounce_does_not_trigger_press);
  RUN_TEST(test_repeat_generates_held_events_after_delay_and_interval);
  RUN_TEST(test_repeat_mask_blocks_nonrepeat_keys);
  RUN_TEST(test_empty_event_queue_drains_all_events);

  return UNITY_END();
}
