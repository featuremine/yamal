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
 * @file files.h
 * @date 19 Jul 2022
 * @brief File contains C declaration of fmc string API
 *
 * This file contains C declaration of fmc string API.
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/error.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a new C string from another C string
 *
 * @param str C string to copy from
 * @param error out-parameter for error handling
 * @return The new C string
 */
FMMODFUNC char *fmc_cstr_new(const char *str, fmc_error_t **error);

/**
 * @brief Creates a new C string from a character buffer
 *
 * @param str string to copy from
 * @param sz length of the string
 * @param error out-parameter for error handling
 * @return The new C string
 */
FMMODFUNC char *fmc_cstr_new2(const char *str, size_t sz, fmc_error_t **error);

/**
 * @brief Check string starts with pairwise options
 *
 * @param targ is the string to compare
 * @param str1 is one of the strings to compare against (length may be 0)
 * @param str2 str2 is the other; it must be the same length as str1
 * @return length of str1 if has correct prefix, (that is, targ is the same
 * length as str1 and str2, and each character of targ is in one of str1 or str2
 * in the corresponding position), or 0 otherwise
 */
FMMODFUNC size_t fmc_cstr_biparse(const char *targ, const char *str1,
                                  const char *str2);

#ifdef __cplusplus
}
#endif
