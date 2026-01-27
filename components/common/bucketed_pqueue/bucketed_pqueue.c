
#include "bucketed_pqueue.h"

result_t bucketed_pqueue_init(bucketed_pqueue_t *pqueue, QueueHandle_t *buckets,
                              uint8_t num_buckets, TaskHandle_t notifier) {
  if (!pqueue || !buckets || num_buckets == 0U || num_buckets > 32U) {
    return RESULT_ERR_INVALID_ARG;
  }

  pqueue->buckets = buckets;
  pqueue->num_buckets = num_buckets;
  pqueue->non_empty_mask = 0U;
  pqueue->notifier = notifier;

  return RESULT_OK;
}

result_t bucketed_pqueue_push(bucketed_pqueue_t *bqueue, uint8_t prio,
                              void *item, TickType_t ticks_to_wait) {
  if (!bqueue || !item || prio >= bqueue->num_buckets) {
    return RESULT_ERR_INVALID_ARG;
  }

  if (xQueueSend(bqueue->buckets[prio], item, ticks_to_wait) != pdPASS) {
    return RESULT_ERR_OVERFLOW;
  }

  taskENTER_CRITICAL();
  bqueue->non_empty_mask |= (1UL << (uint32_t)prio);
  taskEXIT_CRITICAL();

  if (bqueue->notifier != NULL) {
    (void)xTaskNotify(bqueue->notifier, (1UL << (uint32_t)prio), eSetBits);
  }

  return RESULT_OK;
}

result_t bucketed_pqueue_push_from_isr(bucketed_pqueue_t *bqueue, uint8_t prio,
                                       const void *item,
                                       BaseType_t *higher_woken) {
  if (!bqueue || !item || !higher_woken || prio >= bqueue->num_buckets) {
    return RESULT_ERR_INVALID_ARG;
  }

  if (xQueueSendFromISR(bqueue->buckets[prio], item, higher_woken) != pdPASS) {
    return RESULT_ERR_OVERFLOW;
  }

  UBaseType_t s = taskENTER_CRITICAL_FROM_ISR();
  bqueue->non_empty_mask |= (1UL << (uint32_t)prio);
  taskEXIT_CRITICAL_FROM_ISR(s);

  if (bqueue->notifier != NULL) {
    (void)xTaskNotifyFromISR(bqueue->notifier, (1UL << (uint32_t)prio),
                             eSetBits, higher_woken);
  }

  return RESULT_OK;
}

result_t bucketed_pqueue_pop(bucketed_pqueue_t *bq, void *out) {
  if (!bq || !out) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (bq->num_buckets == 0U) {
    return RESULT_ERR_NOT_FOUND;
  }

  uint32_t mask_snapshot;

  taskENTER_CRITICAL();
  mask_snapshot = bq->non_empty_mask;
  taskEXIT_CRITICAL();

  if (mask_snapshot == 0U) {
    return RESULT_ERR_NOT_FOUND;
  }

  for (int prio = (int)bq->num_buckets - 1; prio >= 0; prio--) {
    uint32_t bit = (1UL << (uint32_t)prio);

    if ((mask_snapshot & bit) == 0U) {
      continue;
    }

    if (xQueueReceive(bq->buckets[prio], out, 0) == pdPASS) {
      if (uxQueueMessagesWaiting(bq->buckets[prio]) == 0U) {
        taskENTER_CRITICAL();
        bq->non_empty_mask &= ~bit;
        taskEXIT_CRITICAL();
      }
      return RESULT_OK;
    }

    /* Repair stale bitmap entry */
    taskENTER_CRITICAL();
    bq->non_empty_mask &= ~bit;
    taskEXIT_CRITICAL();
  }

  return RESULT_ERR_NOT_FOUND;
}

result_t bucketed_pqueue_peek(bucketed_pqueue_t *bq, void *out) {
  if (!bq || !out) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (bq->num_buckets == 0U) {
    return RESULT_ERR_NOT_FOUND;
  }

  uint32_t mask_snapshot;

  taskENTER_CRITICAL();
  mask_snapshot = bq->non_empty_mask;
  taskEXIT_CRITICAL();

  if (mask_snapshot == 0U) {
    return RESULT_ERR_NOT_FOUND;
  }

  for (int prio = (int)bq->num_buckets - 1; prio >= 0; prio--) {
    uint32_t bit = (1UL << (uint32_t)prio);

    if ((mask_snapshot & bit) == 0U) {
      continue;
    }

    if (xQueuePeek(bq->buckets[prio], out, 0) == pdPASS) {
      return RESULT_OK;
    }

    /* Repair stale bitmap entry */
    taskENTER_CRITICAL();
    bq->non_empty_mask &= ~bit;
    taskEXIT_CRITICAL();
  }

  return RESULT_ERR_NOT_FOUND;
}
