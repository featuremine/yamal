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

FMMODFUNC void fmc_prio_queue_init(struct fmc_prio_queue_t *q);

FMMODFUNC void fmc_prio_queue_destroy(struct fmc_prio_queue_t *q);

FMMODFUNC void fmc_prio_queue_push(struct fmc_prio_queue_t *q, int val, fmc_error_t **e);

FMMODFUNC bool fmc_prio_queue_pop(struct fmc_prio_queue_t *q, int *out);

#ifdef __cplusplus
}
#endif
