#include "kv_pool.h"
#include "logging.h"
#include "result.h"
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>

static char *TAG = "KV POOL";

void lock_mutex(atomic_flag *lock, void (*delay)(void)) {
  while (atomic_flag_test_and_set(lock)) {
    delay();
  }
}

result_t kv_pool_init_fragmented(void *lookup_table, size_t lookup_table_size,
                                 size_t max_keys, void *pool_data,
                                 size_t pool_size, kv_pool *pool,
                                 void (*delay)(void)) {
  if (lookup_table == NULL || pool_data == NULL || pool == NULL ||
      delay == NULL) {
    LOGE(TAG, "Invalid NULL argument(s) to kv_pool_init_fragmented");
    return RESULT_ERR_INVALID_ARG;
  }
  if (max_keys == 0) {
    LOGE(TAG, "max_keys cannot be zero");
    return RESULT_ERR_INVALID_ARG;
  }

  if (pool_size < MINIMUM_BLOCK_SIZE) {
    LOGE(TAG,
         "Pool size too small for heap, need at least %zu bytes but got %zu "
         "bytes",
         MINIMUM_BLOCK_SIZE, pool_size);
    return RESULT_ERR_BUFFER_TOO_SMALL;
  }
  const size_t required_lookup_size = LOOKUP_TABLE_SIZE(max_keys);
  if (required_lookup_size > lookup_table_size) {
    LOGE(TAG,
         "Lookup table size too small for max_keys, need at least %zu bytes "
         "but got %zu bytes",
         required_lookup_size, lookup_table_size); // <-- Use the param
    return RESULT_ERR_BUFFER_TOO_SMALL;
  }
  LOGI(TAG, "Initializing fragmented KV pool with max keys %d", max_keys);

  memset(lookup_table, 0, lookup_table_size);
  memset(pool_data, 0, pool_size);
  LOGI(TAG, "Cleared lookup table and pool data memory");

  pool->lookup_table = (kv_slot *)lookup_table;
  pool->max_keys = max_keys;
  pool->pool_start = pool_data;
  pool->pool_size = pool_size;
  LOGI(TAG, "Setting up heap free list");
  atomic_flag_clear(&pool->heap_lock);
  LOGI(TAG, "Setting delay function");
  pool->delay = delay;
  LOGI(TAG, "Initializing free list head");
  pool->free_list_head = (kv_header *)pool_data;
  LOGI(TAG, "Setting free list head size to %zu", pool_size);
  pool->free_list_head->size = pool_size;
  LOGI(TAG, "Setting free list head next to NULL");
  pool->free_list_head->as.next_free = NULL;
  LOGI(TAG, "Initializing lookup table slots");
  LOGI(TAG, "!!!! sizeof(kv_slot) is %d bytes !!!!", sizeof(kv_slot));

  for (size_t i = 0; i < max_keys; i++) {
    atomic_flag_clear(&pool->lookup_table[i].slot_lock);
    pool->lookup_table[i] = (kv_slot){
        .is_valid = false,
        .data_ptr = NULL,
        .data_size = 0,
    };
  }
  LOGI(TAG, "KV pool initialized successfully");
  return RESULT_OK;
}

result_t kv_pool_init(void *data, size_t data_size, size_t max_keys,
                      kv_pool *pool, void (*delay)(void)) {
  if (data_size < sizeof(kv_slot) * (max_keys) + MINIMUM_BLOCK_SIZE) {
    LOGE(TAG,
         "Data size too small for lookup table and heap, need at least %zu "
         "bytes but got %zu bytes",
         LOOKUP_TABLE_SIZE(max_keys) + MINIMUM_BLOCK_SIZE, data_size);
    return RESULT_ERR_BUFFER_TOO_SMALL;
  }
  LOGI(TAG, "Initializing KV pool with max keys %zu", max_keys);
  return kv_pool_init_fragmented(
      data, LOOKUP_TABLE_SIZE(max_keys), max_keys,
      (void *)((uintptr_t)data + LOOKUP_TABLE_SIZE(max_keys)),
      data_size - LOOKUP_TABLE_SIZE(max_keys), pool, delay);
}

result_t kv_pool_get(kv_pool *pool, int key, void *buffer,
                     size_t *buffer_size) {
  if (pool == NULL || buffer_size == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (key < 0 || (size_t)key >= pool->max_keys) {
    return RESULT_ERR_NOT_FOUND;
  }

  LOGI(TAG, "Getting key %d", key);
  kv_slot *slot = &pool->lookup_table[key];
  // Lock the slot
  lock_mutex(&slot->slot_lock, pool->delay);
  LOGI(TAG, "Locked slot for key %d", key);
  if (!slot->is_valid) {
    atomic_flag_clear(&slot->slot_lock);
    return RESULT_ERR_NOT_FOUND;
  }

  if (*buffer_size < slot->data_size) {
    *buffer_size = slot->data_size;
    atomic_flag_clear(&slot->slot_lock);
    return RESULT_ERR_BUFFER_TOO_SMALL;
  }

  memcpy(buffer, slot->data_ptr, slot->data_size);
  *buffer_size = slot->data_size;

  // Release the lock
  atomic_flag_clear(&slot->slot_lock);
  return RESULT_OK;
}

/**
 * @brief (Internal) Allocates a block of memory from the pool's heap.
 *
 * This funciton implements a first fit search of the heap's free list
 */
result_t kv_pool_allocate(kv_pool *pool, size_t size, void **out_ptr) {
  if (pool == NULL || out_ptr == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  *out_ptr = NULL;
  if (size == 0) {
    return RESULT_ERR_INVALID_ARG;
  }
  const size_t data_offset = offsetof(kv_header, as.data);

  size_t total_size = ALIGN(data_offset + size);

  if (total_size < MINIMUM_BLOCK_SIZE) {
    total_size = MINIMUM_BLOCK_SIZE;
  }

  lock_mutex(&pool->heap_lock, pool->delay);

  kv_header *current = pool->free_list_head;
  kv_header *previous = NULL;
  while (current != NULL && current->size < total_size) {
    previous = current;
    current = current->as.next_free;
  }
  if (current == NULL) {
    atomic_flag_clear(&pool->heap_lock);
    return RESULT_ERR_NO_MEM;
  }
  atomic_flag_clear(&pool->heap_lock);

  size_t remaining_size = current->size - total_size;
  // Split the block if there's enough space left over
  if (remaining_size >= MINIMUM_BLOCK_SIZE) {
    kv_header *new_free_block = (kv_header *)((uintptr_t)current + total_size);
    new_free_block->size = remaining_size;
    new_free_block->as.next_free = current->as.next_free;

    if (previous == NULL) {
      pool->free_list_head = new_free_block;
    } else {
      previous->as.next_free = new_free_block;
    }
    current->size = total_size;
  } else {
    if (previous == NULL) {
      pool->free_list_head = current->as.next_free;
    } else {
      previous->as.next_free = current->as.next_free;
    }
  }
  atomic_flag_clear(&pool->heap_lock);
  *out_ptr = (void *)&current->as.data;
  return RESULT_OK;
}

/**
 * @brief (Internal) Frees a previously allocated block back to the pool's heap.
 */
result_t kv_pool_free(kv_pool *pool, void *ptr) {
  if (pool == NULL || ptr == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  kv_header *block_to_free =
      (kv_header *)((uintptr_t)ptr - offsetof(kv_header, as.data));

  lock_mutex(&pool->heap_lock, pool->delay);
  kv_header *current = pool->free_list_head;
  kv_header *previous = NULL;
  while (current != NULL && current < block_to_free) {
    previous = current;
    current = current->as.next_free;
  }
  // Coalewsc & Relink
  if (current != NULL &&
      (char *)block_to_free + block_to_free->size == (char *)current) {
    block_to_free->size += current->size;
    block_to_free->as.next_free = current->as.next_free;
  } else {
    block_to_free->as.next_free = current;
  }
  if (previous != NULL &&
      (char *)previous + previous->size == (char *)block_to_free) {
    previous->size += block_to_free->size;
    previous->as.next_free = block_to_free->as.next_free;
  } else {
    if (previous == NULL) {
      pool->free_list_head = block_to_free;
    } else {
      previous->as.next_free = block_to_free;
    }
  }
  for (int i = 0; i < pool->max_keys; i++) {
    kv_slot *slot = &pool->lookup_table[i];
    if (slot->data_ptr == ptr) {
      lock_mutex(&slot->slot_lock, pool->delay);
      slot->data_ptr = NULL;
      slot->data_size = 0;
      slot->is_valid = false;
      atomic_flag_clear(&slot->slot_lock);
      break;
    }
  }

  atomic_flag_clear(&pool->heap_lock);
  return RESULT_OK;
}

result_t kv_pool_write(kv_pool *pool, int key, void *buffer,
                       size_t buffer_size) {
  if (pool == NULL || buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  TRY(kv_pool_is_index_valid(pool, key));
  result_t err;
  kv_slot *slot = &pool->lookup_table[key];
  lock_mutex(&slot->slot_lock, pool->delay);
  if (slot->data_size != buffer_size) {
    LOGE(TAG,
         "Size mismatch on kv_pool_write: existing size=%zu, provided "
         "size=%zu",
         slot->data_size, buffer_size);
    err = RESULT_ERR_INVALID_ARG;
    goto cleanup;
  }
  memcpy(slot->data_ptr, buffer, buffer_size);
  err = RESULT_OK;
cleanup:
  atomic_flag_clear(&slot->slot_lock);
  return err;
}

result_t kv_pool_insert(kv_pool *pool, int key, void *data, size_t data_size) {
  if (pool == NULL || data == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (key < 0 || (size_t)key >= pool->max_keys) {
    LOGE(TAG, "Invalid key %d for kv_pool_insert", key);
    return RESULT_ERR_INVALID_ARG;
  }
  kv_slot *slot = &pool->lookup_table[key];
  // lock_mutex(&slot->slot_lock, pool->delay);
  while (atomic_flag_test_and_set(&slot->slot_lock)) {
    pool->delay();
  }
  slot->is_valid = false; // Mark invalid during insert
  slot->data_size = 0;
  slot->data_ptr = NULL;
  TRY_LOG_CLEAN(kv_pool_allocate(pool, data_size, &slot->data_ptr));
  lock_mutex(&pool->heap_lock, pool->delay);
  memcpy(slot->data_ptr, data, data_size);
cleanup:
  atomic_flag_clear(&pool->heap_lock);
  if (slot->data_ptr != NULL) {
    slot->data_size = data_size;
    slot->is_valid = true;
    atomic_flag_clear(&slot->slot_lock);
    return RESULT_OK;
  } else {
    atomic_flag_clear(&slot->slot_lock);
    return RESULT_ERR_NO_MEM;
  }
}

result_t kv_pool_is_index_valid(kv_pool *pool, int key) {
  if (pool == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (key < 0 || (size_t)key >= pool->max_keys) {
    return RESULT_ERR_INVALID_ARG;
  }
  kv_slot *slot = &pool->lookup_table[key];
  result_t err;
  lock_mutex(&slot->slot_lock, pool->delay);
  if (slot == NULL) {
    err = RESULT_ERR_NOT_FOUND;
    goto cleanup;
  }
  if (!slot->is_valid) {
    err = RESULT_ERR_NOT_FOUND;
    goto cleanup;
  }
  err = RESULT_OK;
cleanup:
  atomic_flag_clear(&slot->slot_lock);
  return err;
}

result_t kv_pool_remove(kv_pool *pool, int key) {
  if (pool == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  TRY(kv_pool_is_index_valid(pool, key));
  kv_slot *slot = &pool->lookup_table[key];
  return kv_pool_free(pool, slot->data_ptr);
}
