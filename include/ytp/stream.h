/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file stream.h
 * @date 6 Jun 2023
 * @brief File contains C declaration of stream layer of YTP
 *
 * This file contains C declaration of stream layer of YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <ytp/api.h>
#include <ytp/yamal.h>

#include <fmc/error.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YTP_STREAM_LIST_DATA 0
#define YTP_STREAM_LIST_ANNS 1
#define YTP_STREAM_LIST_SUBS 2
#define YTP_STREAM_LIST_INDX 3

#define YTP_STREAM_LIST_MIN 0
#define YTP_STREAM_LIST_MAX 3

/**
 * @brief Closes all stream level lists
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_stream_close(ytp_yamal_t *yamal, fmc_error_t **error);

#ifdef __cplusplus
}
#endif
