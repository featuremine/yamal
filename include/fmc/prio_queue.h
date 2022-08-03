#include <stdlib.h>
#include <fmc/error.h>

struct prio_queue {
  size_t size;
  size_t count;
  int buffer[];
};

struct prio_queue * prio_queue_new(size_t initial_size, fmc_error_t **e) {
  struct prio_queue * q = (struct prio_queue*)calloc(1, sizeof(prio_queue) + initial_size * sizeof(int));
  q->size = initial_size;
  q->count = 0;
  return q;
}

void prio_queue_del(struct prio_queue *q) {
  free(q);
}

void heapify_up(struct prio_queue* q, size_t i) {
  size_t parent_index = (i - 1) / 2;
  size_t left = 2 * parent_index;
  size_t right = 2 * parent_index - 1;
  if (i == left) {
    if (q->buffer[parent_index - 1] < q->buffer[q->count - 1]) {
      heapify_up(q, parent_index - 1);
      int tmp = q->buffer[i - 1];
      q->buffer[i - 1] = q->buffer[parent_index - 1];
      q->buffer[parent_index - 1] = tmp;
    }
  } else if (q->buffer[parent_index - 1] >= q->buffer[q->count - 1]) {
    int tmp = q->buffer[i - 1];
    q->buffer[i - 1] = q->buffer[parent_index - 1];
    q->buffer[parent_index - 1] = tmp;
    heapify_up(q, parent_index - 1);
  }
}

void prio_queue_insert(struct prio_queue **q, int val) {
  if ((*q)->count == (*q)->size) {
    (*q)->size = (*q)->size * 2;
    *q = (struct prio_queue*)realloc(*q, sizeof(prio_queue) + q->size * sizeof(int));
  }
  (*q)->buffer[(*q)->count] = val;
  if (++(*q)->count) {
    heapify_up((*q), (*q)->count);
  }
}

void heapify_down(struct prio_queue* q, size_t i) {
  size_t left = 2 * i;
  size_t right = 2 * i - 1;
  size_t largest = i;
  if (left <= q->count && q->buffer[left - 1] > q->buffer[largest - 1]) {
    largest = left;
  }
  if (right <= q->count && q->buffer[right - 1] > q->buffer[largest - 1]) {
    largest = right;
  }
  if (largest != i) {
    int tmp = q->buffer[i - 1];
    q->buffer[i - 1] = q->buffer[largest - 1];
    q->buffer[largest - 1] = tmp;
    heapify_down(q, largest);
  }
}

int prio_queue_pop(struct prio_queue *q, fmc_error_t **e) {
  if (!q->count) {
    fmc_error_set2(e, FMC_ERROR_MEMORY);
    return 0;
  }
  int ret = q->buffer[0];
  q->buffer[0] = q->buffer[--q->count];
  heapify_down(q, 1);
  return ret;
}

