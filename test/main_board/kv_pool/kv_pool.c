
#include "kv_pool.h" // Your header file
#include "unity.h"
#include <stddef.h> // For offsetof
#include <string.h> // For memset

// --- Test Configuration ---

// Define a reasonable number of keys for testing
#define TEST_MAX_KEYS 32

// Define the size of the lookup table based on the keys
static const size_t g_lookup_table_size = sizeof(kv_slot) * TEST_MAX_KEYS;

// Define a total pool size for the contiguous tests
#define TEST_CONTIGUOUS_POOL_SIZE (1024 * 4) // 4KB

// Define separate buffers for the fragmented tests
#define TEST_HEAP_BUFFER_SIZE (1024 * 4) // 4KB

// --- Global Test Fixtures ---

// The global pool instance, reset before each test
static kv_pool g_pool;

// A large buffer for contiguous mode tests
static char g_memory_pool[TEST_CONTIGUOUS_POOL_SIZE];

// Separate buffers for fragmented mode tests
static char g_lookup_buffer[g_lookup_table_size];
static char g_heap_buffer[TEST_HEAP_BUFFER_SIZE];

// --- Helper Functions ---

// Helper to get the header from a data pointer
static kv_header *get_header_from_ptr(void *ptr) {
  if (ptr == NULL)
    return NULL;
  return (kv_header *)((char *)ptr - offsetof(kv_header, as.data));
}

// --- setUp and tearDown ---

// This function is run before *each* test
void setUp(void) {
  // Reset all test fixtures to a known state
  memset(&g_pool, 0, sizeof(kv_pool));
  memset(g_memory_pool, 0xCD, sizeof(g_memory_pool));
  memset(g_lookup_buffer, 0xAB, sizeof(g_lookup_buffer));
  memset(g_heap_buffer, 0xBC, sizeof(g_heap_buffer));
}

// This function is run after *each* test
void tearDown(void) {
  // No cleanup needed, as setUp resets everything
}

// --- Test Cases: kv_pool_init (Contiguous) ---

void test_kv_pool_init_Contiguous_Success(void) {
  // --- Act ---
  result_t result = kv_pool_init(g_memory_pool, sizeof(g_memory_pool),
                                 TEST_MAX_KEYS, &g_pool);

  // --- Assert ---
  TEST_ASSERT_EQUAL(RESULT_OK, result);
  TEST_ASSERT_EQUAL_PTR(g_memory_pool, g_pool.pool_start);
  TEST_ASSERT_EQUAL(sizeof(g_memory_pool), g_pool.pool_size);
  TEST_ASSERT_EQUAL(TEST_MAX_KEYS, g_pool.max_keys);
  TEST_ASSERT_EQUAL_PTR(g_memory_pool, g_pool.lookup_table);

  // Check heap initialization
  void *expected_heap_start = (char *)g_memory_pool + g_lookup_table_size;
  size_t expected_heap_size = sizeof(g_memory_pool) - g_lookup_table_size;

  TEST_ASSERT_EQUAL_PTR(expected_heap_start, g_pool.free_list_head);
  TEST_ASSERT_EQUAL(expected_heap_size, g_pool.free_list_head->size);
  TEST_ASSERT_NULL(g_pool.free_list_head->as.next_free);

  // Check slot initialization
  TEST_ASSERT_EQUAL(false, g_pool.lookup_table[0].is_valid);
  TEST_ASSERT_EQUAL(false, g_pool.lookup_table[TEST_MAX_KEYS - 1].is_valid);
  TEST_ASSERT_NULL(g_pool.lookup_table[0].data_ptr);
}

void test_kv_pool_init_Contiguous_Failure_TooSmall(void) {
  result_t result;

  // 1. Not enough room for the lookup table
  result = kv_pool_init(g_memory_pool, g_lookup_table_size - 1, TEST_MAX_KEYS,
                        &g_pool);
  TEST_ASSERT_EQUAL(RESULT_BUFFER_TOO_SMALL, result);

  // 2. Enough for table, but not enough for one heap header
  size_t just_too_small = g_lookup_table_size + sizeof(kv_header) - 1;
  result = kv_pool_init(g_memory_pool, just_too_small, TEST_MAX_KEYS, &g_pool);
  TEST_ASSERT_EQUAL(RESULT_BUFFER_TOO_SMALL, result);
}

void test_kv_pool_init_Contiguous_Failure_NullArgs(void) {
  TEST_ASSERT_EQUAL(
      RESULT_INVALID_ARGS,
      kv_pool_init(NULL, sizeof(g_memory_pool), TEST_MAX_KEYS, &g_pool));
  TEST_ASSERT_EQUAL(
      RESULT_INVALID_ARGS,
      kv_pool_init(g_memory_pool, sizeof(g_memory_pool), TEST_MAX_KEYS, NULL));
  TEST_ASSERT_EQUAL(
      RESULT_INVALID_ARGS,
      kv_pool_init(g_memory_pool, sizeof(g_memory_pool), 0, &g_pool));
}

// --- Test Cases: kv_pool_init_fragmented ---

void test_kv_pool_init_Fragmented_Success(void) {
  // --- Act ---
  result_t result =
      kv_pool_init_fragmented(g_lookup_buffer, TEST_MAX_KEYS, g_heap_buffer,
                              sizeof(g_heap_buffer), &g_pool);

  // --- Assert ---
  TEST_ASSERT_EQUAL(RESULT_OK, result);
  TEST_ASSERT_EQUAL(TEST_MAX_KEYS, g_pool.max_keys);
  TEST_ASSERT_EQUAL_PTR(g_lookup_buffer, g_pool.lookup_table);

  // Check heap initialization
  TEST_ASSERT_EQUAL_PTR(g_heap_buffer, g_pool.free_list_head);
  TEST_ASSERT_EQUAL(sizeof(g_heap_buffer), g_pool.free_list_head->size);
  TEST_ASSERT_NULL(g_pool.free_list_head->as.next_free);

  // Check slot initialization
  TEST_ASSERT_EQUAL(false, g_pool.lookup_table[0].is_valid);
  TEST_ASSERT_EQUAL(false, g_pool.lookup_table[TEST_MAX_KEYS - 1].is_valid);
}

void test_kv_pool_init_Fragmented_Failure_TooSmall(void) {
  // Not enough room for one heap header
  result_t result =
      kv_pool_init_fragmented(g_lookup_buffer, TEST_MAX_KEYS, g_heap_buffer,
                              sizeof(kv_header) - 1, &g_pool);
  TEST_ASSERT_EQUAL(RESULT_BUFFER_TOO_SMALL, result);
}

void test_kv_pool_init_Fragmented_Failure_NullArgs(void) {
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS,
                    kv_pool_init_fragmented(NULL, TEST_MAX_KEYS, g_heap_buffer,
                                            sizeof(g_heap_buffer), &g_pool));
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS,
                    kv_pool_init_fragmented(g_lookup_buffer, 0, g_heap_buffer,
                                            sizeof(g_heap_buffer), &g_pool));
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS,
                    kv_pool_init_fragmented(g_lookup_buffer, TEST_MAX_KEYS,
                                            NULL, sizeof(g_heap_buffer),
                                            &g_pool));
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS,
                    kv_pool_init_fragmented(g_lookup_buffer, TEST_MAX_KEYS,
                                            g_heap_buffer,
                                            sizeof(g_heap_buffer), NULL));
}

// --- Test Cases: kv_pool_allocate and kv_pool_free ---

void test_kv_pool_Allocate_And_Free_Single_Block(void) {
  // --- Arrange ---
  kv_pool_init(g_memory_pool, sizeof(g_memory_pool), TEST_MAX_KEYS, &g_pool);
  void *ptr = NULL;
  size_t alloc_size = 128;

  // Save original heap state
  kv_header *original_heap = g_pool.free_list_head;
  size_t original_heap_size = original_heap->size;

  // --- Act ---
  result_t alloc_result = kv_pool_allocate(&g_pool, alloc_size, &ptr);

  // --- Assert Alloc ---
  TEST_ASSERT_EQUAL(RESULT_OK, alloc_result);
  TEST_ASSERT_NOT_NULL(ptr);
  TEST_ASSERT_GREATER_OR_EQUAL_PTR((char *)g_memory_pool + g_lookup_table_size,
                                   ptr);

  // Check header
  kv_header *header = get_header_from_ptr(ptr);
  TEST_ASSERT_GREATER_OR_EQUAL(header->size, alloc_size);

  // Check free list
  TEST_ASSERT_NOT_EQUAL(original_heap_size, g_pool.free_list_head->size);

  // --- Act Free ---
  result_t free_result = kv_pool_free(&g_pool, ptr);

  // --- Assert Free ---
  TEST_ASSERT_EQUAL(RESULT_OK, free_result);

  // Check if heap is fully coalesced back to its original state
  TEST_ASSERT_EQUAL_PTR(original_heap, g_pool.free_list_head);
  TEST_ASSERT_EQUAL(original_heap_size, g_pool.free_list_head->size);
  TEST_ASSERT_NULL(g_pool.free_list_head->as.next_free);
}

void test_kv_pool_Allocate_OutOfMemory(void) {
  // --- Arrange ---
  kv_pool_init(g_memory_pool, sizeof(g_memory_pool), TEST_MAX_KEYS, &g_pool);
  void *ptr = NULL;

  // 1. Try to allocate more than the entire heap
  size_t too_big = TEST_CONTIGUOUS_POOL_SIZE + 1000;
  result_t result = kv_pool_allocate(&g_pool, too_big, &ptr);
  TEST_ASSERT_EQUAL(RESULT_OUT_OF_MEMORY, result);
  TEST_ASSERT_NULL(ptr);

  // 2. Allocate almost everything
  size_t almost_all =
      sizeof(g_memory_pool) - g_lookup_table_size - sizeof(kv_header) * 2;
  result = kv_pool_allocate(&g_pool, almost_all, &ptr);
  TEST_ASSERT_EQUAL(RESULT_OK, result);
  TEST_ASSERT_NOT_NULL(ptr);

  // 3. Try to allocate 1 more byte (should fail, not enough room for header)
  void *ptr2 = NULL;
  result = kv_pool_allocate(&g_pool, 1, &ptr2);
  TEST_ASSERT_EQUAL(RESULT_OUT_OF_MEMORY, result);
  TEST_ASSERT_NULL(ptr2);
}

void test_kv_pool_Allocate_Multiple_And_Free_OutOfOrder_Coalescing(void) {
  // --- Arrange ---
  kv_pool_init(g_memory_pool, sizeof(g_memory_pool), TEST_MAX_KEYS, &g_pool);
  void *ptr1 = NULL, *ptr2 = NULL, *ptr3 = NULL;
  size_t original_heap_size = g_pool.free_list_head->size;
  kv_header *original_heap_ptr = g_pool.free_list_head;

  // --- Act ---
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 128, &ptr1));
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 256, &ptr2));
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 512, &ptr3));

  TEST_ASSERT_NOT_NULL(ptr1);
  TEST_ASSERT_NOT_NULL(ptr2);
  TEST_ASSERT_NOT_NULL(ptr3);

  // Free list should now be one block at the end
  TEST_ASSERT_NOT_NULL(g_pool.free_list_head);
  TEST_ASSERT_NULL(g_pool.free_list_head->as.next_free);

  // --- Act Free (Out of Order) ---
  // Free ptr1 (Heap: [Free, Alloc, Alloc, Free_End])
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_free(&g_pool, ptr1));
  TEST_ASSERT_EQUAL_PTR(original_heap_ptr,
                        g_pool.free_list_head); // ptr1 is now the head
  TEST_ASSERT_NOT_NULL(
      g_pool.free_list_head->as.next_free); // Points to Free_End

  // Free ptr3 (Heap: [Free, Alloc, Free, Free_End]) -> Coalesces
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_free(&g_pool, ptr3));
  TEST_ASSERT_EQUAL_PTR(original_heap_ptr,
                        g_pool.free_list_head); // Head is still ptr1's block
  TEST_ASSERT_NOT_NULL(
      g_pool.free_list_head->as
          .next_free); // Still points to Free_End, which is now bigger
  TEST_ASSERT_EQUAL_PTR(get_header_from_ptr(ptr2)->as.next_free,
                        g_pool.free_list_head->as.next_free); // (complex check)

  // Free ptr2 (Heap: [Free, Free, Free, Free_End]) -> 3-way Coalesce
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_free(&g_pool, ptr2));

  // --- Assert ---
  // The entire heap should be one single free block again
  TEST_ASSERT_EQUAL_PTR(original_heap_ptr, g_pool.free_list_head);
  TEST_ASSERT_EQUAL(original_heap_size, g_pool.free_list_head->size);
  TEST_ASSERT_NULL(g_pool.free_list_head->as.next_free);
}

void test_kv_pool_Allocate_Free_Null_Args(void) {
  kv_pool_init(g_memory_pool, sizeof(g_memory_pool), TEST_MAX_KEYS, &g_pool);
  void *ptr;

  // Allocate
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS, kv_pool_allocate(NULL, 128, &ptr));
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS, kv_pool_allocate(&g_pool, 128, NULL));
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS, kv_pool_allocate(&g_pool, 0, &ptr));

  // Free
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS, kv_pool_free(NULL, (void *)0x1234));
  TEST_ASSERT_EQUAL(RESULT_INVALID_ARGS, kv_pool_free(&g_pool, NULL));
}

void test_kv_pool_Allocate_Alignment(void) {
  // This test assumes KV_ALIGNMENT is defined in kv_pool.h
  kv_pool_init(g_memory_pool, sizeof(g_memory_pool), TEST_MAX_KEYS, &g_pool);
  void *ptr1 = NULL;
  void *ptr2 = NULL;

  // Allocate two small, alignment-testing sizes
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 1, &ptr1));
  TEST_ASSERT_EQUAL(RESULT_OK, kv_pool_allocate(&g_pool, 1, &ptr2));

  TEST_ASSERT_NOT_NULL(ptr1);
  TEST_ASSERT_NOT_NULL(ptr2);

  // Check that the returned data pointers are aligned
  TEST_ASSERT_EQUAL(0, (uintptr_t)ptr1 % KV_ALIGNMENT);
  TEST_ASSERT_EQUAL(0, (uintptr_t)ptr2 % KV_ALIGNMENT);

  // Check that the blocks are adjacent based on aligned size
  kv_header *h1 = get_header_from_ptr(ptr1);
  kv_header *h2_check = (kv_header *)((char *)h1 + h1->size);
  TEST_ASSERT_EQUAL_PTR(get_header_from_ptr(ptr2), h2_check);
}

// --- Test Runner ---

// You must define this main function to run the tests
int main(void) {
  UNITY_BEGIN();

  // Init tests
  RUN_TEST(test_kv_pool_init_Contiguous_Success);
  RUN_TEST(test_kv_pool_init_Contiguous_Failure_TooSmall);
  RUN_TEST(test_kv_pool_init_Contiguous_Failure_NullArgs);
  RUN_TEST(test_kv_pool_init_Fragmented_Success);
  RUN_TEST(test_kv_pool_init_Fragmented_Failure_TooSmall);
  RUN_TEST(test_kv_pool_init_Fragmented_Failure_NullArgs);

  // Alloc/Free tests
  RUN_TEST(test_kv_pool_Allocate_OutOfMemory);
  RUN_TEST(test_kv_pool_Allocate_And_Free_Single_Block);
  RUN_TEST(test_kv_pool_Allocate_Multiple_And_Free_OutOfOrder_Coalescing);
  RUN_TEST(test_kv_pool_Allocate_Free_Null_Args);
  RUN_TEST(test_kv_pool_Allocate_Alignment);

  return UNITY_END();
}
