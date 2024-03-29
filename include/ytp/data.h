/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
#include <ytp/stream.h>
#include <ytp/yamal.h>

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
 * @param[out] error out-parameter for error handling
 * @return a writable pointer for data
 */
FMMODFUNC char *ytp_data_reserve(ytp_yamal_t *yamal, size_t sz,
                                 fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list
 *
 * @param[in] yamal
 * @param[in] ts
 * @param[in] stream
 * @param[in] data the value returned by ytp_data_reserve
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_data_commit(ytp_yamal_t *yamal, int64_t ts,
                                         ytp_mmnode_offs stream, void *data,
                                         fmc_error_t **error);

/**
 * @brief Commits a new data node to an existing sublist (first_ptr, last_ptr)
 * that is not in the main memory mapped list
 *
 * @param[in] yamal
 * @param[in] ts
 * @param[in] stream
 * @param[in, out] first_ptr an zero initialized atomic pointer for the first
 * node of the sublist
 * @param[in, out] last_ptr an zero initialized atomic pointer for the last node
 * of the sublist
 * @param[in] data the value returned by ytp_data_reserve for the node that
 * is intended to insert
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_data_sublist_commit(ytp_yamal_t *yamal, int64_t ts,
                                       ytp_mmnode_offs stream, void **first_ptr,
                                       void **last_ptr, void *data,
                                       fmc_error_t **error);

/**
 * @brief Commits a data sublist to the memory mapped list
 *
 * @param[in] yamal
 * @param[in] first_ptr the first node of the sublist
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the first message
 */
FMMODFUNC ytp_iterator_t ytp_data_sublist_finalize(ytp_yamal_t *yamal,
                                                   void *first_ptr,
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
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_data_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                             uint64_t *seqno, int64_t *ts,
                             ytp_mmnode_offs *stream, size_t *sz,
                             const char **data, fmc_error_t **error);

/**
 * @brief Returns an iterator to the beginning of the list
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
static inline ytp_iterator_t ytp_data_begin(ytp_yamal_t *yamal,
                                            fmc_error_t **error) {
  return ytp_yamal_begin(yamal, YTP_STREAM_LIST_DATA, error);
}

/**
 * @brief Returns an iterator to the end of the list
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
static inline ytp_iterator_t ytp_data_end(ytp_yamal_t *yamal,
                                          fmc_error_t **error) {
  return ytp_yamal_end(yamal, YTP_STREAM_LIST_DATA, error);
}

#ifdef __cplusplus
}
#endif
