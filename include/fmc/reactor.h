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
 * @file reactor.h
 * @date 21 Jul 2022
 * @brief Reactor component API
 *
 * @see http://www.featuremine.com
 */

/* Usage example:
fmc_error_t *error;
fmc_reactor loop;
fmc_reactor_init(&loop);
fmc_reactor_component_add(&loop, gateway, 99, &error);
if(error) { fmc_reactor_destroy(&loop); return; }
fmc_reactor_component_add(&loop, manager, 98, &error);
if(error) { fmc_reactor_destroy(&loop); return; }
fmc_reactor_run(&loop, &error);
fmc_reactor_destroy(&loop);
*/

#pragma once

#include <fmc/component.h>
#include <fmc/error.h>
#include <fmc/platform.h>
#include <fmc/time.h>
#include <uthash/utarray.h>
#include <fmc/math.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sched_item {
  fmc_time64_t t;
  size_t idx;
};

struct fmc_reactor_component_list {
  struct fmc_component *comp;
  fmc_time64_t sched;
  struct fmc_reactor_component_list *next, *prev;
};

struct fmc_reactor_ctx;
typedef void (*fmc_reactor_exec_clbck)(struct fmc_component *self,
                                       fmc_time64_t now,
                                       struct fmc_reactor_ctx *ctx);

struct fmc_reactor_ctx {
  struct fmc_reactor *reactor;
  struct fmc_component *comp;
  fmc_reactor_exec_clbck exec;
  size_t idx;
  struct fmc_shmem *inp;
  size_t *deps[];
};

struct fmc_reactor {
  struct fmc_reactor_component_list *comps;
  size_t count;
  struct fmc_reactor_ctx **ctxs;
  UT_array sched;
  UT_array queued;
  UT_array toqueue;
  volatile bool stop;
  bool done;
};

struct fmc_component_input;

FMMODFUNC void fmc_reactor_init(struct fmc_reactor *reactor);
FMMODFUNC void fmc_reactor_destroy(struct fmc_reactor *reactor);
FMMODFUNC void fmc_reactor_component_add(struct fmc_reactor *reactor,
                                    struct fmc_component *comp,
                                    struct fmc_component_input *inps,
                                    fmc_error_t **error);
FMMODFUNC fmc_time64_t fmc_reactor_sched(struct fmc_reactor *reactor);
FMMODFUNC size_t fmc_reactor_run_once(struct fmc_reactor *reactor,
                                      fmc_time64_t now, fmc_error_t **error);
FMMODFUNC void fmc_reactor_run_sched(struct fmc_reactor *reactor,
                               fmc_error_t **error);
FMMODFUNC void fmc_reactor_run_live(struct fmc_reactor *reactor,
                               fmc_error_t **error);
FMMODFUNC void fmc_reactor_stop(struct fmc_reactor *reactor);
FMMODFUNC bool fmc_reactor_done(struct fmc_reactor *reactor);

FMMODFUNC bool sched_item_less(void*a, void*b);

FMMODFUNC bool size_t_less(void *a, void* b);

#ifdef __cplusplus
}
#endif
