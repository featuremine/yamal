/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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
 * @author Federico Ravchina
 * @date 23 Apr 2021
 * @brief File contains C declaration of peer layer of YTP
 *
 * A peer uniquely identifies a source of a messages.
 *
 * <table>
 * <caption id="multi_row">Normal message</caption>
 * <tr><th colspan="2">peer
 * <tr><th>8 bytes  <th>  variable
 * <tr><td>Peer ID != 0  <td>  Data
 * </table>
 *
 * If Peer ID = 0, then it is an announcement message where data is the peer
 * name.
 *
 * <table>
 * <caption id="multi_row">Announcement message</caption>
 * <tr><th colspan="3">peer
 * <tr><th>8 bytes  <th>  variable
 * <tr><td>Peer ID = 0 <td>  Peer name
 * </table>
 *
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
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YTP_PEER_ANN 0
#define YTP_PEER_CTL 1

#define YTP_PEER_OFF 0x100

typedef uint64_t ytp_peer_t;

/**
 * @brief Reserves memory for data in the memory mapped list
 *
 * @param[in] yamal
 * @param[in] sz the size of the data payload
 * @param[out] error
 * @return a writable pointer for data
 */
FMMODFUNC char *ytp_peer_reserve(ytp_yamal_t *yamal, size_t sz,
                                 fmc_error_t **error);
/**
 * @brief Commits the data to the memory mapped list
 *
 * @param[in] yamal
 * @param[in] peer the peer that publishes the data
 * @param[in] data the value returned by ytp_peer_reserve
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_peer_commit(ytp_yamal_t *yamal, ytp_peer_t peer,
                                         void *data, fmc_error_t **error);

/**
 * @brief Declares an existing/new peer
 *
 * @param[in] yamal
 * @param[in] sz
 * @param[in] name
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_peer_name(ytp_yamal_t *yamal, size_t sz,
                                       const char *name, fmc_error_t **error);

/**
 * @brief Reads a message on peer level
 *
 * @param[in] yamal
 * @param[in] iterator
 * @param[out] peer
 * @param[out] sz
 * @param[out] data
 * @param[out] error
 */
FMMODFUNC void ytp_peer_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                             ytp_peer_t *peer, size_t *sz, const char **data,
                             fmc_error_t **error);

/**
 * @brief Checks if if peer is announcement peer
 *
 * @param[in] peer
 * @return true if peer is announcement peer, false otherwise
 */
FMMODFUNC bool ytp_peer_ann(ytp_peer_t peer);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_PEER_H__
