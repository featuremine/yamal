/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file prio_queue.h
 * @date 3 Aug 2020
 * @brief File contains priority queue interface
 * @see http://www.featuremine.com
 */

#include <uthash/utarray.h>

#define _utheap_heapify_up(a, index, cmp)                                      \
  do {                                                                         \
    size_t idx = index;                                                        \
    while (idx) {                                                              \
      size_t parent_index = (idx - 1) / 2;                                     \
      if (cmp(utarray_eltptr(a, parent_index), utarray_eltptr(a, idx))) {      \
        break;                                                                 \
      }                                                                        \
      ut_swap(_utarray_eltptr(a, idx), _utarray_eltptr(a, parent_index),       \
              (a)->icd.sz);                                                    \
      idx = parent_index;                                                      \
    }                                                                          \
  } while (0)

#define utheap_push(a, val, cmp)                                               \
  do {                                                                         \
    utarray_push_back(a, val);                                                 \
    _utheap_heapify_up(a, (a)->i - 1, cmp);                                    \
  } while (0)

#define _utheap_heapify_down(a, index, cmp)                                    \
  do {                                                                         \
    size_t idx =                                                               \
        index + 1; /*Index starts from 1 to be able to compute child idx*/     \
    while (1) {                                                                \
      size_t left = 2 * idx;                                                   \
      size_t right = 2 * idx + 1;                                              \
      size_t largest = idx;                                                    \
      if (left <= (a)->i &&                                                    \
          cmp(utarray_eltptr(a, left - 1), utarray_eltptr(a, largest - 1))) {  \
        largest = left;                                                        \
      }                                                                        \
      if (right <= (a)->i &&                                                   \
          cmp(utarray_eltptr(a, right - 1), utarray_eltptr(a, largest - 1))) { \
        largest = right;                                                       \
      }                                                                        \
      if (largest == idx) {                                                    \
        break;                                                                 \
      }                                                                        \
      ut_swap(_utarray_eltptr(a, idx - 1), _utarray_eltptr(a, largest - 1),    \
              (a)->icd.sz);                                                    \
      idx = largest;                                                           \
    }                                                                          \
  } while (0)

#define utheap_erase(a, idx, cmp)                                              \
  do {                                                                         \
    if ((a)->i) {                                                              \
      ut_swap(_utarray_eltptr(a, idx), _utarray_eltptr(a, (a)->i - 1),         \
              (a)->icd.sz);                                                    \
      utarray_resize(a, (a)->i - 1);                                           \
      if (utarray_len(a)) {                                                    \
        _utheap_heapify_down(a, idx, cmp);                                     \
      }                                                                        \
    }                                                                          \
  } while (0)

#define utheap_pop(a, cmp) utheap_erase(a, 0, cmp)
