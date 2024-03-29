/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file announcement.h
 * @date 6 Jun 2023
 * @brief File contains C declaration of announcement API
 *
 *  This file contains C declaration of announcement API of stream protocol
 * level of YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <ytp/stream.h>
#include <ytp/yamal.h>

#include <fmc/error.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Writes a raw stream announcement message
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
FMMODFUNC ytp_iterator_t ytp_announcement_write(ytp_yamal_t *yamal, size_t psz,
                                                const char *peer, size_t csz,
                                                const char *channel, size_t esz,
                                                const char *encoding,
                                                fmc_error_t **error);

/**
 * @brief Read an announcement message
 *
 * @param[in] yamal
 * @param[in] iterator
 * @param[out] seqno
 * @param[out] psz peer size
 * @param[out] peer peer name
 * @param[out] csz channel size
 * @param[out] channel channel name
 * @param[out] esz encoding size
 * @param[out] encoding encoding metadata
 * @param[out] original offset of the original announcement, zero if
 * uninitialized
 * @param[out] subscribed offset of the first subscribe message
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void
ytp_announcement_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                      uint64_t *seqno, size_t *psz, const char **peer,
                      size_t *csz, const char **channel, size_t *esz,
                      const char **encoding, ytp_mmnode_offs **original,
                      ytp_mmnode_offs **subscribed, fmc_error_t **error);

/**
 * @brief Look up an announcement message
 *
 * @param[in] yamal
 * @param[in] stream
 * @param[out] seqno
 * @param[out] psz peer size
 * @param[out] peer peer name
 * @param[out] csz channel size
 * @param[out] channel channel name
 * @param[out] esz encoding size
 * @param[out] encoding encoding metadata
 * @param[out] original offset of the original announcement, zero if
 * uninitialized
 * @param[out] subscribed offset of the first subscribe message
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void
ytp_announcement_lookup(ytp_yamal_t *yamal, ytp_mmnode_offs stream,
                        uint64_t *seqno, size_t *psz, const char **peer,
                        size_t *csz, const char **channel, size_t *esz,
                        const char **encoding, ytp_mmnode_offs **original,
                        ytp_mmnode_offs **subscribed, fmc_error_t **error);

/**
 * @brief Returns an iterator to the beginning of the list
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_announcement_begin(ytp_yamal_t *yamal,
                                                fmc_error_t **error);

/**
 * @brief Returns iterator for the next announcement message
 *
 * @param[in] yamal
 * @param[in, out] iterator
 * @param[out] seqno
 * @param[out] psz peer size
 * @param[out] peer peer name
 * @param[out] csz channel size
 * @param[out] channel channel name
 * @param[out] esz encoding size
 * @param[out] encoding encoding metadata
 * @param[out] original offset of the original announcement, zero if
 * uninitialized
 * @param[out] subscribed offset of the first subscribe message
 * @param[out] error out-parameter for error handling
 * @return true if an announcement has been read, false otherwise
 */
FMMODFUNC bool
ytp_announcement_next(ytp_yamal_t *yamal, ytp_iterator_t *iterator,
                      uint64_t *seqno, ytp_mmnode_offs *stream, size_t *psz,
                      const char **peer, size_t *csz, const char **channel,
                      size_t *esz, const char **encoding,
                      ytp_mmnode_offs **original, ytp_mmnode_offs **subscribed,
                      fmc_error_t **error);

#ifdef __cplusplus
}
#endif
