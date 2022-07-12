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
 * @file config.h
 * @author Ivan Gonzalez
 * @date 11 Jul 2022
 * @brief Components
 *
 * @see http://www.featuremine.com
 */

#ifndef COMPONENT_H__
#define COMPONENT_H__

#include <fmc/config.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO implement functions definition
typedef void (*schedproc)(void *comp);
typedef void (*processproc)(void *comp);

typedef struct fmc_component_type {
   const char *name;
   const char *descr;
   size_t size; // size of the component struct
   fmc_cfg_node_spec *cfgspec; // configuration specifications
   schedproc sched; // returns the next schedule time. If NULL it allways process
   processproc process; // run the component once
} fmc_component_type;

typedef struct fmc_component {
   bool alive; // TODO: check
} fmc_component;

typedef struct fmc_component_module {
   void *handle; // module handle. Return of dlopen()
   char *module_name; // module name (e.g. "oms")
   char *module_path; // file system path of the library
   fmc_component_type *components_type; // TODO: List?
} fmc_component_module;

typedef struct fmc_component_sys {
   const char **search_paths;
   fmc_component_module *modules; // only != NULL when library succ. loaded // TODO: List?
   fmc_component *components;
} fmc_component_sys;


void fmc_component_sys_init(fmc_component_sys *sys);
void fmc_component_sys_paths_set(fmc_component_sys *sys, const char **paths);
const char **fmc_component_sys_paths_get(fmc_component_sys *sys);
void fmc_component_sys_mod_load(fmc_component_sys *sys, const char *mod); // does not load components
fmc_component *fmc_component_sys_comp_init(fmc_component_sys *sys, const char *comp, fmc_cfg_sect *cfg);
void fmc_component_sys_comp_destroy(fmc_component_sys *sys, fmc_component *comp);
void fmc_component_sys_destroy(fmc_component_sys *sys);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENT_H__ */