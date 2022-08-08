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

#include <uthash/utarray.h>

#define _utheap_heapify_up(a, index, cmp)                                              \
  do {                                                                                 \
    size_t idx = index;                                                                \
    while (idx) {                                                                      \
      size_t parent_index = (idx - 1) / 2;                                             \
      if (!cmp(utarray_eltptr(a, idx), utarray_eltptr(a, parent_index))) {             \
        break;                                                                         \
      }                                                                                \
      ut_swap(_utarray_eltptr(a, idx), _utarray_eltptr(a, parent_index), (a)->icd.sz); \
      idx = parent_index;                                                              \
    }                                                                                  \
  } while (0)

#define utheap_push(a, val, cmp)                                               \
  do {                                                                         \
    utarray_push_back(a, val);                                                 \
    _utheap_heapify_up(a, (a)->i - 1, cmp);                                    \
  } while (0)

#define _utheap_heapify_down(a, index, cmp)                                       \
  do {                                                                            \
    size_t idx = index;                                                           \
    while (1) {                                                                   \
      size_t left = 2 * idx;                                                      \
      size_t right = 2 * idx + 1;                                                 \
      size_t largest = idx;                                                       \
      if (left <= (a)->i - 1 &&                                                   \
          cmp(utarray_eltptr(a, left), utarray_eltptr(a, largest))) {             \
        largest = left;                                                           \
      }                                                                           \
      if (right <= (a)->i - 1 &&                                                  \
          cmp(utarray_eltptr(a, right), utarray_eltptr(a, largest))) {            \
        largest = right;                                                          \
      }                                                                           \
      if (largest == idx) {                                                       \
        break;                                                                    \
      }                                                                           \
      ut_swap(_utarray_eltptr(a, idx), _utarray_eltptr(a, largest), (a)->icd.sz); \
    }                                                                             \
  } while (0)

#define utheap_pop(a, cmp)                                                     \
  do {                                                                         \
    if ((a)->i) {                                                              \
      if ((a)->icd.copy) {                                                     \
        (a)->icd.copy(utarray_eltptr(a, 0), utarray_eltptr(a, (a)->i - 1));    \
      } else {                                                                 \
        memcpy(_utarray_eltptr(a, 0), _utarray_eltptr(a, (a)->i - 1),          \
               (a)->icd.sz);                                                   \
      };                                                                       \
      utarray_resize(a, (a)->i-1);                                             \
      if (utarray_len(a)) {                                                    \
        _utheap_heapify_down(a, 0, cmp);                                       \
      }                                                                        \
    }                                                                          \
  } while (0)
