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
 * @param[out] error
 * @return ytp_streams_t object
 */
FMMODFUNC ytp_streams_t *ytp_streams_new(ytp_yamal_t *yamal,
                                         fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_streams_t object
 *
 * @param[in] streams
 * @param[out] error
 */
FMMODFUNC void ytp_streams_del(ytp_streams_t *streams, fmc_error_t **error);

/**
 * @brief Announces a stream in adherance to stream protocol
 *
 * @param[in] yamal
 * @param[in] psz peer size
 * @param[in] peer peer name
 * @param[in] csz channel size
 * @param[in] channel channel name
 * @param[in] esz encoding size
 * @param[in] encoding encoding metadata
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_mmnode_offs ytp_streams_announce(ytp_streams_t *streams,
                                               size_t psz, const char *peer,
                                               size_t csz, const char *channel,
                                               size_t esz, const char *encoding,
                                               fmc_error_t **error);

/**
 * @brief Announces a stream in adherance to stream protocol
 *
 * @param[in] yamal
 * @param[in] psz peer size
 * @param[in] peer peer name
 * @param[in] csz channel size
 * @param[in] channel channel name
 * @param[out] esz encoding size
 * @param[out] encoding encoding metadata
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_mmnode_offs ytp_streams_lookup(ytp_streams_t *streams, size_t psz,
                                             const char *peer, size_t csz,
                                             const char *channel, size_t *esz,
                                             const char **encoding,
                                             fmc_error_t **error);

#ifdef __cplusplus
}
#endif
