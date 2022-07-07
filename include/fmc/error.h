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
 * @file error.h
 * @author Federico Ravchina
 * @date 10 May 2021
 * @brief File contains C declaration of fmc error handling
 *
 * This file contains C declaration of fmc error handling.
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

struct fmc_error;
typedef struct fmc_error fmc_error_t;

#define FMC_ERROR_REPORT(err, msg)                                             \
  fmc_error_set(err, "%s (%s:%d)", msg, __FILE__, __LINE__)

/**
 * @brief Clears the error pointer
 * @param err
 */
FMMODFUNC void fmc_error_clear(fmc_error_t **err);

/**
 * @brief Set an error message and assigns a pointer to the error message
 *
 * @param err
 * @param fmt error format string
 * @param ... depending on the format string, a sequence of additional arguments
 * is expected
 */
FMMODFUNC void fmc_error_set(fmc_error_t **err, const char *fmt, ...);

/**
 * @brief Returns the message associated with the error object
 *
 * @param err
 * @return a C-string error message
 */
FMMODFUNC const char *fmc_error_msg(fmc_error_t *err);

/**
 * @brief Returns the current thread error object
 * @return the current thread error object
 */
FMMODFUNC fmc_error_t *fmc_error_inst();

/**
 * @brief Returns the last system error message
 *
 * @return a C-string error message
 */
FMMODFUNC const char *fmc_syserror_msg();

#ifdef __cplusplus
}
#endif
