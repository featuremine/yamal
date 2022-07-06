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
 * @file control.h
 * @author Featuremine Corporation
 * @date 23 Apr 2021
 * @brief File contains C declaration of control layer of YTP.\n
 * A control channel is used for communicating peer, channel, publisher and
 * subscription control information.
 * A publisher is a peer that may publish messages on a channel.
 *
 * @par Description
 * - Control message\n
 * <table>
 * <caption id="multi_row">Control message</caption>
 * <tr><th colspan="3">peer/channel/time header  <th>control
 * <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>variable
 * <tr><td>Peer ID  <td>Channel ID  <td>Timestmap  <td>Payload
 * </table>
 * - Channel message\n
 * A peer announces a channel.\n
 * <table>
 * <caption id="multi_row">Channel message</caption>
 * <tr><th colspan="3">peer/channel/time header  <th>channel announcement
 * payload <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>variable <tr><td>Peer
 * ID  <td>Ctrl Channel ID = 0  <td>Timestmap  <td>Channel name
 * </table>
 * The peer needs to avoid publishing duplicated channel messages if the last
 * channel message in yamal has the same payload.\n
 * - Subscribe message\n
 * A peer announces a subscription to a channel.\n
 * <table>
 * <caption id="multi_row">Subscribe message</caption>
 * <tr><th colspan="3">peer/channel/time  <th>subscribe payload
 * <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>variable
 * <tr><td>Peer ID  <td>Ctrl Channel ID = 1  <td>Timestmap  <td>Payload
 * </table>
 * - Payload: If it doesn't end with character "/", a channel name; otherwise, a
 * prefix.\n
 * The peer needs to avoid publishing duplicated subscribe messages if the last
 * subscription message in yamal has the same payload.
 * - Directory message\n
 * Describes the messages that will be published on a particular channel.\n
 * <table>
 * <caption id="multi_row">Directory message</caption>
 * <tr><th colspan="3">peer/channel/time header  <th>directory payload
 * <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>variable
 * <tr><td>Peer ID  <td>Ctrl Channel ID = 2  <td>Timestmap  <td>Payload
 * </table>
 * - Payload : An Simple Channel Directory Protocol (SCDP) encoded string. This
 * field can be empty.\n
 * The peer needs to avoid publishing duplicated directory message if the last
 * directory message in yamal has the same payload.
 * 
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_CONTROL_H__
#define __FM_YTP_CONTROL_H__

#include <stdbool.h> 
#include <stddef.h>
#include <stdint.h>

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_file_io.h> // apr_file_t
#include <ytp/yamal.h>
#include <ytp/channel.h>
#include <ytp/peer.h>
#include <ytp/time.h>
#include <ytp/errno.h> // ytp_status_t


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
 * @param[out] ctrl ytp_control_t object
 * @param[in] f APR file object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_new(ytp_control_t **ctrl, apr_file_t *f);

/**
 * @brief Initializes a ytp_control_t object
 *
 * @param[out] ctrl ytp_control_t object
 * @param[in] f APR file object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_init(ytp_control_t *ctrl, apr_file_t *f);

/**
 * @brief Allocates and initializes a ytp_control_t object
 *
 * @param[out] ctrl ytp_control_t object
 * @param[in] f APR file object
 * @param[in] enable_thread enable the preallocation and sync thread
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_new2(ytp_control_t **ctrl, apr_file_t *f, bool enable_thread);

/**
 * @brief Initializes a ytp_control_t object
 *
 * @param[out] ctrl ytp_control_t object
 * @param[in] f APR file object
 * @param[in] enable_thread enable the preallocation and sync thread
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_init2(ytp_control_t *ctrl, apr_file_t *f, bool enable_thread);

/**
 * @brief Destroys and deallocate a ytp_control_t object
 *
 * @param[out] ctrl ytp_control_t object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_del(ytp_control_t *ctrl);

/**
 * @brief Destroys a ytp_control_t object
 *
 * @param[out] ctrl ytp_control_t object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_destroy(ytp_control_t *ctrl);

/**
 * @brief Reserves memory for data in the memory mapped list
 * to be used to write to the file.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[out] buf A buffer to hold the reserved memory.
 * @param[in] size size of the buffer to hold the memory.
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_reserve(ytp_control_t *ctrl, char **buf, apr_size_t size);

/**
 * @brief Commits the data to the memory mapped node to write
 * it to the file.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[out] it iterator to the next memory mapped node
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] time the time to publish the data
 * @param[in] data the value returned by ytp_control_reserve
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_commit(ytp_control_t *ctrl, ytp_iterator_t *it, ytp_peer_t peer, ytp_channel_t channel, uint64_t time, void *data);
                      
/**
 * @brief Publishes a subscription message
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[in] peer the peer that publishes the subscription message
 * @param[in] time the time to publish the subscription message
 * @param[in] size size of the payload
 * @param[in] payload a prefix or channel name
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_sub(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time, apr_size_t size, const char *payload);

/**
 * @brief Publishes a directory message
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[in] peer the peer that publishes the directory message
 * @param[in] time the time to publish the directory message
 * @param[in] size size of the payload
 * @param[in] payload a SCDP encoded string
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_dir(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time, apr_size_t size, const char *payload);

/**
 * @brief Returns the name of the channel, given the channel reference
 *
 * Complexity: Constant on average, worst case linear in the number of channels.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[in] channel channel reference to obtain the name
 * @param[out] size size of the channel name
 * @param[out] name name of the channel
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel, apr_size_t *size, const char **name);

/**
 * @brief Declares an existing/new channel
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[out] ctrl ytp_control_t object
 * @param[out] channel channel reference declared
 * @param[in] peer the peer that publishes the channel announcement
 * @param[in] time the time to publish the channel announcement
 * @param[in] size size of the channel name
 * @param[in] name name of the channel
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_ch_decl(ytp_control_t *ctrl, ytp_channel_t *channel, ytp_peer_t peer, uint64_t time, apr_size_t size, const char *name);

/**
 * @brief Returns the name of the peer, given the peer reference
 *
 * Complexity: Constant on average, worst case linear in the number of peers.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[in] peer peer reference to obtain the name
 * @param[out] size size of the peer name
 * @param[out] name name of the peer
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer, apr_size_t *size, const char **name);

/**
 * @brief Declares an existing/new peer
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[out] peer peer reference declared
 * @param[in] size size of the peer name
 * @param[in] name name of the peer
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_peer_decl(ytp_control_t *ctrl, ytp_peer_t *peer, apr_size_t size, const char *name);

/**
 * @brief Finds next message on control level, moves iterator forward if there
 * is a next message.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[out] it_ptr iterator to the next message
 * @param[in] iter input iterator
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_next(ytp_control_t *ctrl, ytp_iterator_t *it_ptr, ytp_iterator_t iter);

/**
 * @brief Reads a message on control level
 *
 * Reads a message on a control level indicating in the case of peer
 * and channel announcement whether announcements are duplicated.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] it iterator that points to the message node to read from
 * @param[out] peer peer associated of the read message
 * @param[out] channel channel associated of the read message
 * @param[out] time time associated of the read message
 * @param[out] size size of the read message data
 * @param[out] data pointer to the read message data
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_read(ytp_control_t *ctrl, ytp_iterator_t it, ytp_peer_t *peer,
                                           ytp_channel_t *channel, uint64_t *time, apr_size_t *size, const char **data);

/**
 * @brief Obtains the iterator to the beginning of the list, the first node.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[out] iterator iterator to the beginning of the list
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_begin(ytp_control_t *ctrl, ytp_iterator_t *iterator);

/**
 * @brief Obtains the iterator to the end of the list, the last node.
 * Also moves control pointer to the end.
 * 
 * @param[in] ctrl ytp_control_t object
 * @param[out] iterator iterator to the end of the list
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_end(ytp_control_t *ctrl, ytp_iterator_t *iterator);

/**
 * @brief Checks if there are not more messages
 *
 * @param[in] iterator iterator to test
 * @return true if there are not more messages, false otherwise
 */
APR_DECLARE(bool) ytp_control_term(ytp_iterator_t iterator);

/**
 * @brief Obtains an iterator given a serializable ptr.
 * Also moves control pointer to catch up with iterator.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[out] it_ptr the iterator of the serializable ptr
 * @param[in] ptr the serializable ptr offset
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_seek(ytp_control_t *ctrl, ytp_iterator_t *it_ptr, apr_size_t ptr);

/**
 * @brief Obtains serializable ptr offset given an iterator
 *
 * @param[in] ctrl ytp_control_t object
 * @param[out] ptr the serializable ptr offset
 * @param[in] iterator the iterator of the serializable ptr
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_control_tell(ytp_control_t *ctrl, apr_size_t *ptr, ytp_iterator_t iterator);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_CONTROL_H__
