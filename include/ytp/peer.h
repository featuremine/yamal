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
 * @file peer.h
 * @author Featuremine Corporation
 * @date 23 Apr 2021
 * @brief File contains C declaration of peer layer of YTP.\n
 * A peer uniquely identifies a source of a messages.
 *
 * @par Description
 * - Peer data message\n
 * <table>
 * <caption id="multi_row">Normal message</caption>
 * <tr><th colspan="2">peer
 * <tr><th>8 bytes  <th>  variable
 * <tr><td>Peer ID != 0  <td>  Data
 * </table>
 * If Peer ID = 0, then it is an announcement message where data is the peer
 * name.
 * - Peer announcement message\n
 * <table>
 * <caption id="multi_row">Announcement message</caption>
 * <tr><th colspan="3">peer
 * <tr><th>8 bytes  <th>  variable
 * <tr><td>Peer ID = 0 <td>  Peer name
 * </table>
 * The peer needs to avoid publishing duplicated announcement messages if there
 * is an announcement message in yamal.
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_PEER_H__
#define __FM_YTP_PEER_H__

#include <apr.h> // apr_size_t APR_DECLARE
#include <stdbool.h> 
#include <stddef.h>
#include <stdint.h>
#include <ytp/yamal.h>
#include <ytp/errno.h> // ytp_status_t

#ifdef __cplusplus
extern "C" {
#endif

#define YTP_PEER_ANN 0
#define YTP_PEER_CTL 1

#define YTP_PEER_OFF 0x100

typedef uint64_t ytp_peer_t;

/**
 * @brief Reserves memory for data in the memory mapped list
 * to be used to write to the file, on the peer level.
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[out] buf a buffer to hold the reserved memory.
 * @param[in] size size of the buffer to hold the memory.
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_peer_reserve(ytp_yamal_t *yamal, char **buf, apr_size_t size);

/**
 * @brief Commits the data to the memory mapped node to write
 * it to the file, on the channel level.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[out] it iterator to the next memory mapped node
 * @param[in] peer the peer that publishes the data
 * @param[in] data the value returned by ytp_peer_reserve
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_peer_commit(ytp_yamal_t *yamal, ytp_iterator_t *it, ytp_peer_t peer, void *data);

/**
 * @brief Declares an existing/new peer
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[out] it iterator to the next memory mapped node
 * @param[in] sz size of the buffer to hold the peer name.
 * @param[in] name buffer that hold the peer name.
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_peer_name(ytp_yamal_t *yamal, ytp_iterator_t *it, apr_size_t sz, const char *name);

/**
 * @brief Reads a message of the memory mapped node, on the peer level.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] iterator iterator that points to the memory mapped node to read from
 * @param[out] peer the peer that wrote the data
 * @param[out] size size of the read data
 * @param[out] data pointer to the read data
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_peer_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                        ytp_peer_t *peer, apr_size_t *size, const char **data);

/**
 * @brief Checks if if peer is announcement peer
 *
 * @param[in] peer the input peer to check if it is announcement
 * @return true if the peer is an announcement peer, false otherwise
 */
APR_DECLARE(bool) ytp_peer_ann(ytp_peer_t peer);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_PEER_H__
