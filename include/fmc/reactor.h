/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
#include <fmc/math.h>
#include <fmc/memory.h>
#include <fmc/platform.h>
#include <fmc/time.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(fmc_ctor_f)(void *dst, const void *src);
typedef void(fmc_dtor_f)(void *elt);
typedef void(fmc_init_f)(void *elt);
typedef struct {
  size_t sz;
  fmc_init_f *init;
  fmc_ctor_f *copy;
  fmc_dtor_f *dtor;
} fmc_icd;

typedef struct {
  unsigned i, n; /* i: index of next available slot, n: num slots */
  fmc_icd icd;   /* initializer, copy and destructor functions */
  char *d;       /* n slots of size icd->sz*/
} fmc_array;

#define fmc_array_init(a, _icd)                                                \
  do {                                                                         \
    memset(a, 0, sizeof(fmc_array));                                           \
    (a)->icd = *(_icd);                                                        \
  } while (0)

struct sched_item {
  fmc_time64_t t;
  size_t idx;
};

struct fmc_reactor_ctx;
struct fmc_component;

/*
The reactor should not be used directly in fmc_reactor_dep_clbck that have been
registered
*/
typedef void (*fmc_reactor_dep_clbck)(struct fmc_component *self, int idx,
                                      struct fmc_shmem in);

typedef void (*fmc_reactor_exec_clbck)(struct fmc_component *self,
                                       struct fmc_reactor_ctx *ctx,
                                       fmc_time64_t now);

typedef void (*fmc_reactor_shutdown_clbck)(struct fmc_component *self,
                                           struct fmc_reactor_ctx *ctx);

struct fmc_reactor_ctx_out {
  struct fmc_reactor_ctx_out *next;
  struct fmc_reactor_ctx_out *prev;
  struct fmc_reactor_ctx *ctx;
  char *name;
  char *type;
};

struct fmc_reactor_ctx_dep {
  size_t idx;
  size_t inp_idx;
};

struct fmc_reactor_ctx {
  struct fmc_reactor *reactor;
  struct fmc_component *comp;
  fmc_error_t err;
  fmc_reactor_exec_clbck exec;
  fmc_reactor_shutdown_clbck shutdown;
  fmc_reactor_dep_clbck dep_upd;
  size_t idx;
  bool finishing;
  struct fmc_reactor_ctx_out
      *out_tps;   // list of fmc_reactor_component_output {name, type}
                  // use double linked list, add them with append at the end
  fmc_array deps; // change to use a structure that holds both dep idx and input
                  // idx array of array of structures - no lists.
                  // fmc_reactor_ctx_dep
};

struct fmc_reactor_stop_item {
  struct fmc_reactor_stop_item *next;
  struct fmc_reactor_stop_item *prev;
  size_t idx;
};

struct fmc_reactor {
  struct fmc_reactor_ctx **ctxs;
  size_t size;
  fmc_array sched;
  fmc_array queued;
  fmc_array toqueue;
  size_t finishing;
  int stop;
  volatile int stop_signal;
  struct fmc_reactor_stop_item *stop_list;
  struct fmc_pool pool;
  fmc_error_t err;
};

struct fmc_component_input;

FMMODFUNC void fmc_reactor_init(struct fmc_reactor *reactor);
FMMODFUNC void fmc_reactor_destroy(struct fmc_reactor *reactor);
FMMODFUNC struct fmc_reactor_ctx *
fmc_reactor_ctx_new(struct fmc_reactor *reactor, fmc_error_t **error);
FMMODFUNC void fmc_reactor_ctx_take(struct fmc_reactor_ctx *ctx,
                                    struct fmc_component_input *inps,
                                    fmc_error_t **error);
FMMODFUNC void fmc_reactor_ctx_del(struct fmc_reactor_ctx *ctx);
FMMODFUNC fmc_time64_t fmc_reactor_sched(struct fmc_reactor *reactor);
FMMODFUNC bool fmc_reactor_run_once(struct fmc_reactor *reactor,
                                    fmc_time64_t now, fmc_error_t **error);
FMMODFUNC void fmc_reactor_run(struct fmc_reactor *reactor, bool live,
                               fmc_error_t **error);
FMMODFUNC void fmc_reactor_stop(struct fmc_reactor *reactor);

#ifdef __cplusplus
}
#endif
