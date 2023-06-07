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
 * @file time.h
 * @date 23 Apr 2021
 * @brief File contains C declaration of time layer of YTP
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reserves memory for data in the memory mapped list
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] sz the size of the data payload
 * @param[out] error out-parameter for error handling
 * @return a writable pointer for data
 */
FMMODFUNC char *ytp_time_reserve(ytp_yamal_t *yamal, size_t sz,
                                 fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] ts the time to publish the message
 * @param[in] data the value returned by ytp_time_reserve
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, int64_t ts,
                                         void *data, size_t listidx,
                                         fmc_error_t **error);

/**
 * @brief Reads a message on channel level
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] iterator
 * @param[out] seqno
 * @param[out] ts
 * @param[out] sz
 * @param[out] data
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                             uint64_t *seqno, int64_t *ts, size_t *sz,
                             const char **data, fmc_error_t **error);

#ifdef __cplusplus
}
#endif
