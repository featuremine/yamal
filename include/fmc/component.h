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

#ifdef __cplusplus
extern "C" {
#endif

#define FMC_ERROR_MAX_SIZE 256

#define fmc_comp_HEAD  \
   struct fmc_comp_type *_vt; \
   struct fmc_error _err

struct fmc_component {
   fmc_comp_HEAD;
};

typedef struct fmc_time (*schedproc)(struct fmc_component *);
typedef bool (*processproc)(struct fmc_component *, struct fmc_time);

struct fmc_component_type {
   const char *name;
   const char *descr;
   size_t size; // size of the component struct
   struct fmc_cfg_node_spec *cfgspec; // configuration specifications
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
      .sched = (schedproc)NULL,
      .process = (processproc)gateway_comp_process_one,
   },
   {
      .name = "sched-gateway",
      .size = sizeof(gateway_comp),
      .cfgspec = gateway_cfg_spec;
      .sched = (schedproc)gateway_comp_sched,
      .process = (processproc)gateway_comp_process_one,
   },
   {
      .name = "live-oms",
      .size = sizeof(manager_comp),
      .sched = (schedproc)NULL,
      .process = (processproc)oms_comp_process_one,
   },
   {
      .name = "sched-oms",
      .size = sizeof(manager_comp),
      .sched = (schedproc)gateway_comp_sched,
      .process = (processproc)oms_comp_process_one,
   }
   { NULL }
};

FMCOMPINITFUNC struct fmc_component_type *FMCompInit_oms() {
   return components;
}
*/

struct fmc_component_module {
   fmc_ext_t handle; // module handle. Return of dlopen()
   const char *name; // module name (e.g. "oms")
   const char *path; // file system path of the library
   struct fmc_component_type *components; // Null terminated array
};

typedef struct fmc_component_module fmc_component_module_t;
typedef struct fmc_component fmc_component_t;

#define FMC_PTR_LIST_DEF(type)          \
   struct __fmc_list_##type {           \
      type *_val;                       \
      struct __fmc_list_##type *_next;  \
   }

#define FMC_PTR_LIST_TYPE(type) struct __fmc_list_##type

#define FMC_PTR_LIST_FOREACH(type, list, var, proc) \
   for (FMC_PTR_LIST_TYPE(type) **_iter = &list; *_iter; _iter = &_iter->_next) { \
         type *var = *_iter->_val; {proc}; } \

/*
fmc_component_t cp = NULL;
FMC_PTR_LIST_FOREACH(fmc_component_t, sys->components, x,
   if (strcmp(x->name, key) == 0) {
      cp = x;
      break;
   }
)
if (cp == NULL) {
   // could not find it
}
*/

FMC_PTR_LIST_DEF(fmc_component_module_t);
FMC_PTR_LIST_DEF(fmc_component_t);

struct fmc_component_sys {
   const char **search_paths;
   FMC_PTR_LIST_TYPE(fmc_component_module_t) *modules;
   FMC_PTR_LIST_TYPE(fmc_component_t) *components;
};

void fmc_component_sys_init(struct fmc_component_sys *sys);
void fmc_component_sys_paths_set(struct fmc_component_sys *sys, const char **paths);
const char **fmc_component_sys_paths_get(struct fmc_component_sys *sys);
void fmc_component_sys_mod_load(struct fmc_component_sys *sys, const char *mod);
struct fmc_component *fmc_component_sys_comp_new(struct fmc_component_sys *sys, const char *mod, const char *comp, struct fmc_cfg_sect_item *cfg);
void fmc_component_sys_comp_destroy(struct fmc_component_sys *sys, struct fmc_component *comp);
void fmc_component_sys_destroy(struct fmc_component_sys *sys);

#ifdef __cplusplus
}
#endif
