#ifndef KV_POOL_H
#define KV_POOL_H
#include "result.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KV_ALIGNMENT 16 // Or 8, or 32
#define ALIGN(x) (((x) + (KV_ALIGNMENT - 1)) & ~(KV_ALIGNMENT - 1))
#define MINIMUM_BLOCK_SIZE sizeof(kv_header) + 1
#define LOOKUP_TABLE_SIZE(x) (sizeof(kv_slot) * (x))

/**
 * @brief KV Slot metadata that holds all information about a specific Key-Value
 * Pair (where the key is kv_slot index in the slot pool)
 */
typedef struct {
  atomic_flag slot_lock; // Atomic lock on specific data;
  void *data_ptr; // Pointer to data, should never be freed or reallocated
                  // directly/manually
  size_t data_size;
  bool is_valid; // If the current slot is taken or empty
} kv_slot;

/**
 * @brief Internal header for a memory block within the data heap.
 *
 * Manages free blocks by forming a linked list.
 */
typedef struct kv_header {
  size_t size;
  union {
    struct kv_header *next_free;
    char data[1];
  } as;
} kv_header;

/**
 *
 * @brief The main management structure for the entire key-value pool.
 *
 * Holds the global heap allocator and the fixed-size key lookup table.
 */
typedef struct {
  void *pool_start;
  size_t pool_size;

  atomic_flag heap_lock; // mutex lock for the whole heap
  void (*delay)(void);   // delay function pointer for mutex waits
  kv_header *free_list_head;

  // Key management
  size_t max_keys;
  kv_slot *lookup_table;

} kv_pool;

/**
 * @brief Initializes a kv_pool from two separate, user-provided memory regions.
 *
 * This allows placing the lookup table and the data heap in different
 * memory banks (e.g., fast RAM vs. bulk DRAM).
 *
 * @warning The memory regions at `lookup_table` and `data` must remain
 * valid for the entire lifetime of the `kv_pool`.
 *
 * @param lookup_table  Pointer to the memory for the lookup table.
 * @param lookup_table_size Size of the lookup table memory in bytes.
 * @param max_keys      The number of keys this pool will manage. The
 * `lookup_table` must be at least (max_keys * sizeof(kv_slot)) bytes.
 * @param data          Pointer to the memory for the data heap.
 * @param pool_size     Size of the data heap in bytes.
 * @param[out] pool     The kv_pool struct to initialize.
 * @param delay        A pointer to a delay function to be called when awaiting
 * mutex
 */
result_t kv_pool_init_fragmented(void *lookup_table, size_t lookup_table_size,
                                 size_t max_keys, void *data, size_t pool_size,
                                 kv_pool *pool, void (*delay)(void));

/**
 * @brief Initializes a kv_pool from a single, contiguous block of memory.
 *
 * This function will split the memory block into two sections:
 * 1. The lookup table (size: max_keys * sizeof(kv_slot))
 * 2. The data heap (the remaining memory)
 *
 * @warning The memory region at `data` must remain valid for the entire
 * lifetime of the `kv_pool`.
 *
 * @param data      Pointer to the total allocated memory block.
 * @param data_size Total size of the `data` block in bytes.
 * @param max_keys  The number of keys to allocate for the lookup table.
 * @param[out] pool The kv_pool struct to initialize.
 * @param delay        A pointer to a delay function to be called when awaiting
 * mutex
 */
result_t kv_pool_init(void *data, size_t data_size, size_t max_keys,
                      kv_pool *pool, void (*delay)(void));

/**
 * @brief Safely copies a key's data into a user-provided buffer.
 *
 * This function performs a thread-safe, read-only "snapshot" of the data
 * associated with a key. It locks the slot, copies the data into the
 * 'buffer', and immediately releases the lock.
 *
 * The 'buffer_size' parameter is used for both input (capacity) and
 * output (actual size).
 *
 * @param[in]     pool         A pointer to the initialized kv_pool.
 * @param[in]     key          The key (index) of the slot to read.
 * @param[out]    buffer       A pointer to the destination buffer where
 * the data will be copied.
 * holding the *maximum capacity* of the buffer.
 * As **output**, this value will be updated to
 * the *actual data size* of the key's value.
 *
 * @return      RESULT_OK on success.
 * @return      RESULT_ERR_NOT_FOUND if the key is out of bounds or not valid.
 * @return      RESULT_ERR_BUFFER_TOO_SMALL if the buffer's capacity (the
 * @param[in,out] buffer_size  As **input**, this must point to a size_t
 * input *buffer_size) was less than the key's data size.
 * On this error, *buffer_size is still updated to the
 * required size, but no data is copied.
 */
result_t kv_pool_get(kv_pool *pool, int key, void *buffer, size_t *buffer_size);

/**
 * @brief Safely overwrites the data for an existing, valid key.
 *
 * This function performs a thread-safe "write" operation. It locks the
 * slot, verifies the existing data size *exactly* matches 'buffer_size',
 * copies the new data from 'buffer' into the pool's heap, and then
 * releases the lock.
 *
 * @note This function is for *overwriting* existing data. It is not
 * for allocation or resizing. Use kv_pool_insert() to create a new
 * key-value pair.
 *
 * @param[in] pool         A pointer to the initialized kv_pool.
 * @param[in] key          The key (index) of the slot to write to.
 * @param[in] buffer       A pointer to the source buffer containing the
 * new data to be written.
 * @param[in] buffer_size  The size of the data in 'buffer'. This *must*
 * match the currently allocated size for this key.
 *
 * @return      RESULT_OK on success.
 * @return      RESULT_NOT_FOUND if the key is out of bounds or not valid.
 * @return      RESULT_INVALID_SIZE if 'buffer_size' does not match the
 * existing, allocated data size for this key. This
 * allocator does not support in-place resizing.
 */
result_t kv_pool_write(kv_pool *pool, int key, void *buffer,
                       size_t buffer_size);

/**
 * @brief Allocates heap space and inserts new data for a given key.
 *
 * This function creates a new key-value pair. It locks the slot,
 * allocates a new block from the internal heap, copies the user's 'data'
 * into that block, and updates the slot metadata, marking it as valid.
 *
 * @param[in] pool      A pointer to the initialized kv_pool.
 * @param[in] key       The key (index) of the slot to insert into.
 * @param[in] data      A pointer to the source data to be copied.
 * @param[in] data_size The size (in bytes) of the data to allocate and copy.
 *
 * @return      RESULT_OK on success.
 * @return      RESULT_KEY_EXISTS if the key is already valid (in use).
 * Use kv_pool_write() to overwrite or kv_pool_remove()
 * to delete it first.
 * @return      RESULT_OUT_OF_MEMORY if the internal heap cannot satisfy
 * the 'data_size' allocation request.
 * @return      RESULT_INVALID_KEY if the key is out of bounds
 * (key < 0 or key >= pool->max_keys).
 */
result_t kv_pool_insert(kv_pool *pool, int key, void *data, size_t data_size);

/**
 * @brief Removes a key and deallocates its associated data from the heap.
 *
 * This function safely acquires a lock on the specified key,
 * deallocates its data block from the internal heap (if it exists),
 * and marks the slot as invalid (is_valid = false).
 *
 * @param[in] pool  A pointer to the initialized kv_pool.
 * @param[in] key   The key (index) of the slot to remove.
 *
 * @return      RESULT_OK on success. This is returned even if the key
 * was already invalid.
 * @return      RESULT_INVALID_KEY if the key is out of bounds.
 */
result_t kv_pool_remove(kv_pool *pool, int key);

/**
 * @brief Checks if a key's slot is currently marked as valid (in use).
 *
 * This function performs a thread-safe check by locking the slot,
 * reading its 'is_valid' flag, and immediately releasing the lock.
 *
 * @warning Due to concurrency, a positive result (RESULT_OK) is a
 * "snapshot" and may be stale. Another thread could delete the
 * key immediately after this check. A negative result
 * (RESULT_NOT_FOUND) is always definitively true at the
 * time of the check.
 *
 * @param[in] pool  A pointer to the initialized kv_pool.
 * @param[in] key   The key (index) to check.
 *
 * @return      RESULT_OK if the key is in-bounds and its 'is_valid'
 * flag is true.
 * @return      RESULT_NOT_FOUND if the key is in-bounds but 'is_valid'
 * is false.
 * @return      RESULT_INVALID_KEY if the key is out of bounds.
 */
result_t kv_pool_is_index_valid(kv_pool *pool, int key);

/**
 * @brief Allocates a block of memory from the pool's heap.
 */
result_t kv_pool_allocate(kv_pool *pool, size_t size, void **out_ptr);
/**
 * @brief Frees a previously allocated block back to the pool's heap.
 */
result_t kv_pool_free(kv_pool *pool, void *ptr);
#endif // !KV_POOL_H
