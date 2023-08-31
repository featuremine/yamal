/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file subscription.h
 * @date 6 Jun 2023
 * @brief File contains C declaration of subscription API
 *
 * This file contains C declaration of subscription API
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
 * @brief Writes a raw stream subscription message
 *
 * @param[in] yamal
 * @param[in] stream
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_subscription_write(ytp_yamal_t *yamal,
                                                ytp_mmnode_offs stream,
                                                fmc_error_t **error);

/**
 * @brief Writes and commits a stream subscription message
 *
 * @param[in] yamal
 * @param[in] stream
 * @param[out] error out-parameter for error handling
 * @return true if the message subscription was successfully committed, false
 * otherwise
 */
FMMODFUNC bool ytp_subscription_commit(ytp_yamal_t *yamal,
                                       ytp_mmnode_offs stream,
                                       fmc_error_t **error);

/**
 * @brief Reads a raw stream subscription message
 *
 * @param[in] yamal
 * @param[in] iterator
 * @param[out] seqno
 * @param[out] stream
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC void ytp_subscription_read(ytp_yamal_t *yamal,
                                     ytp_iterator_t iterator, uint64_t *seqno,
                                     ytp_mmnode_offs *stream,
                                     fmc_error_t **error);

/**
 * @brief Lookups a raw stream subscription message
 *
 * @param[in] yamal
 * @param[in] offset
 * @param[out] seqno
 * @param[out] stream
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC void ytp_subscription_lookup(ytp_yamal_t *yamal,
                                       ytp_mmnode_offs offset, uint64_t *seqno,
                                       ytp_mmnode_offs *stream,
                                       fmc_error_t **error);

/**
 * @brief Lookups a raw stream subscription message
 *
 * @param[in] yamal
 * @param[in, out] iterator
 * @param[out] stream
 * @param[out] error out-parameter for error handling
 * @return bool true if a message was read, false otherwise
 */
FMMODFUNC bool ytp_subscription_next(ytp_yamal_t *yamal,
                                     ytp_iterator_t *iterator,
                                     ytp_mmnode_offs *stream,
                                     fmc_error_t **error);

/**
 * @brief Returns an iterator to the beginning of the list
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
static inline ytp_iterator_t ytp_subscription_begin(ytp_yamal_t *yamal,
                                                    fmc_error_t **error) {
  return ytp_yamal_begin(yamal, YTP_STREAM_LIST_SUBS, error);
}

/**
 * @brief Returns an iterator to the end of the list
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
static inline ytp_iterator_t ytp_subscription_end(ytp_yamal_t *yamal,
                                                  fmc_error_t **error) {
  return ytp_yamal_end(yamal, YTP_STREAM_LIST_SUBS, error);
}

#ifdef __cplusplus
}
#endif
