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
 * Timestamp is the original time of the message. Whenever the message is copied
 * or forwarded, it should maintain the timestamp.
 *
 * <table>
 * <caption id="multi_row">Time message</caption>
 * <tr><th colspan="4">peer/channel/time
 * <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>  variable
 * <tr><td>Peer ID  <td>Channel ID  <td>Timestmap  <td>Data
 * </table>
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_TIME_H__
#define __FM_YTP_TIME_H__

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/channel.h>
#include <ytp/peer.h>
#include <ytp/yamal.h>

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
FMMODFUNC char *ytp_time_reserve(ytp_yamal_t *yamal, size_t sz,
                                 fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list on the time level.
 *
 * @param[in] yamal
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] time
 * @param[in] data the value returned by ytp_peer_reserve if the node is not a
 * sublist. Otherwise the first_ptr returned by ytp_peer_sublist_commit
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, ytp_peer_t peer,
                                         ytp_channel_t channel, uint64_t time,
                                         void *data, fmc_error_t **error);

/**
 * @brief Commits a new data node to an existing sublist (first_ptr, last_ptr)
 * that is not in the main memory mapped list
 *
 * @param[in] yamal
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] time
 * @param[in, out] first_ptr an zero initialized atomic pointer for the first
 * node of the sublist
 * @param[in, out] last_ptr an zero initialized atomic pointer for the last node
 * of the sublist
 * @param[in] new_ptr the value returned by ytp_peer_reserve for the node that
 * is intended to insert
 * @param[out] error
 */
FMMODFUNC void ytp_time_sublist_commit(ytp_yamal_t *yamal, ytp_peer_t peer,
                                       ytp_channel_t channel, uint64_t time,
                                       void **first_ptr, void **last_ptr,
                                       void *new_ptr, fmc_error_t **error);

/**
 * @brief Reads a message on channel level
 *
 * @param[in] yamal
 * @param[in] iterator
 * @param[out] peer
 * @param[out] channel
 * @param[out] time
 * @param[out] sz
 * @param[out] data
 * @param[out] error
 */
FMMODFUNC void ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                             ytp_peer_t *peer, ytp_channel_t *channel,
                             uint64_t *time, size_t *sz, const char **data,
                             fmc_error_t **error);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_TIME_H__
