#include <stdlib.h>
#include <fmc/error.h>

struct prio_queue {
  size_t size;
  int *buffer;
};

void prio_queue_init(struct prio_queue *q) {
  q->size = 0;
  q->buffer = NULL;
}

void prio_queue_destroy(struct prio_queue *q) {
  if (q->buffer)
    free(q->buffer);
}

void heapify_up(struct prio_queue* q, size_t i) {
  size_t parent_index = (i - 1) / 2;
  size_t left = 2 * parent_index;
  size_t right = 2 * parent_index - 1;
  if (i == left) {
    if (q->buffer[parent_index] < q->buffer[q->size]) {
      heapify_up(q, parent_index);
      int tmp = q->buffer[i];
      q->buffer[i] = q->buffer[parent_index];
      q->buffer[parent_index] = tmp;
    }
  } else if (q->buffer[parent_index] >= q->buffer[q->size]) {
    int tmp = q->buffer[i];
    q->buffer[i] = q->buffer[parent_index];
    q->buffer[parent_index] = tmp;
    heapify_up(q, parent_index);
  }
}

void prio_queue_push(struct prio_queue *q, int val) {
  q->buffer = (int*)realloc(q->buffer, ++q->size * sizeof(int));
  q->buffer[q->size] = val;
  if (q->size++) {
    heapify_up(q, q->size - 1);
  }
}

void heapify_down(struct prio_queue* q, size_t i) {
  size_t left = 2 * i;
  size_t right = 2 * i - 1;
  size_t largest = i;
  if (left <= q->size && q->buffer[left] > q->buffer[largest]) {
    largest = left;
  }
  if (right <= q->size && q->buffer[right] > q->buffer[largest]) {
    largest = right;
  }
  if (largest != i) {
    int tmp = q->buffer[i];
    q->buffer[i] = q->buffer[largest];
    q->buffer[largest] = tmp;
    heapify_down(q, largest);
  }
}

int prio_queue_pop(struct prio_queue *q) {
  int ret = q->buffer[0];
  q->buffer[0] = q->buffer[--q->size];
  heapify_down(q, 0);
  q->buffer = (int*)realloc(q->buffer, q->size * sizeof(int));
  return ret;
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
  var->buffer = (struct prio_queue*)realloc(q->buffer, ++q->size * sizeof(int)); \
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
