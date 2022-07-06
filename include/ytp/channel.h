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
 * @author Featuremine Corporation
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
#include <apr.h> // apr_size_t APR_DECLARE
#include <ytp/peer.h>
#include <ytp/yamal.h>
#include <ytp/errno.h> // ytp_status_t

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t ytp_channel_t;

/**
 * @brief Reserves memory for data in the memory mapped list
 * to be used to write to the file, on the channel level.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[out] buf a buffer to hold the reserved memory.
 * @param[in] size size of the buffer to hold the memory.
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_channel_reserve(ytp_yamal_t *yamal, char **buf, apr_size_t size);

/**
 * @brief Commits the data to the memory mapped node to write
 * it to the file, on the channel level.
 * 
 * @param[out] yamal ytp_yamal_t object
 * @param[out] it iterator to the next memory mapped node
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] data the value returned by ytp_channel_reserve
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_channel_commit(ytp_yamal_t *yamal, ytp_iterator_t *it, ytp_peer_t peer,
                                             ytp_channel_t channel, void *data);

/**
 * @brief Reads a message of the memory mapped node, on the channel level.
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[in] iterator iterator that points to the memory mapped node to read from
 * @param[out] peer the peer that wrote the data
 * @param[out] channel the channel that wrote the data
 * @param[out] size size of the read data
 * @param[out] data pointer to the read data
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_channel_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                           ytp_peer_t *peer, ytp_channel_t *channel,
                                           apr_size_t *size, const char **data);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_CHANNEL_H__
