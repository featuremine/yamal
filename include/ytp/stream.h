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
#define YTP_STREAM_LIST_ANNS 0
#define YTP_STREAM_LIST_SUBS 0
#define YTP_STREAM_LIST_INDX 0

/**
 * @brief Closes all stream level lists
 *
 * @param[in] yamal
 * @param[out] error
 */
FMMODFUNC void ytp_stream_close(ytp_yamal_t *yamal, fmc_error_t **error);

#ifdef __cplusplus
}
#endif
