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
 * @file channel.h
 * @date 23 Apr 2021
 * @brief File contains C declaration of channel layer of YTP.\n
 * A channel uniquely identifies a logical partition of a set of messages.
 *
 * @par Description
 * - Channel data message\n
 * <table>
 * <caption id="multi_row">Channel message</caption>
 * <tr><th colspan="3">peer/channel
 * <tr><th>8 bytes  <th>8 bytes  <th>  variable
 * <tr><td>Peer ID  <td>Channel ID  <td>Data
 * </table>
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_CHANNEL_H__
#define __FM_YTP_CHANNEL_H__

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/api.h>
#include <ytp/peer.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reserves memory for data in the memory mapped list
 * to be used to write to the file, on the channel level.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] sz size of the buffer to hold the memory
 * @param[out] error out-parameter for error handling
 * @return a buffer to hold the reserved memory
 */
FMMODFUNC char *ytp_channel_reserve(ytp_yamal_t *yamal, size_t sz,
                                    fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped node to write
 * it to the file, on the channel level.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] data the value returned by ytp_channel_reserve
 * @param[out] error out-parameter for error handling
 * @return iterator to the next memory mapped node
 */
FMMODFUNC ytp_iterator_t ytp_channel_commit(ytp_yamal_t *yamal, ytp_peer_t peer,
                                            ytp_channel_t channel, void *data,
                                            fmc_error_t **error);

/**
 * @brief Reads a message on channel level
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[in] iterator iterator that points to the memory mapped node to read
 * from
 * @param[out] peer the peer that wrote the data
 * @param[out] channel the channel that wrote the data
 * @param[out] sz size of the read data
 * @param[out] data pointer to the read data
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_channel_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                ytp_peer_t *peer, ytp_channel_t *channel,
                                size_t *sz, const char **data,
                                fmc_error_t **error);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_CHANNEL_H__
