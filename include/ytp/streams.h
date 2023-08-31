/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file streams.h
 * @date 6 Jun 2023
 * @brief File contains C declaration of streams API
 *
 *  This file contains C declaration of streams API of stream protocol level of
 * YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <ytp/announcement.h>
#include <ytp/stream.h>
#include <ytp/yamal.h>

#include <fmc/error.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ytp_streams ytp_streams_t;

/**
 * @brief Allocates and initializes a ytp_streams_t object
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_streams_t object
 */
FMMODFUNC ytp_streams_t *ytp_streams_new(ytp_yamal_t *yamal,
                                         fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_streams_t object
 *
 * @param[in] streams
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_streams_del(ytp_streams_t *streams, fmc_error_t **error);

/**
 * @brief Announces a stream in adherence to stream protocol
 *
 * @param[in] yamal
 * @param[in] psz peer size
 * @param[in] peer peer name
 * @param[in] csz channel size
 * @param[in] channel channel name
 * @param[in] esz encoding size
 * @param[in] encoding encoding metadata
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_mmnode_offs ytp_streams_announce(ytp_streams_t *streams,
                                               size_t psz, const char *peer,
                                               size_t csz, const char *channel,
                                               size_t esz, const char *encoding,
                                               fmc_error_t **error);

/**
 * @brief Lookups a stream in adherence to stream protocol
 *
 * @param[in] yamal
 * @param[in] psz peer size
 * @param[in] peer peer name
 * @param[in] csz channel size
 * @param[in] channel channel name
 * @param[out] esz encoding size
 * @param[out] encoding encoding metadata
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_mmnode_offs ytp_streams_lookup(ytp_streams_t *streams, size_t psz,
                                             const char *peer, size_t csz,
                                             const char *channel, size_t *esz,
                                             const char **encoding,
                                             fmc_error_t **error);

struct ytp_streams_anndata_t {
  uint64_t seqno;
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
  size_t esz;
  const char *encoding;
  ytp_mmnode_offs stream;
  ytp_mmnode_offs *original;
  ytp_mmnode_offs *subscribed;
};

enum ytp_streams_pred_result {
  YTP_STREAMS_PRED_CONTINUE,
  YTP_STREAMS_PRED_DONE,
  YTP_STREAMS_PRED_ROLLBACK,
};

FMMODFUNC void ytp_streams_search_ann(
    ytp_yamal_t *yamal, ytp_iterator_t *iterator,
    enum ytp_streams_pred_result (*predicate)(
        void *closure, const struct ytp_streams_anndata_t *ann,
        fmc_error_t **error),
    void *closure, fmc_error_t **error);

#ifdef __cplusplus
}
#endif
