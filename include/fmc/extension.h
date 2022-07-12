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
typedef void * fmc_ext_t;
#else
#error "Not defined for this operating system"
#endif

/**
 * @brief Returns a pointer of a symbol in a shared object or executable
 *
 * This functions loads the dynamic shared object (shared library) file named
 * and returns the address where the specified symbol is loaded into memory
 *
 * @param sym_name symbol name
 * @param path shared object or executable path
 * @param error out-parameter for error handling
 * @return a pointer associated with the symbol
 */
FMMODFUNC fmc_ext_t fmc_ext_load(const char *path, fmc_error_t **error);

#ifdef __cplusplus
}
#endif
