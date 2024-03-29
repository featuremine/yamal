/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file prio_queue.c
 * @date 3 Aug 2020
 * @brief File contains priority queue implementation
 * @see http://www.featuremine.com
 */

#include <fmc/error.h>
#include <fmc/prio_queue.h>
#include <stdlib.h>

void fmc_prio_queue_init(struct fmc_prio_queue_t *q) {
  q->size = 0;
  q->buffer = NULL;
}

void fmc_prio_queue_destroy(struct fmc_prio_queue_t *q) {
  if (q->buffer)
    free(q->buffer);
}

void heapify_up(struct fmc_prio_queue_t *q, size_t i) {
  while (i) {
    size_t parent_index = (i - 1) / 2;
    if (q->buffer[i] <= q->buffer[parent_index]) {
      break;
    }
    int tmp = q->buffer[i];
    q->buffer[i] = q->buffer[parent_index];
    q->buffer[parent_index] = tmp;
    i = parent_index;
  }
}

void fmc_prio_queue_push(struct fmc_prio_queue_t *q, int val, fmc_error_t **e) {
  fmc_error_clear(e);
  q->buffer = (int *)realloc(q->buffer, ++q->size * sizeof(int));
  if (!q->buffer) {
    fmc_error_set2(e, FMC_ERROR_MEMORY);
    return;
  }
  q->buffer[q->size - 1] = val;
  heapify_up(q, q->size - 1);
}

void heapify_down(struct fmc_prio_queue_t *q, size_t i) {
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

bool fmc_prio_queue_pop(struct fmc_prio_queue_t *q, int *out) {
  if (!q->size) {
    return false;
  }
  *out = q->buffer[0];
  q->buffer[0] = q->buffer[--q->size];
  heapify_down(q, 0);
  return true;
}
