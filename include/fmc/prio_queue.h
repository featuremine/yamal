/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file prio_queue.h
 * @date 3 Aug 2020
 * @brief File contains priority queue interface
 * @see http://www.featuremine.com
 */

#include <stdlib.h>
#include <fmc/error.h>

struct fmc_prio_queue_t {
  size_t size;
  int *buffer;
};

void fmc_prio_queue_init(struct fmc_prio_queue_t *q) {
  q->size = 0;
  q->buffer = NULL;
}

void fmc_prio_queue_destroy(struct fmc_prio_queue_t *q) {
  if (q->buffer)
    free(q->buffer);
}

void heapify_up(struct fmc_prio_queue_t* q, size_t i) {
  while (i) {
    size_t parent_index = (i - 1) / 2;
    if (q->buffer[i] > q->buffer[parent_index]) {
      int tmp = q->buffer[i];
      q->buffer[i] = q->buffer[parent_index];
      q->buffer[parent_index] = tmp;
      i = parent_index;
      continue;
    }
    i = 0;
  }
}

void fmc_prio_queue_push(struct fmc_prio_queue_t *q, int val, fmc_error_t **e) {
  fmc_error_clear(e);
  q->buffer = (int*)realloc(q->buffer, ++q->size * sizeof(int));
  if (!q->buffer) {
    fmc_error_set2(e, FMC_ERROR_MEMORY);
    return;
  }
  q->buffer[q->size - 1] = val;
  heapify_up(q, q->size - 1);
}

void heapify_down(struct fmc_prio_queue_t* q, size_t i) {
  while (1) {
    size_t left = 2 * i;
    size_t right = 2 * i + 1;
    size_t largest = i;
    if (left <= q->size && q->buffer[left] > q->buffer[largest]) {
      largest = left;
    }
    if (right <= q->size && q->buffer[right] > q->buffer[largest]) {
      largest = right;
    }
    if (largest == i) {
      break;
    }
    int tmp = q->buffer[i];
    q->buffer[i] = q->buffer[largest];
    q->buffer[largest] = tmp;
  }
}

bool fmc_prio_queue_pop(struct fmc_prio_queue_t *q, int*out) {
  if (!q->size) {
    return false;
  }
  *out = q->buffer[0];
  q->buffer[0] = q->buffer[--q->size];
  heapify_down(q, 0);
  return true;
}

#define PRIO_QUEUE_NEW(type, var)                          \
  struct {                                                            \
    size_t size;                                                      \
    type *buffer;                                                     \
  } var;                                                             \

#define PRIO_QUEUE_INIT(var) \
  var->size = 0; \
  var->buffer = NULL

#define PRIO_QUEUE_DESTROY(var) \
  if (var->buffer) {\
    free(var->buffer); \
  }

#define HEAPIFY_UP(var, i)\
{                                                                            \
  size_t parent_index = (i - 1) / 2;                                         \
  size_t left = 2 * parent_index;                                            \
  size_t right = 2 * parent_index - 1;                                       \
  if (i == left) {                                                           \
    if (var->buffer[parent_index] < var->buffer[var->size - 1]) {       \
      HEAPIFY_UP(var, parent_index);                                     \
      int tmp = var->buffer[i];                                          \
      var->buffer[i] = var->buffer[parent_index];                    \
      var->buffer[parent_index] = tmp;                                   \
    }                                                                        \
  } else if (var->buffer[parent_index] >= var->buffer[var->size - 1]) { \
    int tmp = var->buffer[i];                                            \
    var->buffer[i] = var->buffer[parent_index];                      \
    var->buffer[parent_index] = tmp;                                     \
    HEAPIFY_UP(var, parent_index);                                       \
  }                                                                          \
}

#define PRIO_QUEUE_PUSH(var, val)                                                      \
  var->buffer = (struct fmc_prio_queue_t*)realloc(q->buffer, ++q->size * sizeof(int)); \
  var->buffer[q->size] = val; \
  memcpy(&var->buffer[var->size], &val, sizeof(val));                                 \
  if (++var->size) {                                                                  \
    HEAPIFY_UP(var, var->size);                                                       \
  }

#define PRIO_QUEUE_FRONT(var) \
  var->buffer[0]

#define HEAPIFY_DOWN(var, i)                                                      \
{                                                                                 \
  size_t left = 2 * i;                                                            \
  size_t right = 2 * i - 1;                                                       \
  size_t largest = i;                                                             \
  if (left <= var->size && var->buffer[left] > var->buffer[largest]) {   \
    largest = left;                                                               \
  }                                                                               \
  if (right <= var->size && var->buffer[right] > var->buffer[largest]) { \
    largest = right;                                                              \
  }                                                                               \
  if (largest != i) {                                                             \
    int tmp = var->buffer[i];                                                 \
    var->buffer[i] = var->buffer[largest];                                \
    var->buffer[largest] = tmp;                                               \
    HEAPIFY_DOWN(var, largest);                                                   \
  }                                                                               \
}

#define PRIO_QUEUE_POP(var)                   \
  var->buffer[0] = var->buffer[--var->size]; \
  HEAPIFY_DOWN(var, 1);
