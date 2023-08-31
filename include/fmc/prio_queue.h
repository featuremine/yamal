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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <fmc/error.h>
#include <stdlib.h>

struct fmc_prio_queue_t {
  size_t size;
  int *buffer;
};

/**
 * @brief Initialize priority queue
 *
 * @param q pointer to priority queue to be initialized
 */
FMMODFUNC void fmc_prio_queue_init(struct fmc_prio_queue_t *q);

/**
 * @brief Destroy priority queue
 *
 * @param q pointer to priority queue to be destroyed
 */
FMMODFUNC void fmc_prio_queue_destroy(struct fmc_prio_queue_t *q);

/**
 * @brief Push element to priority queue
 *
 * @param q priority queue pointer
 * @param val value to be inserted
 * @param e out-parameter for error handling
 */
FMMODFUNC void fmc_prio_queue_push(struct fmc_prio_queue_t *q, int val,
                                   fmc_error_t **e);

/**
 * @brief Pop element from queue
 *
 * @param q priority queue pointer
 * @param out out-parameter for value to pop
 * @return true if a value was popped, false otherwise
 */
FMMODFUNC bool fmc_prio_queue_pop(struct fmc_prio_queue_t *q, int *out);

#ifdef __cplusplus
}
#endif
