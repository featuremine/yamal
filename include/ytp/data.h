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
 * @file data.h
 * @date 6 Jun 2023
 * @brief File contains C declaration of data API
 *
 * This file contains C declaration of data API of stream protocol level of YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <ytp/api.h>
#include <ytp/yamal.h>
#include <ytp/stream.h>

#include <fmc/error.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reserves memory for data in the memory mapped list
 *
 * @param[in] yamal
 * @param[in] sz the size of the data payload
 * @param[out] error
 * @return a writable pointer for data
 */
FMMODFUNC char *ytp_data_reserve(ytp_yamal_t *yamal, size_t sz,
                                  fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list
 *
 * @param[in] yamal
 * @param[in] data the value returned by ytp_data_reserve
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_data_commit(ytp_yamal_t *yamal, void *data,
                                         fmc_error_t **error);

/**
 * @brief Reads a message on data level
 *
 * @param[in] yamal
 * @param[in] iterator
 * @param[out] seqno
 * @param[out] ts
 * @param[out] stream
 * @param[out] sz
 * @param[out] data
 * @param[out] error
 */
FMMODFUNC void ytp_data_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                             uint64_t *seqno, int64_t *ts, ytp_mmnode_offs *stream,
                             size_t *sz, const char **data, fmc_error_t **error);

/**
 * @brief Returns an iterator to the beginning of the list
 *
 * @param[in] yamal
 * @param[out] error
 * @return ytp_iterator_t
 */
#define ytp_data_begin(yamal, error) ytp_yamal_begin(yamal, YTP_STREAM_LIST_DATA, error)

/**
 * @brief Returns an iterator to the end of the list
 *
 * @param[in] yamal
 * @param[out] error
 * @return ytp_iterator_t
 */
#define ytp_data_end(yamal, YTP_STREAM_LIST_DATA, error)

/**
 * @brief Returns the reserved size
 *
 * @param[in] yamal
 * @param[out] error
 * @return data size
 */
FMMODFUNC size_t ytp_data_reserved_size(ytp_yamal_t *yamal, fmc_error_t **error);

#ifdef __cplusplus
}
#endif
