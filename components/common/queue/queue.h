#include "stdint.h"
#include "result.h"
#include "FreeRTOS.h"
#include "semphr.h"



/**
 * @brief Fixed-capacity queue with byte-addressable backing storage.
 *
 * @details
 * This queue stores fixed-size items in a user-provided buffer (no dynamic
 * allocation for storage). Concurrency between tasks is protected by a FreeRTOS
 * mutex. This implementation is task-thread-safe but not ISR-safe.
 *
 * @warning Not ISR-safe: do not call push/pop/peek from an interrupt context.
 */
typedef struct {
    uint16_t head;                 /**< Index of next element to pop/peek. */
    uint16_t tail;                 /**< Index of next slot to push into. */
    uint16_t count;                /**< Number of elements currently stored. */
    uint16_t capacity;             /**< Maximum number of elements the queue can hold. */
    size_t   item_size;            /**< Size in bytes of each element. */
    uint8_t *buffer;               /**< Backing storage: capacity * item_size bytes. */
    SemaphoreHandle_t lock;        /**< FreeRTOS mutex protecting all operations. */
} queue_t;

/**
 * @brief Initialize a queue with caller-provided backing storage.
 *
 * @param q         Queue instance to initialize.
 * @param buffer    Backing storage pointer (must be at least capacity * item_size bytes).
 * @param buffer_size Size of the provided buffer in bytes.
 * @param capacity  Number of elements the queue can hold.
 * @param item_size Size in bytes of each element.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if any argument is invalid.
 * @return RESULT_ERR_NO_MEM if the mutex could not be created.
 *
 * @note This function creates a FreeRTOS mutex.
 * @warning Not ISR-safe.
 */
static inline result_t queue_init(queue_t *q,
                                  void *buffer,
                                    size_t buffer_size,
                                  uint16_t capacity,
                                  size_t item_size)
{
    if (!q || !buffer || capacity == 0U || item_size == 0U)
    {
        return RESULT_ERR_INVALID_ARG;
    }
    if (capacity*item_size > buffer_size) {
        return RESULT_ERR_INVALID_ARG;
    }

    q->head = 0U;
    q->tail = 0U;
    q->count = 0U;
    q->capacity = capacity;
    q->item_size = item_size;
    q->buffer = (uint8_t *)buffer;
    q->mutex = xSemaphoreCreateMutex();
    if (q->mutex == NULL) {
        return RESULT_ERR_NO_MEM;
    }

    return RESULT_OK;
}

/**
 * @brief Push an item into the queue.
 *
 * @param q    Queue instance.
 * @param item Pointer to the item to copy into the queue.
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if q or item is NULL.
 * @return RESULT_ERR_OVERFLOW if the queue is full.
 * @return RESULT_ERR_INTERNAL if the mutex could not be taken.
 *
 * @warning Not ISR-safe (may block).
 */
static inline result_t queue_push(queue_t *q, const void *item){
    if (!q || !item)
    {
        return RESULT_ERR_INVALID_ARG;
    }
    if (xSemaphoreTake(q->mutex, portMAX_DELAY) != pdTRUE) {
        return RESULT_ERR_MUTEX;
    }

    if (q->count == q->capacity)
    {
        (void)xSemaphoreGive(q->mutex);
        return RESULT_ERR_OVERFLOW;
    }

    memcpy(&q->buffer[q->tail * q->item_size],
           item,
           q->item_size);

    q->tail++;
    if (q->tail == q->capacity)
    {
        q->tail = 0U;
    }

    q->count++;
    (void)xSemaphoreGive(q->mutex);
    return RESULT_OK;
}

/**
 * @brief Pop (remove) the oldest item from the queue.
 *
 * @param q    Queue instance.
 * @param item Output buffer to receive the popped item (size >= item_size).
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if q or item is NULL.
 * @return RESULT_ERR_NOT_FOUND if the queue is empty.
 * @return RESULT_ERR_INTERNAL if the mutex could not be taken.
 *
 * @warning Not ISR-safe (may block).
 */
static inline result_t queue_pop(queue_t *q, void *item)
{
    if (!q || !item) return RESULT_ERR_INVALID_ARG;

    if (xSemaphoreTake(q->mutex, portMAX_DELAY) != pdTRUE) {
        return RESULT_ERR_INTERNAL;
    }

    if (q->count == 0U) {
        (void)xSemaphoreGive(q->lock);
        return RESULT_ERR_NOT_FOUND;
    }

    memcpy(item, &q->buffer[q->head * q->item_size], q->item_size);

    q->head++;
    if (q->head == q->capacity) q->head = 0U;

    q->count--;

    (void)xSemaphoreGive(q->lock);
    return RESULT_OK;
}

/**
 * @brief Peek the oldest item in the queue without removing it.
 *
 * @param q    Queue instance.
 * @param item Output buffer to receive the peeked item (size >= item_size).
 *
 * @return RESULT_OK on success.
 * @return RESULT_ERR_INVALID_ARG if q or item is NULL.
 * @return RESULT_ERR_NOT_FOUND if the queue is empty.
 * @return RESULT_ERR_INTERNAL if the mutex could not be taken.
 *
 * @note The peek is not an atomic "peek+pop" operation. Another task may pop the
 * item after peek returns.
 *
 * @warning Not ISR-safe (may block).
 */
static inline result_t queue_peek(queue_t *q, void *item)
{
    if (!q || !item) {
        return RESULT_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(q->lock, portMAX_DELAY) != pdTRUE) {
        return RESULT_ERR_INTERNAL;
    }

    if (q->count == 0U) {
        (void)xSemaphoreGive(q->lock);
        return RESULT_ERR_NOT_FOUND;
    }

    memcpy(item,
           &q->buffer[q->head * q->item_size],
           q->item_size);

    (void)xSemaphoreGive(q->lock);
    return RESULT_OK;
}
