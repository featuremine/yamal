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

typedef struct fmc_comp_type {
   const char *name;
   const char *descr;
   size_t size;
   fmc_cfg_node_spec cfgspec;
   schedproc sched;
   processproc process;
} fmc_comp_type;

typedef struct fmc_comp_sys {
   fmc_comp_type *components;
} fmc_comp_sys;


void fmc_component_sys_init(fmc_comp_sys *sys);
void fmc_component_sys_paths_set(fmc_comp_sys *sys, const char **paths);
const char **fmc_component_sys_path_get(fmc_comp_sys *sys);
void fmc_component_sys_mod_load(fmc_comp_sys *sys, const char *mod);
// fmc_component *fmc_component_sys_comp_init(fmc_comp_sys *sys, const char *comp, fmc_cfg_node_spec *cfg);
// void fmc_comp_sys_comp_destroy(fmc_comp_sys *sys, fmc_component *comp);
void fmc_comp_sys_destroy(fmc_comp_sys *sys);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENT_H__ */