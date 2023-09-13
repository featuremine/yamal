/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file cmdline.h
 * @date 7 Aug 2023
 * @brief Yet another command line arguments parser API
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fmc_cmdline_opt {
  const char *str;    /* option key string */
  bool required;      /* specifies whether option is required */
  const char **value; /* if not NULL, the address of the variable to store the value */
  bool set;           /* will be set by the parser if the option was set */
} fmc_cmdline_opt_t;

/**
 * @brief Parses a single command line option
 *
 * @param argc count of command line arguments
 * @param argv command line arguments
 * @param opts command line option descriptions
 * @param err out-parameter for error handling
 */
FMMODFUNC void fmc_cmdline_opt_proc(int argc, const char **argv,
                                    fmc_cmdline_opt_t *opts, fmc_error_t **err);

#ifdef __cplusplus
}
#endif
