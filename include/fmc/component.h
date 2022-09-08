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
 * @file component.h
 * @date 11 Jul 2022
 * @brief Reactor component API
 *
 * @see http://www.featuremine.com
 */

/* Example of module main file
struct gateway_comp {
   fmc_component_HEAD;
};

struct manager_comp {
   fmc_component_HEAD;
};

struct fmc_reactor_api_v1 *_reactor;

static void gateway_comp_del(struct gateway_comp *comp) {
  free(comp);
};

static struct gateway_comp *
gateway_comp_new_live(struct fmc_cfg_sect_item *cfg,
                       struct fmc_reactor_ctx *ctx,
                       char **inp_tps,
                       fmc_error_t **err) {
  struct test_component *c = (struct test_component *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  _reactor->on_exec(ctx, &gateway_comp_process_one_live);
  _reactor->queue(ctx);
  return c;
cleanup:
  fmc_error_set2(err, FMC_ERROR_MEMORY);
  return NULL;
};

static struct gateway_comp *
gateway_comp_new_sched(struct fmc_cfg_sect_item *cfg,
                       struct fmc_reactor_ctx *ctx,
                       char **inp_tps,
                       fmc_error_t **err) {
  struct test_component *c = (struct test_component *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  _reactor->on_exec(ctx, &gateway_comp_process_one_sched);
  _reactor->schedule(ctx, start_time);
  return c;
cleanup:
  fmc_error_set2(err, FMC_ERROR_MEMORY);
  return NULL;
};

fmc_cfg_node_spec session_cfg_spec[] = {
   {"channel", "YTP channel of the session", FMCCFGSTRING, true, NULL},
   {NULL}
}

fmc_cfg_arr_spec sessions_cfg_spec = {FMCCFGSECT, &session_cfg_spec};

fmc_cfg_node_spec gateway_cfg_spec[] = {
   {"sessions", "Array of individual session configuration", FMCCFGARRAY, true,
&sessions_cfg_spec}, {NULL}
};

struct fmc_component_def_v1 components[] = {
   {
      .tp_name = "live-gateway",
      .tp_descr = "Gateway live component",
      .tp_size = sizeof(gateway_comp),
      .tp_cfgspec = gateway_cfg_spec;
      .tp_new = (fmc_newfunc)gateway_comp_new_live,
      .tp_del = (fmc_delfunc)gateway_comp_del,
   },
   {
      .tp_name = "sched-gateway",
      .tp_descr = "Gateway scheduled component",
      .tp_size = sizeof(gateway_comp),
      .tp_cfgspec = gateway_cfg_spec;
      .tp_new = (fmc_newfunc)gateway_comp_new_sched,
      .tp_del = (fmc_delfunc)gateway_comp_del,
   },
   {
      .tp_name = "live-oms",
      .tp_descr = "OMS live component",
      .tp_size = sizeof(manager_comp),
      .tp_new = (fmc_newfunc)oms_comp_new,
      .tp_del = (fmc_delfunc)oms_comp_del,
   },
   {
      .tp_name = "sched-oms",
      .tp_descr = "OMS scheduled component",
      .tp_size = sizeof(manager_comp),
      .tp_new = (fmc_newfunc)oms_comp_new,
      .tp_del = (fmc_delfunc)oms_comp_del,
   },
   { NULL },
};

FMCOMPMODINITFUNC void FMCompInit_oms(struct fmc_component_api *api,
                                      struct fmc_component_module *mod) {
   // The number at the end of the functions signifies the API version.
   api->components_add_v1(mod, components);
   _reactor = api->reactor_v1;
}
*/

#pragma once

#include <fmc/config.h>
#include <fmc/extension.h>
#include <fmc/memory.h>
#include <fmc/platform.h>
#include <fmc/reactor.h>
#include <fmc/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FMCOMPMODINITFUNC FMMODFUNC
#define FMC_COMPONENT_INIT_FUNC_PREFIX "FMCompInit_"

#define fmc_component_HEAD                                                     \
  struct fmc_component_type *_vt;                                              \
  struct fmc_reactor_ctx *_ctx;

struct fmc_component {
  fmc_component_HEAD;
};

// TODO: Complete
struct fmc_reactor_api_v1 {
  void (*queue)(struct fmc_reactor_ctx *);
  void (*schedule)(struct fmc_reactor_ctx *, fmc_time64_t);
  void (*set_error)(struct fmc_reactor_ctx *, const char *fmt,
                    ...); // set the component error
  void (*on_shutdown)(struct fmc_reactor_ctx *,
                      fmc_reactor_shutdown_clbck); // system shutting down
  void (*finished)(
      struct fmc_reactor_ctx *); // notify the system that component is finished
  void (*on_exec)(
      struct fmc_reactor_ctx *,
      fmc_reactor_exec_clbck); // all input components have been updated
  void (*on_dep)(struct fmc_reactor_ctx *,
                 fmc_reactor_dep_clbck); // a dependency has been updated
  void (*add_output)(struct fmc_reactor_ctx *, const char *type,
                     const char *name);
  void (*notify)(
      struct fmc_reactor_ctx *, size_t,
      struct fmc_shmem); // notify the system that output have been updated
  struct fmc_pool *(*get_pool)(struct fmc_reactor_ctx *);
};

/* NOTE: fmc_error_t, fmc_time64_t and fmc_cfg_sect_item cannot change.
         If changes to config or error object are required, must add
         new error or config structure and implement new API version */
typedef struct fmc_component *(*fmc_newfunc)(struct fmc_cfg_sect_item *,
                                             struct fmc_reactor_ctx *ctx,
                                             char **inp_tps);
typedef void (*fmc_delfunc)(struct fmc_component *);

struct fmc_component_def_v1 {
  const char *tp_name; // prohibited characters: '-'
  const char *tp_descr;
  size_t tp_size;                       // size of the component struct
  struct fmc_cfg_node_spec *tp_cfgspec; // configuration specifications
  fmc_newfunc tp_new;                   // alloc and initialize the component
  fmc_delfunc tp_del;                   // destroy the component
};

struct fmc_component_list {
  struct fmc_component *comp;
  struct fmc_component_list *next, *prev;
};

struct fmc_component_type {
  const char *tp_name; // prohibited characters: '-'
  const char *tp_descr;
  size_t tp_size;                       // size of the component struct
  struct fmc_cfg_node_spec *tp_cfgspec; // configuration specifications
  fmc_newfunc tp_new;                   // alloc and initialize the component
  fmc_delfunc tp_del;                   // destroy the component
  struct fmc_component_type *next, *prev;
};

struct fmc_component_module {
  struct fmc_component_sys *sys;    // the system that owns the module
  fmc_error_t error;                // reports errors for this module
  fmc_ext_t handle;                 // module handle. Return of dlopen()
  char *name;                       // module name (e.g. "oms")
  char *file;                       // file full path of the library
  struct fmc_component_type *types; // list of component types
  struct fmc_component_module *next, *prev;
};

typedef struct fmc_component_path_list {
  struct fmc_component_path_list *next, *prev;
  char path[]; // FAM
} fmc_component_path_list_t;

struct fmc_component_sys {
  fmc_component_path_list_t *search_paths;
  struct fmc_component_module *modules;
};

FMMODFUNC void fmc_component_sys_init(struct fmc_component_sys *sys);
FMMODFUNC void fmc_component_sys_paths_set(struct fmc_component_sys *sys,
                                           const char **paths,
                                           fmc_error_t **error);
FMMODFUNC void fmc_component_sys_paths_add(struct fmc_component_sys *sys,
                                           const char *path,
                                           fmc_error_t **error);
FMMODFUNC fmc_component_path_list_t *
fmc_component_sys_paths_get(struct fmc_component_sys *sys);
FMMODFUNC void
fmc_component_sys_paths_set_default(struct fmc_component_sys *sys,
                                    fmc_error_t **error);
FMMODFUNC void fmc_component_sys_destroy(struct fmc_component_sys *sys);

FMMODFUNC struct fmc_component_module *
fmc_component_module_get(struct fmc_component_sys *sys, const char *mod,
                         fmc_error_t **error);
FMMODFUNC void fmc_component_module_del(struct fmc_component_module *mod);
FMMODFUNC struct fmc_component_type *
fmc_component_module_type_get(struct fmc_component_module *mod,
                              const char *comp, fmc_error_t **error);

struct fmc_component_input {
  struct fmc_component *comp;
  size_t idx;
};

FMMODFUNC struct fmc_component *
fmc_component_new(struct fmc_reactor *reactor, struct fmc_component_type *tp,
                  struct fmc_cfg_sect_item *cfg,
                  struct fmc_component_input *inps, fmc_error_t **error);
FMMODFUNC size_t fmc_component_out_idx(struct fmc_component *, const char *name,
                                       fmc_error_t **error);
FMMODFUNC size_t fmc_component_out_sz(struct fmc_component *);

/* Current API version: 1 (components_add_v1) */
struct fmc_component_api {
  struct fmc_reactor_api_v1 *reactor_v1;
  void (*components_add_v1)(struct fmc_component_module *mod,
                            struct fmc_component_def_v1 *tps);
  void *reactor_v2;
  void (*components_add_v2)(struct fmc_component_module *mod, void *);
  void *reactor_v3;
  void (*components_add_v3)(struct fmc_component_module *mod, void *);
  void *reactor_v4;
  void (*components_add_v4)(struct fmc_component_module *mod, void *);
  void *reactor_v5;
  void (*components_add_v5)(struct fmc_component_module *mod, void *);
  void *_zeros[128];
};

typedef void (*fmc_component_module_init_func)(struct fmc_component_api *,
                                               struct fmc_component_module *);

#ifdef __cplusplus
}
#endif
