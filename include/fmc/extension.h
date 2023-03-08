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
 * @file extension.h
 * @date 17 Jun 2021
 * @brief File contains C declaration of fmc ext loading
 *
 * This file contains C declaration of fmc ext loading.
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/error.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(FMC_SYS_UNIX)
typedef void *fmc_ext_t;
#else
#error "Not defined for this operating system"
#endif

struct fmc_ext_searchpath_t {
  struct fmc_ext_searchpath_t *next, *prev;
  char path[]; // FAM
};

struct fmc_ext_mod_t {
  fmc_ext_t handle;
  void *func;
  const char *path;
};

/**
 * @brief opens an extension
 *
 * This functions loads the dynamic shared object (shared library)
 * and returns the handle
 *
 * @param path shared object or executable path
 * @param error out-parameter for error handling
 * @return the extension handle
 */
FMMODFUNC fmc_ext_t fmc_ext_open(const char *path, fmc_error_t **error);

/**
 * @brief Returns a pointer of a symbol of a shared object or executable
 *
 * @param handle Extension handle
 * @param sym symbol name
 * @param error out-parameter for error handling
 * @return the address where the specified symbol is loaded into memory
 */
FMMODFUNC void *fmc_ext_sym(fmc_ext_t handle, const char *sym,
                            fmc_error_t **error);

/**
 * @brief Closes an extension
 *
 * @param handle Extension handle
 */
FMMODFUNC void fmc_ext_close(fmc_ext_t handle);

/**
 * @brief Searches and loads a module
 *
 * @param modname Module name
 * @param initfunc_prefix
 * @param search_paths a list of search paths
 * @param error
 * @return the extension handle and the function pointer
 */
FMMODFUNC struct fmc_ext_mod_t
fmc_ext_mod_load(const char *modname, const char *initfunc_prefix,
                 struct fmc_ext_searchpath_t *search_paths,
                 fmc_error_t **error);

/**
 * @brief Destroy a module
 *
 * @param mod Module
 */
FMMODFUNC void fmc_ext_mod_destroy(struct fmc_ext_mod_t *mod);

/**
 * @brief Destroy a search path list
 *
 * @param phead Search path list
 */
FMMODFUNC void fmc_ext_searchpath_del(struct fmc_ext_searchpath_t **phead);

/**
 * @brief Add a path to the search path list
 *
 * @param phead Search path list
 * @param path
 * @param error
 */
FMMODFUNC void fmc_ext_searchpath_add(struct fmc_ext_searchpath_t **phead,
                                      const char *path, fmc_error_t **error);

/**
 * @brief Set the search path list given an null-terminated array of paths
 *
 * @param phead Search path list
 * @param paths A null-terminated array of paths
 * @param error
 */
FMMODFUNC void fmc_ext_searchpath_set(struct fmc_ext_searchpath_t **head,
                                      const char **paths, fmc_error_t **error);

/**
 * @brief Set the search path list as default:
 * 1. Current working directory
 * 2. $HOME/.local/{defaultpath}
 * 3. /usr/local/{defaultpath}
 * 4. If available: A list of paths from ${pathenvname}
 *
 * @param phead Search path list
 * @param defaultpath
 * @param pathenvname
 * @param error
 */
FMMODFUNC void
fmc_ext_searchpath_set_default(struct fmc_ext_searchpath_t **head,
                               const char *defaultpath, const char *pathenvname,
                               fmc_error_t **error);

#ifdef __cplusplus
}
#endif
