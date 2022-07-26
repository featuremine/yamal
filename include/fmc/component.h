/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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

gateway_comp_process_one(gateway_comp *comp) {
   gateway_comp *c = (gateway_comp *)comp;
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
      .tp_size = sizeof(gateway_comp),
      .tp_cfgspec = gateway_cfg_spec;
      .tp_new = (newfunc)gateway_comp_new,
      .tp_del = (delfunc)gateway_comp_del,
      .tp_sched = (schedfunc)NULL,
      .tp_proc = (procfunc)gateway_comp_process_one,
   },
   {
      .tp_name = "sched-gateway",
      .tp_size = sizeof(gateway_comp),
      .tp_cfgspec = gateway_cfg_spec;
      .tp_new = (newfunc)gateway_comp_new,
      .tp_del = (delfunc)gateway_comp_del,
      .tp_sched = (schedfunc)gateway_comp_sched,
      .tp_proc = (procfunc)gateway_comp_process_one,
   },
   {
      .tp_name = "live-oms",
      .tp_size = sizeof(manager_comp),
      .tp_new = (newfunc)oms_comp_new,
      .tp_del = (delfunc)oms_comp_del,
      .tp_sched = (schedfunc)NULL,
      .tp_proc = (procfunc)oms_comp_process_one,
   },
   {
      .tp_name = "sched-oms",
      .tp_size = sizeof(manager_comp),
      .tp_new = (newfunc)oms_comp_new,
      .tp_del = (delfunc)oms_comp_del,
      .tp_sched = (schedfunc)oms_comp_sched,
      .tp_proc = (procfunc)oms_comp_process_one,
   },
   { NULL },
};

FMCOMPMODINITFUNC void FMCompInit_oms(struct fmc_component_api *api,
                                      struct fmc_component_module *mod) {
   // The number at the end of the functions signifies the API version.
   api->components_add_v1(mod, components);
}
*/

#pragma once

#include <fmc/config.h>
#include <fmc/extension.h>
#include <fmc/platform.h>
#include <fmc/time.h>

#ifdef __cplusplus
extern "C" {
#endif

// MODULES SEARCH PATHS
#define FMC_MOD_SEARCHPATH_CUR        ""
#define FMC_MOD_SEARCHPATH_SYSLOCAL   "/usr/local/lib/yamal/modules"
#define FMC_MOD_SEARCHPATH_USRLOCAL   ".local/lib/yamal/modules"
#define FMC_MOD_SEARCHPATH_SIZE       3
#define FMC_MOD_SEARCHPATH_ENV        "YAMALCOMPPATH"
#define FMC_MOD_SEARCHPATH_ENV_SEP    ":"

#define FMCOMPMODINITFUNC FMMODFUNC
#define FMC_COMPONENT_INIT_FUNC_PREFIX "FMCompInit_"

#define fmc_component_HEAD                                                     \
  struct fmc_component_type *_vt;                                              \
  struct fmc_error _err

struct fmc_component {
  fmc_component_HEAD;
};

/* NOTE: fmc_error_t, fmc_time64_t and fmc_cfg_sect_item cannot change.
         If changes to config or error object are required, must add
         new error or config structure and implement new API version */
typedef struct fmc_component *(*newfunc)(struct fmc_cfg_sect_item *,
                                         fmc_error_t **);
typedef void (*delfunc)(struct fmc_component *);
typedef fmc_time64_t (*schedfunc)(struct fmc_component *);
/*
procfunc must return false and report in the internal
error if an error occurred.
The stop flag argument must return true if the component is stopped.
*/
typedef bool (*procfunc)(struct fmc_component *, fmc_time64_t, bool *);

struct fmc_component_def_v1 {
  const char *tp_name;                  // prohibited characters: '-'
  const char *tp_descr;
  size_t tp_size;                       // size of the component struct
  struct fmc_cfg_node_spec *tp_cfgspec; // configuration specifications
  newfunc tp_new;                       // allocate and initialize the component
  delfunc tp_del;                       // destroy the component
  schedfunc tp_sched;                   // returns the next schedule time
  procfunc tp_proc;                     // run the component once
};

struct fmc_component_list {
  struct fmc_component *comp;
  struct fmc_component_list *next, *prev;
};

struct fmc_component_type {
  const char *tp_name;                  // prohibited characters: '-'
  const char *tp_descr;
  size_t tp_size;                       // size of the component struct
  struct fmc_cfg_node_spec *tp_cfgspec; // configuration specifications
  newfunc tp_new;                       // allocate and initialize the component
  delfunc tp_del;                       // destroy the component
  schedfunc tp_sched;                   // returns the next schedule time
  procfunc tp_proc;                     // run the component once
  struct fmc_component_list *comps;     // pointer to the containing component
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
FMMODFUNC void fmc_component_sys_paths_default_set(struct fmc_component_sys *sys,
                                                   fmc_error_t **error);
FMMODFUNC void fmc_component_sys_destroy(struct fmc_component_sys *sys);

FMMODFUNC struct fmc_component_module *
fmc_component_module_get(struct fmc_component_sys *sys, const char *mod,
                         fmc_error_t **error);
FMMODFUNC void fmc_component_module_del(struct fmc_component_module *mod);
FMMODFUNC struct fmc_component_type *
fmc_component_module_type_get(struct fmc_component_module *mod,
                              const char *comp, fmc_error_t **error);

FMMODFUNC struct fmc_component *fmc_component_new(struct fmc_component_type *tp,
                                                  struct fmc_cfg_sect_item *cfg,
                                                  fmc_error_t **error);
FMMODFUNC void fmc_component_del(struct fmc_component *comp);

/* Current API version: 1 (components_add_v1) */
struct fmc_component_api {
  void (*components_add_v1)(struct fmc_component_module *mod,
                            struct fmc_component_def_v1 *tps);
  void (*components_add_v2)(struct fmc_component_module *mod, void *);
  void (*components_add_v3)(struct fmc_component_module *mod, void *);
  void (*components_add_v4)(struct fmc_component_module *mod, void *);
  void (*components_add_v5)(struct fmc_component_module *mod, void *);
  void *_zeros[128];
};

typedef void (*fmc_component_module_init_func)(struct fmc_component_api *,
                                               struct fmc_component_module *);

#ifdef __cplusplus
}
#endif
