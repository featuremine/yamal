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
 * @file sequence.h
 * @author Featuremine Corporation
 * @date 23 Apr 2021
 * @brief File contains C declaration of sequence API of YTP
 *
 * This file contains C declaration of sequence API of YTP.
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_SEQUENCE_H__
#define __FM_YTP_SEQUENCE_H__

#include <stdbool.h> 
#include <stddef.h>
#include <stdint.h>

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_file_io.h> // apr_file_t
#include <ytp/channel.h>
#include <ytp/peer.h>
#include <ytp/yamal.h>
#include <ytp/errno.h> // ytp_status_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interface for reading and writing to ytp
 *
 * The ytp_sequence interface provides the ability to read and write to ytp
 * using callbacks for reading, on top of the control layer of ytp
 *
 * Internally, it uses the ytp_control and ytp_timeline API.
 *
 * Guaranties: control iterator is always at or ahead of
 * any ytp_iterator returned by the sequence API.
 */
struct ytp_sequence;
typedef struct ytp_sequence ytp_sequence_t;

/**
 * @brief Interface for shared access to a unique ytp_sequence object
 *
 * The ytp_sequence_shared interface provides the ability share access to a
 * ytp_sequence object by using a reference counter
 */
struct ytp_sequence_shared;
typedef struct ytp_sequence_shared ytp_sequence_shared_t;

typedef void (*ytp_sequence_peer_cb_t)(void *closure, ytp_peer_t peer,
                                       apr_size_t sz, const char *name);
typedef void (*ytp_sequence_ch_cb_t)(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, uint64_t time,
                                     apr_size_t sz, const char *name);
typedef void (*ytp_sequence_data_cb_t)(void *closure, ytp_peer_t peer,
                                               ytp_channel_t channel, uint64_t time,
                                               apr_size_t sz, const char *data);

/**
 * @brief Allocates and initializes a ytp_sequence_t object
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] f APR file object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_new(ytp_sequence_t **seq, apr_file_t *f);

/**
 * @brief Initializes a ytp_sequence_t object
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] f APR file object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_init(ytp_sequence_t *seq, apr_file_t *f);

/**
 * @brief Allocates and initializes a ytp_sequence_t object
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] f APR file object
 * @param[in] enable_thread enable the preallocation and sync thread
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_new2(ytp_sequence_t **seq, apr_file_t *f, bool enable_thread);

/**
 * @brief Initializes a ytp_sequence_t object
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] f APR file object
 * @param[in] enable_thread enable the preallocation and sync thread
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_init2(ytp_sequence_t *seq, apr_file_t *f, bool enable_thread);

/**
 * @brief Destroys and deallocate a ytp_sequence_t object
 *
 * @param[out] seq ytp_sequence_t object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_del(ytp_sequence_t *seq);

/**
 * @brief Destroys a ytp_sequence_t object
 *
 * @param[out] seq ytp_sequence_t object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_destroy(ytp_sequence_t *seq);

/**
 * @brief Reserves memory for data in the memory mapped list
 * to be used to write to the file.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[out] buf a buffer to hold the reserved memory.
 * @param[in] size size of the buffer to hold the memory.
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_reserve(ytp_sequence_t *seq, char **buf, apr_size_t size);

/**
 * @brief Commits the data to the memory mapped node to write
 * it to the file.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] it iterator to the next memory mapped node
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] time the time to publish the data
 * @param[in] data the value returned by ytp_sequence_reserve
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_commit(ytp_sequence_t *seq, ytp_iterator_t *it, ytp_peer_t peer,
                                              ytp_channel_t channel, uint64_t time, void *data);

/**
 * @brief Publishes a subscription message if it is not already published.
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] peer the peer that publishes the subscription message
 * @param[in] time the time to publish the subscription message
 * @param[in] sz size of the payload
 * @param[in] payload a prefix or channel name
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_sub(ytp_sequence_t *seq, ytp_peer_t peer, uint64_t time,
                                           apr_size_t sz, const char *payload);

/**
 * @brief Publishes a directory message if it is not already published.
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] peer the peer that publishes the directory message
 * @param[in] time the time to publish the directory message
 * @param[in] sz size of the payload
 * @param[in] payload a SCDP encoded string
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_dir(ytp_sequence_t *seq, ytp_peer_t peer, uint64_t time,
                                           apr_size_t sz, const char *payload);

/**
 * @brief Returns the name of the channel, given the channel reference
 *
 * Complexity: Constant on average, worst case linear in the number of channels.
 *
 * @param[in] seq ytp_sequence_t object
 * @param[in] channel channel reference to obtain the name
 * @param[out] sz size of the channel name
 * @param[out] name name of the channel
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_ch_name(ytp_sequence_t *seq, ytp_channel_t channel, apr_size_t *sz, const char **name);

/**
 * @brief Declares an existing/new channel
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[out] channel channel reference declared
 * @param[in] peer the peer that publishes the channel announcement
 * @param[in] time the time to publish the channel announcement
 * @param[in] sz size of the channel name
 * @param[in] name name of the channel
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_ch_decl(ytp_sequence_t *seq, ytp_channel_t *channel, ytp_peer_t peer,
                                               uint64_t time, apr_size_t sz, const char *name);

/**
 * @brief Registers a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] cb channel announcement callback
 * @param[in] closure closure data for the channel announcement callback
 */
APR_DECLARE(void) ytp_sequence_ch_cb(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb, void *closure);

/**
 * @brief Unregisters a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] cb channel announcement callback
 * @param[in] closure closure data for the channel announcement callback
 */
APR_DECLARE(void) ytp_sequence_ch_cb_rm(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb, void *closure);

/**
 * @brief Returns the name of the peer, given the peer reference
 *
 * Complexity: Constant on average, worst case linear in the number of peers.
 *
 * @param[in] seq ytp_sequence_t object
 * @param[in] peer peer reference to obtain the name
 * @param[out] sz size of the peer name
 * @param[out] name name of the peer
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_peer_name(ytp_sequence_t *seq, ytp_peer_t peer, apr_size_t *sz, const char **name);

/**
 * @brief Declares an existing/new peer
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] seq ytp_sequence_t object
 * @param[out] peer peer reference declared
 * @param[in] sz size of the peer name
 * @param[in] name name of the peer
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_peer_decl(ytp_sequence_t *seq, ytp_peer_t *peer, apr_size_t sz, const char *name);

/**
 * @brief Registers a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] cb peer announcement callback
 * @param[in] closure closure data for the peer announcement callback
 */
APR_DECLARE(void) ytp_sequence_peer_cb(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb, void *closure);

/**
 * @brief Unregisters a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] cb peer announcement callback
 * @param[in] closure closure data for the peer announcement callback
 */
APR_DECLARE(void) ytp_sequence_peer_cb_rm(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb, void *closure);

/**
 * @brief Registers a channel data callback by channel name or prefix
 *
 * Subscribes to data on any channel matching the prefix if chprfx terminates
 * with '/' or exact channel name instead
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] sz size of the channel prefix
 * @param[in] prfx channel prefix
 * @param[in] cb channel data callback
 * @param[in] closure closure data for the channel data callback
 */
APR_DECLARE(void) ytp_sequence_prfx_cb(ytp_sequence_t *seq, apr_size_t sz, const char *prfx,
                                       ytp_sequence_data_cb_t cb, void *closure);
/**
 * @brief Unregisters a channel data callback by channel name or prefix
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] sz size of the channel prefix
 * @param[in] prfx channel prefix
 * @param[in] cb channel data callback
 * @param[in] closure closure data for the channel data callback
 */
APR_DECLARE(void) ytp_sequence_prfx_cb_rm(ytp_sequence_t *seq, apr_size_t sz, const char *prfx,
                                          ytp_sequence_data_cb_t cb, void *closure);

/**
 * @brief Registers a channel data callback by channel reference
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] channel channel reference
 * @param[in] cb channel data callback
 * @param[in] closure closure data for the channel data callback
 */
APR_DECLARE(void) ytp_sequence_indx_cb(ytp_sequence_t *seq, ytp_channel_t channel,
                                       ytp_sequence_data_cb_t cb, void *closure);

/**
 * @brief Unregisters a channel data callback by channel reference
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] channel channel reference
 * @param[in] cb channel data callback
 * @param[in] closure closure data for the channel data callback
 */
APR_DECLARE(void) ytp_sequence_indx_cb_rm(ytp_sequence_t *seq, ytp_channel_t channel,
                                          ytp_sequence_data_cb_t cb, void *closure);

/**
 * @brief Reads one message and executes the callbacks that applies.
 *
 * @param[in] seq ytp_sequence_t object
 * @param[out] new_data true if a message was processed, false otherwise
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_poll(ytp_sequence_t *seq, bool *new_data);

/**
 * @brief Checks if there are not more messages
 *
 * Complexity: Constant.
 *
 * @param[in] seq ytp_sequence_t object
 * @return true if there are not more messages, false otherwise
 */
APR_DECLARE(bool) ytp_sequence_term(ytp_sequence_t *seq);

/**
 * @brief Obtains the iterator to the end of the list, the last node.
 * Also moves control pointer to the end.
 *
 * Complexity: Constant.
 *
 * @param[in] seq ytp_sequence_t object
 * @param[out] iterator iterator to the end of the list
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_end(ytp_sequence_t *seq, ytp_iterator_t *iterator);

/**
 * @brief Returns the current data iterator
 *
 * @param[in] seq ytp_sequence_t object
 * @return the current data iterator
 */
APR_DECLARE(ytp_iterator_t) ytp_sequence_get_it(ytp_sequence_t *seq);

/**
 * @brief Sets the current data iterator
 *
 * @param[out] seq ytp_sequence_t object
 * @param[in] iterator iterator to be set
 */
APR_DECLARE(void) ytp_sequence_set_it(ytp_sequence_t *seq, ytp_iterator_t iterator);

/**
 * @brief Obtains an iterator given a serializable ptr offset.
 * Also moves control pointer to catch up with iterator.
 *
 * @param[in] seq ytp_sequence_t object
 * @param[out] it_ptr the iterator of the serializable ptr
 * @param[in] ptr the serializable ptr offset
 * @return ytp_status_t with the outcome of the function
 *
 * Sets data offset to offset. All control messages from the
 * beginning up to the offset are guarantied do have been processed
 * and the callbacks to be called.
 */
APR_DECLARE(ytp_status_t) ytp_sequence_seek(ytp_sequence_t *seq, ytp_iterator_t *it_ptr, apr_size_t ptr);

/**
 * @brief Obtains serializable ptr offset given an iterator
 *
 * @param[in] seq ytp_sequence_t object
 * @param[out] ptr the serializable ptr offset
 * @param[in] iterator the iterator of the serializable ptr
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_tell(ytp_sequence_t *seq, apr_size_t *ptr, ytp_iterator_t iterator);

/**
 * @brief Allocates and initializes a ytp_sequence_shared object with a
 * reference counter equal to one
 *
 * @param[out] shared_seq ytp_sequence_shared_t object
 * @param[in] filename a yamal file path
 * @param[out] flag APR file creation flags
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_sequence_shared_new(ytp_sequence_shared_t **shared_seq, const char *filename, apr_int32_t flag);


/**
 * @brief Increases the reference counter
 *
 * @param[out] shared_seq ytp_sequence_shared_t object
 */
APR_DECLARE(void) ytp_sequence_shared_inc(ytp_sequence_shared_t *shared_seq);

/**
 * @brief Decreases the reference counter and call ytp_sequence_del the sequence
 * if the reference counter is zero
 *
 * @param[in] shared_seq ytp_sequence_shared_t object
 */
APR_DECLARE(void) ytp_sequence_shared_dec(ytp_sequence_shared_t *shared_seq);

/**
 * @brief Returns the ytp_sequence_t object
 *
 * @param[in] shared_seq ytp_sequence_shared_t object
 * @return associated ytp_sequence_t object
 */
APR_DECLARE(ytp_sequence_t *) ytp_sequence_shared_get(ytp_sequence_shared_t *shared_seq);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_SEQUENCE_H__
