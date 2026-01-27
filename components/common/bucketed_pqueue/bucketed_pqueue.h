#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "result.h"
#include "task.h"

/**
 * @file bucketed_pqueue.h
 * @brief Bucketed priority queue built on FreeRTOS queues.
 *
 * @details
 * This module implements a strict-priority queue by composing multiple
 * FreeRTOS FIFO queues into priority "buckets".
 *
 * - Bucket 0 is the lowest priority
 * - Bucket (num_buckets - 1) is the highest priority
 *
 * A bitmap is maintained to track which buckets are non-empty, allowing
 * efficient selection of the highest-priority available item.
 *
 * Threading model:
 * - Multiple producers supported (task and ISR context)
 * - Intended for a single consumer task
 *
 * Limitations:
 * - Not reentrant for multiple consumers
 * - num_buckets is limited to 32
 */

/**
 * @brief Bucketed priority queue built from FreeRTOS queues.
 *
 * @details
 * - Each priority bucket is a FreeRTOS FIFO queue (QueueHandle_t).
 * - Priority is discrete: bucket 0 is lowest, (num_buckets - 1) is highest.
 * - A bitmap tracks which buckets are believed non-empty to avoid scanning.
 *
 * Concurrency:
 * - Producers may be tasks or ISRs (use the appropriate push function).
 * - Consumer is intended to be a task (pop/peek are task APIs).
 *
 * Optional wakeup:
 * - If notifier is set, successful pushes will notify that task with a bit
 *   corresponding to the bucket index (1UL << prio).
 *
 * Limits:
 * - num_buckets must be 1..32.
 */
typedef struct {
  QueueHandle_t *buckets; /**< Array of FreeRTOS queues (length num_buckets). */
  uint8_t num_buckets;    /**< Number of buckets (1..32). */
  uint32_t non_empty_mask; /**< Bitmap: bit n set => bucket n is non-empty. */
  TaskHandle_t notifier;   /**< Task to notify on push, or NULL. */
} bucketed_pqueue_t;

/**
 * @brief Initialize a bucketed priority queue.
 *
 * @param[out] pqueue     Bucketed queue instance to initialize.
 * @param[in]  buckets    Array of FreeRTOS queue handles. Must remain valid for
 *                        the lifetime of the priority queue.
 * @param[in]  num_buckets Number of priority buckets (1..32).
 * @param[in]  notifier   Optional task to notify on enqueue (NULL to disable).
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if arguments are invalid.
 *
 * @note Buckets must already be created using xQueueCreate() or
 *       xQueueCreateStatic().
 */
result_t bucketed_pqueue_init(bucketed_pqueue_t *pqueue, QueueHandle_t *buckets,
                              uint8_t num_buckets, TaskHandle_t notifier);

/**
 * @brief Push an item into a priority bucket (task context).
 *
 * @param[in] bqueue        Bucketed queue instance.
 * @param[in] prio          Bucket index (0..num_buckets-1).
 * @param[in] item          Pointer to item to enqueue (copied by FreeRTOS
 * queue).
 * @param[in] ticks_to_wait Ticks to wait if the bucket queue is full.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if arguments are invalid.
 * @return RESULT_ERR_OVERFLOW if the bucket queue is full or times out.
 *
 * @warning Must not be called from ISR context.
 */
result_t bucketed_pqueue_push(bucketed_pqueue_t *bqueue, uint8_t prio,
                              void *item, TickType_t ticks_to_wait);

/**
 * @brief Push an item into a priority bucket (ISR context).
 *
 * @param[in]  bqueue        Bucketed queue instance.
 * @param[in]  prio          Bucket index (0..num_buckets-1).
 * @param[in]  item          Pointer to item to enqueue.
 * @param[out] higher_woken  Set to pdTRUE if a higher-priority task should be
 * woken.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if arguments are invalid.
 * @return RESULT_ERR_OVERFLOW if the bucket queue is full.
 *
 * @note The caller must call portYIELD_FROM_ISR(*higher_woken) if set.
 */
result_t bucketed_pqueue_push_from_isr(bucketed_pqueue_t *bqueue, uint8_t prio,
                                       const void *item,
                                       BaseType_t *higher_woken);

/**
 * @brief Pop the highest-priority available item (task context).
 *
 * @param[in]  bq   Bucketed queue instance.
 * @param[out] out  Output buffer to receive the popped item.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if arguments are invalid.
 * @return RESULT_ERR_NOT_FOUND if all buckets are empty.
 *
 * @note This function is non-blocking.
 */
result_t bucketed_pqueue_pop(bucketed_pqueue_t *bq, void *out);

/**
 * @brief Peek the highest-priority available item without removing it.
 *
 * @param[in]  bq   Bucketed queue instance.
 * @param[out] out  Output buffer to receive the peeked item.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if arguments are invalid.
 * @return RESULT_ERR_NOT_FOUND if all buckets are empty.
 *
 * @note This function is non-blocking.
 */
result_t bucketed_pqueue_peek(bucketed_pqueue_t *bq, void *out);
