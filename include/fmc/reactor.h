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
struct fmc_component_sys sys;
struct fmc_reactor loop;
fmc_component_sys_init(&sys);
fmc_reactor_init(&loop);
struct fmc_component_module *mod =
      fmc_component_module_get(&sys, "my_components_module", &error);
if(error) { ...; }
struct fmc_component_type *tp =
    fmc_component_module_type_get(mod, "my_component", &error);
if(error) { ...; }
struct fmc_component *comp =
    fmc_component_new(&loop, tp, cfg, nullptr, &error);
if(error) { ...; }
fmc_reactor_run_live(&loop, &error);
if(error) { ...; }
fmc_reactor_destroy(&loop);
fmc_component_sys_destroy(&sys);
*/

#pragma once

#include <fmc/component.h>
#include <fmc/error.h>
#include <fmc/platform.h>
#include <fmc/time.h>
#include <uthash/utarray.h>
#include <fmc/math.h>
#include <fmc/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sched_item {
  fmc_time64_t t;
  size_t idx;
};

struct fmc_reactor_ctx;

typedef void (*fmc_reactor_dep_clbck)(struct fmc_component *self,
                                      struct fmc_reactor_ctx *ctx,
                                      fmc_time64_t now,
                                      int idx,
                                      struct fmc_shmem in);

typedef void (*fmc_reactor_exec_clbck)(struct fmc_component *self,
                                       struct fmc_reactor_ctx *ctx,
                                       fmc_time64_t now,
                                       int argc, struct fmc_shmem[]);

struct fmc_reactor_ctx {
  struct fmc_reactor *reactor;
  struct fmc_component *comp;
  fmc_reactor_exec_clbck exec;
  size_t idx;
  struct fmc_shmem *inp;
  size_t *deps[];
};

struct fmc_reactor {
  struct fmc_reactor_ctx **ctxs;
  size_t size;
  UT_array sched;
  UT_array queued;
  UT_array toqueue;
  volatile bool stop;
  bool done;
  struct fmc_pool pool;
};

struct fmc_component_input;

FMMODFUNC void fmc_reactor_init(struct fmc_reactor *reactor);
FMMODFUNC void fmc_reactor_destroy(struct fmc_reactor *reactor);
FMMODFUNC void fmc_reactor_ctx_init(struct fmc_reactor *reactor, 
                                    struct fmc_reactor_ctx *ctx);
FMMODFUNC void fmc_reactor_ctx_push(struct fmc_reactor_ctx *ctx,
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

#ifdef __cplusplus
}
#endif
