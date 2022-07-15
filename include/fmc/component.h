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

#pragma once

#include <fmc/config.h>
#include <fmc/time.h>
#include <fmc/extension.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(FMC_SYS_LINUX)
#define FMC_LIB_SUFFIX ".so"
#elif defined(FMC_SYS_MACH)
#define FMC_LIB_SUFFIX ".dylib"
#else
#error "Unsupported operating system"
#endif

#define fmc_comp_HEAD            \
   struct fmc_component_type *_vt;    \
   struct fmc_error _err;        \
   struct fmc_component_module *_mod;

struct fmc_component_module;
struct fmc_component {
   fmc_comp_HEAD;
};

typedef struct fmc_component *(*newfunc)(struct fmc_cfg_sect_item *, fmc_error_t **);
typedef void (*delfunc)(struct fmc_component *);
typedef struct fmc_time (*schedproc)(struct fmc_component *);
typedef bool (*processproc)(struct fmc_component *, struct fmc_time);

struct fmc_component_type {
   const char *name;
   const char *descr;
   size_t size; // size of the component struct
   struct fmc_cfg_node_spec *cfgspec; // configuration specifications
   newfunc new; // allocate and initialize the component
   delfunc del; // destroy the component
   schedproc sched; // returns the next schedule time. If NULL it allways process
   processproc process; // run the component once
};

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
   {"sessions", "Array of individual session configuration", FMCCFGARRAY, true, &sessions_cfg_spec},
   {NULL}
};

struct fmc_component_type components[] = {
   {
      .name = "live-gateway",
      .size = sizeof(gateway_comp),
      .cfgspec = gateway_cfg_spec;
      .new = (newfunc)gateway_comp_new,
      .del = (delfunc)gateway_comp_del,
      .sched = (schedproc)NULL,
      .process = (processproc)gateway_comp_process_one,
   },
   {
      .name = "sched-gateway",
      .size = sizeof(gateway_comp),
      .cfgspec = gateway_cfg_spec;
      .new = (newfunc)gateway_comp_new,
      .del = (delfunc)gateway_comp_del,
      .sched = (schedproc)gateway_comp_sched,
      .process = (processproc)gateway_comp_process_one,
   },
   {
      .name = "live-oms",
      .size = sizeof(manager_comp),
      .new = (newfunc)oms_comp_new,
      .del = (delfunc)oms_comp_del,
      .sched = (schedproc)NULL,
      .process = (processproc)oms_comp_process_one,
   },
   {
      .name = "sched-oms",
      .size = sizeof(manager_comp),
      .new = (newfunc)oms_comp_new,
      .del = (delfunc)oms_comp_del,
      .sched = (schedproc)oms_comp_sched,
      .process = (processproc)oms_comp_process_one,
   },
   {
      .name = NULL,
      .size = 0,
      .cfgspec = NULL;
      .sched = (schedproc)NULL,
      .process = (processproc)NULL,
   },
};

FMCOMPINITFUNC struct fmc_component_type *FMCompInit_oms() {
   return components;
}
*/

typedef struct fmc_component_list {
    struct fmc_component comp;
    struct fmc_component_list *next, *prev;
} fmc_component_list_t;

struct fmc_component_sys;
struct fmc_component_module {
   struct fmc_component_sys *sys; // the system that owns the module
   fmc_ext_t handle; // module handle. Return of dlopen()
   char *name; // module name (e.g. "oms")
   char *path; // file system path of the library
   struct fmc_component_type *components_type; // Null terminated array
   fmc_component_list_t *components;
};

typedef struct fmc_component_module_list {
    struct fmc_component_module mod;
    struct fmc_component_module_list *next, *prev;
} fmc_component_module_list_t;

struct fmc_component_sys {
   char **search_paths; // TODO: this should be a list of strings (utlist)
   fmc_component_module_list_t *modules;
};

typedef struct fmc_component_type * (*FMCOMPINITFUNC)(void);

void fmc_component_sys_init(struct fmc_component_sys *sys);
void fmc_component_sys_paths_set(struct fmc_component_sys *sys, const char **paths, fmc_error_t **error);
void fmc_component_sys_paths_add(struct fmc_component_sys *sys, const char *path, fmc_error_t **error);
const char **fmc_component_sys_paths_get(struct fmc_component_sys *sys); //  TODO: Return a list of paths
struct fmc_component_module *fmc_component_module_new(struct fmc_component_sys *sys, const char *mod, fmc_error_t **error);
void fmc_component_module_destroy(struct fmc_component_module *mod);
struct fmc_component *fmc_component_new(struct fmc_component_module *mod, const char *comp, struct fmc_cfg_sect_item *cfg, fmc_error_t **error);
void fmc_component_destroy(struct fmc_component *comp);
void fmc_component_sys_destroy(struct fmc_component_sys *sys);

#ifdef __cplusplus
}
#endif
