#include "kv_pool.h"
#include "logging.h"
#include "result.h"
#include <pthread.h>
#include <string.h>

const static char *TAG = "KV POOL";

result_t kv_pool_init_fragmented(void *lookup_table, size_t max_keys,
                                 void *pool_data, size_t pool_size,
                                 kv_pool *pool, void (*delay)(void)) {
  if (lookup_table == NULL || pool_data == NULL || pool == NULL ||
      delay == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (max_keys == 0) {
    return RESULT_ERR_INVALID_ARG;
  }

  if (pool_size < MINIMUM_BLOCK_SIZE) {
    return RESULT_ERR_BUFFER_TOO_SMALL;
  }
  if (LOOKUP_TABLE_SIZE(max_keys) > sizeof(lookup_table)) {
    return RESULT_ERR_BUFFER_TOO_SMALL;
  }

  memset(lookup_table, 0, sizeof(lookup_table));
  memset(pool_data, 0, pool_size);

  pool->lookup_table = (kv_slot *)lookup_table;
  pool->max_keys = max_keys;
  pool->pool_start = pool_data;
  pool->pool_size = pool_size;
  atomic_flag_clear(&pool->heap_lock);
  pool->delay = delay;
  pool->free_list_head = (kv_header *)pool_data;
  pool->free_list_head->size = pool_size;
  pool->free_list_head->as.next_free = NULL;

  for (size_t i = 0; i < max_keys; i++) {
    atomic_flag_clear(&pool->lookup_table[i].slot_lock);
    pool->lookup_table[i].is_valid = false;
    pool->lookup_table[i].data_size = 0;
    pool->lookup_table[i].data_ptr = NULL;
  }
  return RESULT_OK;
}

result_t kv_pool_init(void *data, size_t data_size, size_t max_keys,
                      kv_pool *pool, void (*delay)(void)) {
  if (data_size < LOOKUP_TABLE_SIZE(max_keys) + MINIMUM_BLOCK_SIZE) {
    return RESULT_ERR_BUFFER_TOO_SMALL;
  }
  return kv_pool_init_fragmented(
      data, max_keys, (void *)((uintptr_t)data + LOOKUP_TABLE_SIZE(max_keys)),
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

  kv_slot *slot = &pool->lookup_table[key];

  // Lock the slot
  while (atomic_flag_test_and_set(&slot->slot_lock)) {
    pool->delay();
  }

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
  while (atomic_flag_test_and_set(&pool->heap_lock)) {
    pool->delay();
  }

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
  atomic_fetch_clear(&pool->heap_lock);

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

  while (atomic_flag_test_and_set(&pool->heap_lock)) {
    pool->delay();
  }

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

  atomic_flag_clear(&pool->heap_lock);
  return RESULT_OK;
}
