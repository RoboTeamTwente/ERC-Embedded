
#include "kv_pool.h"
#include "logging.h"
#include "result.h" // Assuming this is where result_t is defined
#include "unity.h"
#include <stdint.h> // For uintptr_t
#include <string.h>
#include <time.h> // For nanosleep

// --- Test Configuration ---
// Fix for: "variably modified ‘...’ at file scope"
// 'const size_t' is not a true compile-time constant in C.
// Use #define for global array sizes.
#define TEST_MAX_KEYS 8
#define G_LOOKUP_TABLE_SIZE (TEST_MAX_KEYS * sizeof(kv_slot))
#define G_HEAP_SIZE (1024)
#define G_TOTAL_POOL_SIZE (G_LOOKUP_TABLE_SIZE + G_HEAP_SIZE)

// Ensure KV_ALIGNMENT is defined if not in kv_pool.h
#ifndef KV_ALIGNMENT
#define KV_ALIGNMENT 16
#endif

const char *TAG = "TEST_KV_POOL";

// --- Global Test Fixtures ---
static kv_pool g_pool;
static char g_memory_pool[G_TOTAL_POOL_SIZE];
static char g_lookup_buffer[G_LOOKUP_TABLE_SIZE];
static char g_heap_buffer[G_HEAP_SIZE];

// --- Helper Functions ---

/**
 * @brief A simple 100ms delay for testing.
 */
static void delay_100ms(void) {
  HAL_Delay(100);
}

// Helper to get the start of the heap in the contiguous pool
static void *get_contiguous_heap_start(void) {
  return (char *)g_memory_pool + G_LOOKUP_TABLE_SIZE;
}

// --- setUp and tearDown ---
void setUp(void) {
  // Clear all memory before each test
  memset(&g_pool, 0, sizeof(g_pool));
  memset(g_memory_pool, 0xAA, sizeof(g_memory_pool));
  memset(g_lookup_buffer, 0xBB, sizeof(g_lookup_buffer));
  memset(g_heap_buffer, 0xCC, sizeof(g_heap_buffer));

  // Always init with a default pool for KV tests
  kv_pool_init(g_memory_pool, sizeof(g_memory_pool), TEST_MAX_KEYS, &g_pool,
               delay_100ms);
}

void tearDown(void) {
  // No explicit cleanup needed, setUp handles it
}

// --- Test Cases ---

//
// kv_pool_init (Contiguous)
//
void test_kv_pool_init_Contiguous_Success(void) {
  // setUp already ran, but we re-init for clarity
  memset(&g_pool, 0, sizeof(g_pool)); // clear setUp's init

  TEST_ASSERT_EQUAL(RESULT_OK,
                    kv_pool_init(g_memory_pool, sizeof(g_memory_pool),
                                 TEST_MAX_KEYS, &g_pool, delay_100ms));

  // Check that the pool struct is correctly populated
  TEST_ASSERT_EQUAL_PTR(g_memory_pool + G_LOOKUP_TABLE_SIZE, g_pool.pool_start);
  TEST_ASSERT_EQUAL(sizeof(g_memory_pool) - G_LOOKUP_TABLE_SIZE,
                    g_pool.pool_size);
  TEST_ASSERT_EQUAL(TEST_MAX_KEYS, g_pool.max_keys);
  TEST_ASSERT_EQUAL_PTR(g_memory_pool, g_pool.lookup_table);
  TEST_ASSERT_EQUAL_PTR(get_contiguous_heap_start(), g_pool.free_list_head);

  // Check that the heap is set up correctly
  kv_header *head = g_pool.free_list_head;
  TEST_ASSERT_NOT_NULL(head);
  TEST_ASSERT_EQUAL(G_HEAP_SIZE, head->size);
  TEST_ASSERT_NULL(head->as.next_free);
}

void test_kv_pool_init_Contiguous_Failure_NullArgs(void) {
  TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_ARG,
                    kv_pool_init(NULL, sizeof(g_memory_pool), TEST_MAX_KEYS,
                                 &g_pool, delay_100ms));
  TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_ARG,
                    kv_pool_init(g_memory_pool, sizeof(g_memory_pool),
                                 TEST_MAX_KEYS, NULL, delay_100ms));
  TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_ARG,
                    kv_pool_init(g_memory_pool, sizeof(g_memory_pool), 0,
                                 &g_pool, delay_100ms));
}

void test_kv_pool_init_Contiguous_Failure_TooSmall(void) {
  // Too small for even the lookup table
  TEST_ASSERT_EQUAL(RESULT_ERR_BUFFER_TOO_SMALL,
                    kv_pool_init(g_memory_pool, G_LOOKUP_TABLE_SIZE - 1,
                                 TEST_MAX_KEYS, &g_pool, delay_100ms));

  // Too small for lookup table + minimal heap
  size_t barely_too_small = G_LOOKUP_TABLE_SIZE + sizeof(kv_header) - 1;
  TEST_ASSERT_EQUAL(RESULT_ERR_BUFFER_TOO_SMALL,
                    kv_pool_init(g_memory_pool, barely_too_small, TEST_MAX_KEYS,
                                 &g_pool, delay_100ms));
}

//
// kv_pool_init_fragmented
//
void test_kv_pool_init_Fragmented_Success(void) {
  // clear setUp's init
  memset(&g_pool, 0, sizeof(g_pool));

  TEST_ASSERT_EQUAL(
      RESULT_OK, kv_pool_init_fragmented(g_lookup_buffer, G_LOOKUP_TABLE_SIZE,
                                         TEST_MAX_KEYS, g_heap_buffer,
                                         G_HEAP_SIZE, &g_pool, delay_100ms));

  // Check that the pool struct is correctly populated
  TEST_ASSERT_EQUAL(g_heap_buffer,
                    g_pool.pool_start); // Not used in fragmented init
  TEST_ASSERT_EQUAL(G_HEAP_SIZE,
                    g_pool.pool_size); // Not used in fragmented init
  TEST_ASSERT_EQUAL(TEST_MAX_KEYS, g_pool.max_keys);
  TEST_ASSERT_EQUAL_PTR(g_lookup_buffer, g_pool.lookup_table);

  // Check that the heap is set up correctly
  kv_header *head = g_pool.free_list_head;
  TEST_ASSERT_NOT_NULL(head);
  TEST_ASSERT_EQUAL(G_HEAP_SIZE, head->size);
  TEST_ASSERT_NULL(head->as.next_free);
}

//
// kv_pool_allocate & kv_pool_free (Internal Heap API)
//
void test_kv_pool_Allocate_And_Free_Single_Block(void) {
  void *ptr = NULL;

  // Allocate
  size_t alloc_size = 128;
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, alloc_size, &ptr));
  TEST_ASSERT_NOT_NULL(ptr);

  // Check that the pointer is *within* the heap
  TEST_ASSERT_GREATER_OR_EQUAL((uintptr_t)get_contiguous_heap_start(),
                               (uintptr_t)ptr);

  // Free
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_free(&g_pool, ptr));

  // Check that the free list is coalesced back to one giant block
  kv_header *head = g_pool.free_list_head;
  TEST_ASSERT_NOT_NULL(head);
  TEST_ASSERT_EQUAL_PTR(get_contiguous_heap_start(), head);
  TEST_ASSERT_EQUAL(G_HEAP_SIZE, head->size);
  TEST_ASSERT_NULL(head->as.next_free);
}

void test_kv_pool_Allocate_OutOfMemory(void) {
  void *ptr = NULL;

  // Allocate the entire heap
  TEST_ASSERT_EQUAL(
      RESULT_OK,
      kv_pool_allocate(&g_pool, G_HEAP_SIZE - sizeof(kv_header), &ptr));
  TEST_ASSERT_NOT_NULL(ptr);

  // Try to allocate more
  void *ptr2 = NULL;
  TEST_ASSERT_EQUAL(RESULT_ERR_NO_MEM, kv_pool_allocate(&g_pool, 64, &ptr2));
  TEST_ASSERT_NULL(ptr2);

  // Free the first block
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_free(&g_pool, ptr));

  // Now the second allocation should succeed
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 64, &ptr2));
  TEST_ASSERT_NOT_NULL(ptr2);
}

void test_kv_pool_Allocate_Multiple_And_Free_OutOfOrder_Coalescing(void) {
  void *p1 = NULL, *p2 = NULL, *p3 = NULL;

  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 100, &p1));
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 200, &p2));
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 300, &p3));
  TEST_ASSERT_NOT_NULL(p1);
  TEST_ASSERT_NOT_NULL(p2);
  TEST_ASSERT_NOT_NULL(p3);

  // Free list should have one block left
  TEST_ASSERT_NOT_NULL(g_pool.free_list_head);

  // Free p1 (at start of heap)
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_free(&g_pool, p1));
  // Free p3 (at end of allocated region)
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_free(&g_pool, p3));

  // Free p2 (the middle block)
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_free(&g_pool, p2));

  // Now, a full coalesce should have happened.
  kv_header *head = g_pool.free_list_head;
  TEST_ASSERT_NOT_NULL(head);
  TEST_ASSERT_EQUAL_PTR(get_contiguous_heap_start(), head);
  TEST_ASSERT_EQUAL(G_HEAP_SIZE, head->size);
  TEST_ASSERT_NULL(head->as.next_free);
}

//
// Key-Value API (External)
//
void test_kv_pool_Insert_And_Get_Valid(void) {
  const char *test_string = "Hello KV Pool!";
  size_t test_size = strlen(test_string) + 1;
  int key = 3;

  // Check that it's not valid yet
  TEST_ASSERT_EQUAL(RESULT_ERR_NOT_FOUND, kv_pool_is_index_valid(&g_pool, key));

  // Insert
  TEST_ASSERT_EQUAL(
      RESULT_OK, kv_pool_insert(&g_pool, key, (void *)test_string, test_size));

  // Check that it is now valid
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_is_index_valid(&g_pool, key));

  // Get (copy-out)
  char read_buffer[64];
  size_t read_size = sizeof(read_buffer);
  TEST_ASSERT_EQUAL(RESULT_OK,
                    kv_pool_get(&g_pool, key, read_buffer, &read_size));

  // Verify data
  TEST_ASSERT_EQUAL(test_size, read_size);
  TEST_ASSERT_EQUAL_STRING(test_string, read_buffer);
}

void test_kv_pool_Remove_Key(void) {
  int key = 5;
  uint32_t data = 12345;

  // Insert
  TEST_ASSERT_EQUAL(RESULT_OK,
                    kv_pool_insert(&g_pool, key, &data, sizeof(data)));
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_is_index_valid(&g_pool, key));

  // Remove
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_remove(&g_pool, key));

  // Check that it's no longer valid
  TEST_ASSERT_EQUAL(RESULT_ERR_NOT_FOUND, kv_pool_is_index_valid(&g_pool, key));

  // Try to get data (should fail)
  char buffer[16];
  size_t size = sizeof(buffer);
  TEST_ASSERT_EQUAL(RESULT_ERR_NOT_FOUND,
                    kv_pool_get(&g_pool, key, buffer, &size));
}

void test_kv_pool_Get_Fails_BufferTooSmall(void) {
  int key = 2;
  const char *test_string = "This string is definitely too long";
  size_t test_size = strlen(test_string) + 1;

  TEST_ASSERT_EQUAL(
      RESULT_OK, kv_pool_insert(&g_pool, key, (void *)test_string, test_size));

  char small_buffer[10];
  size_t size = sizeof(small_buffer);

  TEST_ASSERT_EQUAL(RESULT_ERR_BUFFER_TOO_SMALL,
                    kv_pool_get(&g_pool, key, small_buffer, &size));

  // Check that the 'size' was updated to the *required* size
  TEST_ASSERT_EQUAL(test_size, size);
}

void test_kv_pool_Write_Valid(void) {
  int key = 7;
  const char *string1 = "Hello"; // 6 bytes
  const char *string2 = "World"; // 6 bytes
  size_t size = 6;

  TEST_ASSERT_EQUAL(RESULT_OK,
                    kv_pool_insert(&g_pool, key, (void *)string1, size));

  // Write over it
  TEST_ASSERT_EQUAL(RESULT_OK,
                    kv_pool_write(&g_pool, key, (void *)string2, size));

  // Get and verify new data
  char buffer[10];
  size_t read_size = sizeof(buffer);
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_get(&g_pool, key, buffer, &read_size));
  TEST_ASSERT_EQUAL(size, read_size);
  TEST_ASSERT_EQUAL_STRING(string2, buffer);
}

void test_kv_pool_Write_Fails_SizeMismatch(void) {
  int key = 6;
  uint32_t data1 = 12345;
  uint64_t data2 = 987654321;

  TEST_ASSERT_EQUAL(RESULT_OK,
                    kv_pool_insert(&g_pool, key, &data1, sizeof(data1)));

  // Try to write with a different size
  TEST_ASSERT_EQUAL(RESULT_ERR_INVALID_ARG,
                    kv_pool_write(&g_pool, key, &data2, sizeof(data2)));
}

// --- Main Test Runner ---
int main(void) {
  UNITY_BEGIN();

  // kv_pool_init (Contiguous)
  RUN_TEST(test_kv_pool_init_Contiguous_Success);
  RUN_TEST(test_kv_pool_init_Contiguous_Failure_NullArgs);
  RUN_TEST(test_kv_pool_init_Contiguous_Failure_TooSmall);

  // kv_pool_init_fragmented
  RUN_TEST(test_kv_pool_init_Fragmented_Success);

  // Alloc/Free tests
  RUN_TEST(test_kv_pool_Allocate_And_Free_Single_Block);
  RUN_TEST(test_kv_pool_Allocate_OutOfMemory);
  RUN_TEST(test_kv_pool_Allocate_Multiple_And_Free_OutOfOrder_Coalescing);

  // KV API Tests
  RUN_TEST(test_kv_pool_Insert_And_Get_Valid);
  RUN_TEST(test_kv_pool_Remove_Key);
  RUN_TEST(test_kv_pool_Get_Fails_BufferTooSmall);
  RUN_TEST(test_kv_pool_Write_Valid);
  RUN_TEST(test_kv_pool_Write_Fails_SizeMismatch);

  return UNITY_END();
}
