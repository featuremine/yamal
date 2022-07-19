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
   fmc_comp_HEAD;
};

struct manager_comp {
   fmc_comp_HEAD;
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

struct fmc_component_type components[] = {
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

FMCOMPMODINITFUNC struct fmc_component_type *FMCompInit_oms() {
   return components;
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

#define FMCOMPMODINITFUNC FMMODFUNC
#define fmc_comp_INIT_FUNCT_PREFIX "FMCompInit_"

#define fmc_comp_HEAD                                                          \
  struct fmc_component_type *_vt;                                              \
  struct fmc_error _err;                                                       \
  struct fmc_component_module *_mod;

struct fmc_component {
  fmc_comp_HEAD;
};

typedef struct fmc_component_type *(*fmc_comp_mod_init_func)(void);
typedef struct fmc_component *(*newfunc)(struct fmc_cfg_sect_item *,
                                         fmc_error_t **);
typedef void (*delfunc)(struct fmc_component *);
typedef fm_time64_t (*schedfunc)(struct fmc_component *);
typedef bool (*procfunc)(struct fmc_component *, fm_time64_t);

struct fmc_component_type {
  const char *tp_name;
  const char *tp_descr;
  size_t tp_size;                       // size of the component struct
  struct fmc_cfg_node_spec *tp_cfgspec; // configuration specifications
  newfunc tp_new;    // allocate and initialize the component
  delfunc tp_del;     // destroy the component
  schedfunc tp_sched; // returns the next schedule time. If NULL it allways process
  procfunc tp_proc; // run the component once
};

typedef struct fmc_component_list {
  struct fmc_component *comp;
  struct fmc_component_list *next, *prev;
} fmc_component_list_t;

struct fmc_component_module {
  struct fmc_component_sys *sys; // the system that owns the module
  fmc_ext_t handle;              // module handle. Return of dlopen()
  char *name;                    // module name (e.g. "oms")
  char *file;                    // file full path of the library
  struct fmc_component_type *components_type; // null terminated array
  fmc_component_list_t *components;           // allocated components
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
                                 const char **paths, fmc_error_t **error);
FMMODFUNC void fmc_component_sys_paths_add(struct fmc_component_sys *sys,
                                 const char *path, fmc_error_t **error);
FMMODFUNC fmc_component_path_list_t *
fmc_component_sys_paths_get(struct fmc_component_sys *sys);
FMMODFUNC void fmc_component_sys_destroy(struct fmc_component_sys *sys);
FMMODFUNC struct fmc_component_module *
fmc_component_module_new(struct fmc_component_sys *sys, const char *mod,
                         fmc_error_t **error);
FMMODFUNC void fmc_component_module_del(struct fmc_component_module *mod);
FMMODFUNC struct fmc_component *fmc_component_new(struct fmc_component_module *mod,
                                        const char *comp,
                                        struct fmc_cfg_sect_item *cfg,
                                        fmc_error_t **error);
FMMODFUNC void fmc_component_del(struct fmc_component *comp);

#ifdef __cplusplus
}
#endif
