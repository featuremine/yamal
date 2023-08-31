/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
 * @brief Commits the data to the memory mapped list on the time level.
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] ts the time to publish the message
 * @param[in] data the value returned by ytp_peer_reserve if the node is not a
 * sublist. Otherwise the first_ptr returned by ytp_peer_sublist_commit
 * @param[in] lstidx the list index to commit to
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, int64_t ts,
                                         void *data, size_t listidx,
                                         fmc_error_t **error);

/**
 * @brief Commits a new data node to an existing sublist (first_ptr, last_ptr)
 * that is not in the main memory mapped list
 *
 * @param[in] yamal
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] ts the time to publish the message
 * @param[in, out] first_ptr an zero initialized atomic pointer for the first
 * node of the sublist
 * @param[in, out] last_ptr an zero initialized atomic pointer for the last node
 * of the sublist
 * @param[in] new_ptr the value returned by ytp_peer_reserve for the node that
 * is intended to insert
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_time_sublist_commit(ytp_yamal_t *yamal, int64_t ts,
                                       void **first_ptr, void **last_ptr,
                                       void *new_ptr, fmc_error_t **error);

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
