/******************************************************************************

        COPYRIGHT (c) 2018 by Featuremine Corporation.
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
 * @file extension.cpp
 * @author Alejandro Farfan
 * @date 17 Jun 2021
 * @brief File contains C implementation of fmc ext loading
 *
 * This file contains C implementation of fmc ext loading.
 * @see http://www.featuremine.com
 */
#include <fmc/extension.h>

#if defined(FMC_SYS_WIN)
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

void *fmc_ext_load(const char *sym_name, const char *path,
                   fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_UNIX)
  auto *handle = dlopen(path, RTLD_NOW);
  if (!handle) {
    FMC_ERROR_REPORT(error, dlerror());
    return nullptr;
  }
  return dlsym(handle, sym_name);
#else
#error "Unsupported operating system"
#endif
}
