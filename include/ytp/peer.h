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

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/api.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YTP_PEER_ANN 0
#define YTP_PEER_CTL 1

#define YTP_PEER_OFF 0x100

/**
 * @brief Reserves memory for data in the memory mapped list
 * to be used to write to the file, on the peer level.
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[in] sz size of the buffer to hold the memory
 * @param[out] error out-parameter for error handling
 * @return a buffer to hold the reserved memory
 */
FMMODFUNC char *ytp_peer_reserve(ytp_yamal_t *yamal, size_t sz,
                                 fmc_error_t **error);
/**
 * @brief Commits the data to the memory mapped node to write
 * it to the file, on the channel level.
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[in] peer the peer that publishes the data
 * @param[in] data the value returned by ytp_peer_reserve()
 * @param[out] error out-parameter for error handling
 * @return iterator to the next memory mapped node
 */
FMMODFUNC ytp_iterator_t ytp_peer_commit(ytp_yamal_t *yamal, ytp_peer_t peer,
                                         void *data, fmc_error_t **error);

/**
 * @brief Declares an existing/new peer
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[in] sz size of the buffer to hold the peer name
 * @param[in] name buffer that hold the peer name
 * @param[out] error out-parameter for error handling
 * @return iterator to the next memory mapped node
 */
FMMODFUNC ytp_iterator_t ytp_peer_name(ytp_yamal_t *yamal, size_t sz,
                                       const char *name, fmc_error_t **error);

/**
 * @brief Reads a message of the memory mapped node, on the peer level.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] iterator iterator that points to the memory mapped node to read
 * from
 * @param[out] peer the peer that wrote the data
 * @param[out] sz size of the read data
 * @param[out] data pointer to the read data
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_peer_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                             ytp_peer_t *peer, size_t *sz, const char **data,
                             fmc_error_t **error);

/**
 * @brief Checks if peer is an announcement peer
 *
 * @param[in] peer the input peer to check if it is announcement
 * @return true if peer is an announcement peer, false otherwise
 */
FMMODFUNC bool ytp_peer_ann(ytp_peer_t peer);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_PEER_H__
