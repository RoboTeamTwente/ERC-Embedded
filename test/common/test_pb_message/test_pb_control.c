
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unity.h>

#include "pb_control.h"
#include "result.h"

/* -------------------------------------------------------------------------- */
/* Test reset hook (preferred)                                                */
/* -------------------------------------------------------------------------- */
#ifdef UNIT_TEST
void pb_control_test_reset(void);
#endif

/* If you do NOT have a test reset hook, and the module globals are visible to
 * this TU, uncomment these externs and remove pb_control_test_reset usage.
 *
 * NOTE: This only works if DispatchTable/DispachTableSize are NOT static in the
 * module .c, or if you compile the .c into this TU (not recommended).
 */
/*
extern handler_entry_t DispatchTable[MAX_PROCESSABLE_PACKET_TYPES];
extern uint16_t DispachTableSize;
*/

/* -------------------------------------------------------------------------- */
/* Tracker style (as requested)                                               */
/* -------------------------------------------------------------------------- */
typedef struct {
  int call_count;
  uint8_t *last_pointer;
  size_t last_size;
} callback_tracker_t;

static callback_tracker_t trackers[15];
static packet_handler_t handler_array[15];
static uint16_t packet_type_array[15];

/* -------------------------------------------------------------------------- */
/* Distinct handlers: each tied to its own tracker */
/* This keeps your tracker style but makes wrong-dispatch detectable. */
/* -------------------------------------------------------------------------- */
#define DEFINE_TRACKED_HANDLER(N)                                              \
  static result_t test_handler_##N(const uint8_t *packet_data,                 \
                                   size_t packet_size) {                       \
    trackers[N].call_count++;                                                  \
    trackers[N].last_pointer = (uint8_t *)packet_data;                         \
    trackers[N].last_size = packet_size;                                       \
    return RESULT_OK;                                                          \
  }

DEFINE_TRACKED_HANDLER(0)
DEFINE_TRACKED_HANDLER(1)
DEFINE_TRACKED_HANDLER(2)
DEFINE_TRACKED_HANDLER(3)
DEFINE_TRACKED_HANDLER(4)
DEFINE_TRACKED_HANDLER(5)
DEFINE_TRACKED_HANDLER(6)
DEFINE_TRACKED_HANDLER(7)
DEFINE_TRACKED_HANDLER(8)
DEFINE_TRACKED_HANDLER(9)
DEFINE_TRACKED_HANDLER(10)
DEFINE_TRACKED_HANDLER(11)
DEFINE_TRACKED_HANDLER(12)
DEFINE_TRACKED_HANDLER(13)
DEFINE_TRACKED_HANDLER(14)

static void fill_handler_array(void) {
  handler_array[0] = test_handler_0;
  handler_array[1] = test_handler_1;
  handler_array[2] = test_handler_2;
  handler_array[3] = test_handler_3;
  handler_array[4] = test_handler_4;
  handler_array[5] = test_handler_5;
  handler_array[6] = test_handler_6;
  handler_array[7] = test_handler_7;
  handler_array[8] = test_handler_8;
  handler_array[9] = test_handler_9;
  handler_array[10] = test_handler_10;
  handler_array[11] = test_handler_11;
  handler_array[12] = test_handler_12;
  handler_array[13] = test_handler_13;
  handler_array[14] = test_handler_14;
}

static void reset_trackers(void) {
  for (int i = 0; i < 15; i++) {
    trackers[i].call_count = 0;
    trackers[i].last_pointer = NULL;
    trackers[i].last_size = 0;
  }
}

static void reset_module_state(void) {
#ifdef UNIT_TEST
  pb_control_test_reset();
#else
  /* If you don't have pb_control_test_reset(), you need direct access to the
   * module globals here. */
  /* memset(DispatchTable, 0, sizeof(DispatchTable)); */
  /* DispachTableSize = 0; */
#endif
}

void setUp(void) {
  reset_trackers();
  fill_handler_array();

  for (int i = 0; i < 15; i++) {
    packet_type_array[i] = (uint16_t)i;
  }

  reset_module_state();
}

void tearDown(void) {}

/* -------------------------------------------------------------------------- */
/* Helper asserts                                                             */
/* -------------------------------------------------------------------------- */
static void assert_no_handlers_called(void) {
  for (int i = 0; i < 15; i++) {
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, trackers[i].call_count,
                                  "Unexpected handler call_count != 0");
    TEST_ASSERT_NULL_MESSAGE(trackers[i].last_pointer,
                             "Unexpected last_pointer != NULL");
    TEST_ASSERT_EQUAL_size_t_MESSAGE(0, trackers[i].last_size,
                                     "Unexpected last_size != 0");
  }
}

static void assert_only_handler_called(int idx) {
  for (int i = 0; i < 15; i++) {
    if (i == idx) {
      TEST_ASSERT_EQUAL_INT_MESSAGE(1, trackers[i].call_count,
                                    "Expected handler not called exactly once");
    } else {
      TEST_ASSERT_EQUAL_INT_MESSAGE(0, trackers[i].call_count,
                                    "Unexpected other handler call");
    }
  }
}

/* -------------------------------------------------------------------------- */
/* Tests: initialization                                                      */
/* -------------------------------------------------------------------------- */
void test_initialized_before_initialization(void) {
  TEST_ASSERT_FALSE(pb_control_is_initialized());
}

void test_initialize_success_sets_initialized(void) {
  TEST_ASSERT_EQUAL(
      RESULT_OK, pb_control_initialize(packet_type_array, handler_array, 10));
  TEST_ASSERT_TRUE(pb_control_is_initialized());
}

void test_initialize_failure_too_big(void) {
  TEST_ASSERT_FALSE(pb_control_is_initialized());

  TEST_ASSERT_EQUAL(
      RESULT_ERR_INVALID_ARG,
      pb_control_initialize(packet_type_array, handler_array,
                            (uint16_t)(MAX_PROCESSABLE_PACKET_TYPES + 1)));

  TEST_ASSERT_FALSE(pb_control_is_initialized());
}

void test_initialize_zero_entries_ok_and_remains_uninitialized(void) {
  TEST_ASSERT_EQUAL(RESULT_OK, pb_control_initialize(NULL, NULL, 0));
  TEST_ASSERT_FALSE(pb_control_is_initialized());
}

void test_initialize_null_arrays_when_count_nonzero_fails(void) {
  TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_ARG,
                    pb_control_initialize(NULL, handler_array, 1));
  TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_ARG,
                    pb_control_initialize(packet_type_array, NULL, 1));
  TEST_ASSERT_FALSE(pb_control_is_initialized());
}

void test_initialize_null_handler_entry_fails(void) {
  handler_array[3] = NULL;

  TEST_ASSERT_EQUAL(
      RESULT_ERR_INVALID_ARG,
      pb_control_initialize(packet_type_array, handler_array, 10));
  TEST_ASSERT_FALSE(pb_control_is_initialized());
}

/* -------------------------------------------------------------------------- */
/* Tests: processing validation */
/* -------------------------------------------------------------------------- */
void test_process_rejects_null_packet_data(void) {
  TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_ARG,
                    pb_control_process_incoming_packet(NULL, 3));
  assert_no_handlers_called();
}

void test_process_rejects_too_small_packet_size(void) {
  uint8_t pkt2[2] = {0x00, 0x01};
  TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_ARG,
                    pb_control_process_incoming_packet(pkt2, sizeof(pkt2)));
  assert_no_handlers_called();
}

void test_process_not_found_when_uninitialized(void) {
  uint8_t pkt[3] = {0x00, 0x01, 0xAA}; /* type=1, 1 byte payload */
  TEST_ASSERT_EQUAL(RESULT_ERR_NOT_INITIALIZED,
                    pb_control_process_incoming_packet(pkt, sizeof(pkt)));
  assert_no_handlers_called();
}

/* -------------------------------------------------------------------------- */
/* Tests: dispatch behavior */
/* Assumption enforced by tests: type is first two bytes big-endian, payload */
/* begins at &packet_data[2], size = packet_size - 2. */
/* -------------------------------------------------------------------------- */
void test_process_not_found_when_type_not_registered(void) {
  /* Register types 0..9 */
  TEST_ASSERT_EQUAL(
      RESULT_OK, pb_control_initialize(packet_type_array, handler_array, 10));

  /* Request type 14, not registered */
  uint8_t pkt[4] = {0x00, 0x0E, 0xDE, 0xAD}; /* 0x000E == 14 */
  TEST_ASSERT_EQUAL(RESULT_ERR_NOT_FOUND,
                    pb_control_process_incoming_packet(pkt, sizeof(pkt)));

  assert_no_handlers_called();
}

void test_process_dispatches_correct_handler_and_passes_payload_ptr_and_size(
    void) {
  /* Register types 0..9 */
  TEST_ASSERT_EQUAL(
      RESULT_OK, pb_control_initialize(packet_type_array, handler_array, 10));

  /* type=7 => handler_7 */
  uint8_t pkt[6] = {0x00, 0x07, 0xDE, 0xAD, 0xBE, 0xEF}; /* payload 4 bytes */

  TEST_ASSERT_EQUAL(RESULT_OK,
                    pb_control_process_incoming_packet(pkt, sizeof(pkt)));

  assert_only_handler_called(7);
  TEST_ASSERT_EQUAL_PTR((void *)&pkt[2], (void *)trackers[7].last_pointer);
  TEST_ASSERT_EQUAL_size_t(sizeof(pkt) - 2u, trackers[7].last_size);
}

void test_process_multiple_calls_hit_correct_handlers_each_time(void) {
  /* Register types 0..14 */
  TEST_ASSERT_EQUAL(
      RESULT_OK, pb_control_initialize(packet_type_array, handler_array, 15));

  uint8_t pkt_a[3] = {0x00, 0x02, 0xAA};             /* type=2 */
  uint8_t pkt_b[5] = {0x00, 0x0D, 0x01, 0x02, 0x03}; /* type=13 */

  TEST_ASSERT_EQUAL(RESULT_OK,
                    pb_control_process_incoming_packet(pkt_a, sizeof(pkt_a)));
  TEST_ASSERT_EQUAL(RESULT_OK,
                    pb_control_process_incoming_packet(pkt_b, sizeof(pkt_b)));

  TEST_ASSERT_EQUAL_INT(1, trackers[2].call_count);
  TEST_ASSERT_EQUAL_PTR((void *)&pkt_a[2], (void *)trackers[2].last_pointer);
  TEST_ASSERT_EQUAL_size_t(sizeof(pkt_a) - 2u, trackers[2].last_size);

  TEST_ASSERT_EQUAL_INT(1, trackers[13].call_count);
  TEST_ASSERT_EQUAL_PTR((void *)&pkt_b[2], (void *)trackers[13].last_pointer);
  TEST_ASSERT_EQUAL_size_t(sizeof(pkt_b) - 2u, trackers[13].last_size);

  /* Everyone else untouched */
  for (int i = 0; i < 15; i++) {
    if (i == 2 || i == 13) {
      continue;
    }
    TEST_ASSERT_EQUAL_INT(0, trackers[i].call_count);
  }
}

/* -------------------------------------------------------------------------- */
/* Main */
/* -------------------------------------------------------------------------- */
int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_initialized_before_initialization);

  RUN_TEST(test_initialize_success_sets_initialized);
  RUN_TEST(test_initialize_failure_too_big);
  RUN_TEST(test_initialize_zero_entries_ok_and_remains_uninitialized);
  RUN_TEST(test_initialize_null_arrays_when_count_nonzero_fails);
  RUN_TEST(test_initialize_null_handler_entry_fails);

  RUN_TEST(test_process_rejects_null_packet_data);
  RUN_TEST(test_process_rejects_too_small_packet_size);
  RUN_TEST(test_process_not_found_when_uninitialized);

  RUN_TEST(test_process_not_found_when_type_not_registered);
  RUN_TEST(
      test_process_dispatches_correct_handler_and_passes_payload_ptr_and_size);
  RUN_TEST(test_process_multiple_calls_hit_correct_handlers_each_time);

  return UNITY_END();
}
