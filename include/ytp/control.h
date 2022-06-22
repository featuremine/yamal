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
 * @file control.h
 * @author Federico Ravchina
 * @date 23 Apr 2021
 * @brief File contains C declaration of control layer of YTP
 *
 * A control channel is used for communicating peer, channel, publisher and
 * subscription control information.
 *
 * A publisher is a peer that may publish messages on a channel.
 *
 * <table>
 * <caption id="multi_row">Control message</caption>
 * <tr><th colspan="3">peer/channel/time header  <th>control
 * <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>variable
 * <tr><td>Peer ID  <td>Channel ID  <td>Timestmap  <td>Payload
 * </table>
 *
 * Channel message
 * ========================
 *
 * A peer announces a channel.
 *
 * <table>
 * <caption id="multi_row">Channel message</caption>
 * <tr><th colspan="3">peer/channel/time header  <th>channel announcement
 * payload <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>variable <tr><td>Peer
 * ID  <td>Ctrl Channel ID = 0  <td>Timestmap  <td>Channel name
 * </table>
 *
 * The peer needs to avoid publishing duplicated channel messages if the last
 * channel message in yamal has the same payload.
 *
 * Subscribe message
 * ========================
 *
 * A peer announces a subscription to a channel.
 *
 * <table>
 * <caption id="multi_row">Subscribe message</caption>
 * <tr><th colspan="3">peer/channel/time  <th>subscribe payload
 * <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>variable
 * <tr><td>Peer ID  <td>Ctrl Channel ID = 1  <td>Timestmap  <td>Payload
 * </table>
 *
 * - Payload: If it doesn't end with character "/", a channel name; otherwise, a
 * prefix.
 *
 * The peer needs to avoid publishing duplicated subscribe messages if the last
 * subscription message in yamal has the same payload.
 *
 * Directory message
 * ========================
 *
 * Describes the messages that will be published on a particular channel.
 *
 * <table>
 * <caption id="multi_row">Directory message</caption>
 * <tr><th colspan="3">peer/channel/time header  <th>directory payload
 * <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>variable
 * <tr><td>Peer ID  <td>Ctrl Channel ID = 2  <td>Timestmap  <td>Payload
 * </table>
 *
 * - Payload : An Simple Channel Directory Protocol (SCDP) encoded string. This
 * field can be empty.
 *
 * The peer needs to avoid publishing duplicated directory message if the last
 * directory message in yamal has the same payload.
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/channel.h>
#include <ytp/peer.h>
#include <ytp/time.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Implements ytp control protocol
 *
 * Guaranties: control iterator is always at or ahead of
 * any ytp_iterator returned by the control API.
 */
typedef struct ytp_control ytp_control_t;

#define YTP_CHANNEL_ANN 0
#define YTP_CHANNEL_SUB 1
#define YTP_CHANNEL_DIR 2

#define YTP_CHANNEL_OFF 0x100

/**
 * @brief Allocates and initializes a ytp_control_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[out] error
 * @return ytp_control_t object
 */
FMMODFUNC ytp_control_t *ytp_control_new(fmc_fd fd, fmc_error_t **error);

/**
 * @brief Initializes a ytp_control_t object
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] fd a yamal file descriptor
 * @param[out] error
 */
FMMODFUNC void ytp_control_init(ytp_control_t *ctrl, fmc_fd fd,
                                fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_control_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[out] error
 * @param[in] enable_thread enable the preallocation and sync thread
 * @return ytp_control_t object
 */
FMMODFUNC ytp_control_t *ytp_control_new_2(fmc_fd fd, bool enable_thread,
                                           fmc_error_t **error);

/**
 * @brief Initializes a ytp_control_t object
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enable the preallocation and sync thread
 * @param[out] error
 */
FMMODFUNC void ytp_control_init_2(ytp_control_t *ctrl, fmc_fd fd,
                                  bool enable_thread, fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_control_t object
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[out] error
 */
FMMODFUNC void ytp_control_del(ytp_control_t *ctrl, fmc_error_t **error);

/**
 * @brief Destroys a ytp_control_t object
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[out] error
 */
FMMODFUNC void ytp_control_destroy(ytp_control_t *ctrl, fmc_error_t **error);

/**
 * @brief Reserves memory for data in the memory mapped list
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] sz the size of the data payload
 * @param[out] error
 * @return a writable pointer for data
 */
FMMODFUNC char *ytp_control_reserve(ytp_control_t *ctrl, size_t sz,
                                    fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] time
 * @param[in] data the value returned by ytp_control_reserve
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_control_commit(ytp_control_t *ctrl,
                                            ytp_peer_t peer,
                                            ytp_channel_t channel,
                                            uint64_t time, void *data,
                                            fmc_error_t **error);

/**
 * @brief Publishes a subscription message
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the message
 * @param[in] time
 * @param[in] sz
 * @param[in] payload a prefix or channel name
 * @param[out] error
 */
FMMODFUNC void ytp_control_sub(ytp_control_t *ctrl, ytp_peer_t peer,
                               uint64_t time, size_t sz, const char *payload,
                               fmc_error_t **error);

/**
 * @brief Publishes a directory message
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the message
 * @param[in] time
 * @param[in] sz
 * @param[in] payload a SCDP encoded string
 * @param[out] error
 */
FMMODFUNC void ytp_control_dir(ytp_control_t *ctrl, ytp_peer_t peer,
                               uint64_t time, size_t sz, const char *payload,
                               fmc_error_t **error);

/**
 * @brief Returns the name of the channel, given the channel reference
 *
 * Complexity: Constant on average, worst case linear in the number of channels.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] channel
 * @param[out] sz
 * @param[out] name
 * @param[out] error
 */
FMMODFUNC void ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel,
                                   size_t *sz, const char **name,
                                   fmc_error_t **error);

/**
 * @brief Declares an existing/new channel
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the channel announcement
 * @param[in] time
 * @param[in] sz
 * @param[in] name
 * @param[out] error
 * @return channel reference
 */
FMMODFUNC ytp_channel_t ytp_control_ch_decl(ytp_control_t *ctrl,
                                            ytp_peer_t peer, uint64_t time,
                                            size_t sz, const char *name,
                                            fmc_error_t **error);

/**
 * @brief Returns the name of the peer, given the peer reference
 *
 * Complexity: Constant on average, worst case linear in the number of peers.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the channel announcement
 * @param[out] sz
 * @param[out] name
 * @param[out] error
 */
FMMODFUNC void ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer,
                                     size_t *sz, const char **name,
                                     fmc_error_t **error);

/**
 * @brief Declares an existing/new peer
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] sz
 * @param[in] name
 * @param[out] error
 * @return peer reference
 */
FMMODFUNC ytp_peer_t ytp_control_peer_decl(ytp_control_t *ctrl, size_t sz,
                                           const char *name,
                                           fmc_error_t **error);

/**
 * @brief Finds next message on control level, moves iterator forward if there
 * is a next message.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] iter the ytp iterator
 * @param[out] error
 * @return the iterator of the next element
 *
 * Do check for error prior to using returned iterator.
 */
FMMODFUNC ytp_iterator_t ytp_control_next(ytp_control_t *ctrl,
                                          ytp_iterator_t iter,
                                          fmc_error_t **error);

/**
 * @brief Reads a message on control level
 *
 * Reads a message on a control level indicating in the case of peer
 * and channel announcement whether announcements are duplicated.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] iter
 * @param[out] peer
 * @param[out] channel
 * @param[out] time
 * @param[out] sz
 * @param[out] data
 * @param[out] error
 */
FMMODFUNC void ytp_control_read(ytp_control_t *ctrl, ytp_iterator_t iter,
                                ytp_peer_t *peer, ytp_channel_t *channel,
                                uint64_t *time, size_t *sz, const char **data,
                                fmc_error_t **error);

/**
 * @brief Returns the iterator to the begining of yamal
 *
 * @param[in] ctrl the ytp_control_t object
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_control_begin(ytp_control_t *ctrl,
                                           fmc_error_t **error);

/**
 * @brief Returns the iterator to the end of yamal
 *
 * Moves control pointer to the end.
 * @param[in] ctrl the ytp_control_t object
 * @param[out] error
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_control_end(ytp_control_t *ctrl,
                                         fmc_error_t **error);
/**
 * @brief Checks if there are not more messages
 *
 * @param[in] iterator
 * @return true if there are not more messages, false otherwise
 */
FMMODFUNC bool ytp_control_term(ytp_iterator_t iterator);

/**
 * @brief Returns an iterator given a serializable ptr
 *
 * Moves control pointer to catch up with iterator.
 * @param[in] ctrl
 * @param[in] off
 * @param[out] error
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_control_seek(ytp_control_t *ctrl, size_t off,
                                          fmc_error_t **error);

/**
 * @brief Returns serializable offset given an iterator
 *
 * @param[in] ctrl
 * @param[in] iterator
 * @param[out] error
 * @return serializable
 */
FMMODFUNC size_t ytp_control_tell(ytp_control_t *ctrl, ytp_iterator_t iterator,
                                  fmc_error_t **error);

#ifdef __cplusplus
}
#endif
