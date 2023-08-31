/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file sequence.h
 * @date 23 Apr 2021
 * @brief File contains C declaration of sequence API of YTP
 *
 * This file contains C declaration of sequence API of YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/api.h>
#include <ytp/yamal.h>

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

/**
 * @brief Allocates and initializes a ytp_sequence_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[out] error out-parameter for error handling
 * @return ytp_sequence_t object
 */
FMMODFUNC ytp_sequence_t *ytp_sequence_new(fmc_fd fd, fmc_error_t **error);

/**
 * @brief Initializes a ytp_sequence_t object
 *
 * @param[out] seq the ytp_sequence_t object
 * @param[in] fd the yamal file descriptor
 * @param[out] error out-parameter for error handling
 */

FMMODFUNC void ytp_sequence_init(ytp_sequence_t *seq, fmc_fd fd,
                                 fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_sequence_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enables the auxiliary thread
 * @param[out] error out-parameter for error handling
 * @return ytp_sequence_t object
 */
FMMODFUNC ytp_sequence_t *ytp_sequence_new_2(fmc_fd fd, bool enable_thread,
                                             fmc_error_t **error);

/**
 * @brief Initializes a ytp_sequence_t object
 *
 * @param[out] seq the ytp_sequence_t object
 * @param[in] fd the yamal file descriptor
 * @param[in] enable_thread enables the auxiliary thread
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_init_2(ytp_sequence_t *seq, fmc_fd fd,
                                   bool enable_thread, fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_sequence_t object
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_del(ytp_sequence_t *seq, fmc_error_t **error);

/**
 * @brief Destroys a ytp_sequence_t object
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_destroy(ytp_sequence_t *seq, fmc_error_t **error);

/**
 * @brief Reserves memory for data in the memory mapped list
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] sz the size of the data payload
 * @param[out] error out-parameter for error handling
 * @return buffer to hold the reserved memory
 */
FMMODFUNC char *ytp_sequence_reserve(ytp_sequence_t *seq, size_t sz,
                                     fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list on the sequence level.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] msgtime the time to publish the message
 * @param[in] data the value returned by ytp_peer_reserve if the node is not a
 * sublist. Otherwise the first_ptr returned by ytp_peer_sublist_commit
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_sequence_commit(ytp_sequence_t *seq,
                                             ytp_peer_t peer,
                                             ytp_channel_t channel, int64_t ts,
                                             void *data, fmc_error_t **error);

/**
 * @brief Commits a new data node to an existing sublist (first_ptr, last_ptr)
 * that is not in the main memory mapped list
 *
 * @param[in] seq
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] ts
 * @param[in, out] first_ptr an zero initialized atomic pointer for the first
 * node of the sublist
 * @param[in, out] last_ptr an zero initialized atomic pointer for the last node
 * of the sublist
 * @param[in] new_ptr the value returned by ytp_peer_reserve for the node that
 * is intended to insert
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_sublist_commit(ytp_sequence_t *seq, ytp_peer_t peer,
                                           ytp_channel_t channel, int64_t ts,
                                           void **first_ptr, void **last_ptr,
                                           void *new_ptr, fmc_error_t **error);

/**
 * @brief Commits the sublist to the memory mapped list on the sequence level.
 *
 * @param[in] seq
 * @param[in] first_ptr the first node of the sublist
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC ytp_iterator_t ytp_sequence_sublist_finalize(ytp_sequence_t *seq,
                                                       void *first_ptr,
                                                       fmc_error_t **error);

/**
 * @brief Publishes a subscription message
 *
 * Publishes a subscription message if it is not already published.
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] seq
 * @param[in] peer the peer that publishes the subscription
 * @param[in] ts
 * @param[in] sz
 * @param[in] payload
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_sub(ytp_sequence_t *seq, ytp_peer_t peer,
                                int64_t ts, size_t sz, const char *payload,
                                fmc_error_t **error);

/**
 * @brief Publishes a directory message
 *
 * Publishes a directory message if it is not already published.
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] seq
 * @param[in] peer the peer that publishes the message
 * @param[in] ts
 * @param[in] sz
 * @param[in] payload
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_dir(ytp_sequence_t *seq, ytp_peer_t peer,
                                int64_t ts, size_t sz, const char *payload,
                                fmc_error_t **error);

/**
 * @brief Returns the name of the channel, given the channel reference
 *
 * Complexity: Constant on average, worst case linear in the number of channels.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] channel channel reference to obtain the name
 * @param[out] sz size of the channel name
 * @param[out] name name of the channel
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_ch_name(ytp_sequence_t *seq, ytp_channel_t channel,
                                    size_t *sz, const char **name,
                                    fmc_error_t **error);

/**
 * @brief Declares an existing/new channel
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] peer the peer that publishes the channel announcement
 * @param[in] msgtime the time to publish the channel announcement
 * @param[in] sz size of the channel name
 * @param[in] name name of the channel
 * @param[out] error out-parameter for error handling
 * @return channel reference
 */
FMMODFUNC ytp_channel_t ytp_sequence_ch_decl(ytp_sequence_t *seq,
                                             ytp_peer_t peer, int64_t ts,
                                             size_t sz, const char *name,
                                             fmc_error_t **error);

/**
 * @brief Registers a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_ch_cb(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb,
                                  void *closure, fmc_error_t **error);

/**
 * @brief Unregisters a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_ch_cb_rm(ytp_sequence_t *seq,
                                     ytp_sequence_ch_cb_t cb, void *closure,
                                     fmc_error_t **error);

/**
 * @brief Returns the name of the peer, given the peer reference
 *
 * Complexity: Constant on average, worst case linear in the number of peers.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] peer peer reference to obtain the name
 * @param[out] sz size of the peer name
 * @param[out] name name of the peer
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_peer_name(ytp_sequence_t *seq, ytp_peer_t peer,
                                      size_t *sz, const char **name,
                                      fmc_error_t **error);

/**
 * @brief Declares an existing/new peer
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] sz size of the peer name
 * @param[in] name name of the peer
 * @param[out] error out-parameter for error handling
 * @return peer reference
 */
FMMODFUNC ytp_peer_t ytp_sequence_peer_decl(ytp_sequence_t *seq, size_t sz,
                                            const char *name,
                                            fmc_error_t **error);

/**
 * @brief Registers a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_peer_cb(ytp_sequence_t *seq,
                                    ytp_sequence_peer_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Unregisters a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_peer_cb_rm(ytp_sequence_t *seq,
                                       ytp_sequence_peer_cb_t cb, void *closure,
                                       fmc_error_t **error);

/**
 * @brief Registers a channel data callback by channel name or prefix
 *
 * Subscribes to data on any channel matching the prefix if chprfx terminates
 * with '/' or exact channel name instead
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] sz
 * @param[in] prfx
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_prfx_cb(ytp_sequence_t *seq, size_t sz,
                                    const char *prfx, ytp_sequence_data_cb_t cb,
                                    void *closure, fmc_error_t **error);
/**
 * @brief Unregisters a channel data callback by channel name or prefix
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] sz
 * @param[in] prfx
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_prfx_cb_rm(ytp_sequence_t *seq, size_t sz,
                                       const char *prfx,
                                       ytp_sequence_data_cb_t cb, void *closure,
                                       fmc_error_t **error);
/**
 * @brief Registers a channel data callback by channel handler
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] channel
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_indx_cb(ytp_sequence_t *seq, ytp_channel_t channel,
                                    ytp_sequence_data_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Unregisters a channel data callback by channel handler
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] channel
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_indx_cb_rm(ytp_sequence_t *seq,
                                       ytp_channel_t channel,
                                       ytp_sequence_data_cb_t cb, void *closure,
                                       fmc_error_t **error);

/**
 * @brief Reads one message and executes the callbacks that applies.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[out] error out-parameter for error handling
 * @return true if a message was processed, false otherwise
 */
FMMODFUNC bool ytp_sequence_poll(ytp_sequence_t *seq, fmc_error_t **error);

/**
 * @brief Returns the iterator to the end of yamal
 *
 * Complexity: Constant.
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_sequence_end(ytp_sequence_t *seq,
                                          fmc_error_t **error);
/**
 * @brief Returns the current data iterator
 *
 * Complexity: Constant.
 *
 * @param[in] seq the ytp_sequence_t object
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_sequence_cur(ytp_sequence_t *seq);

/**
 * @brief Returns the current data iterator
 *
 * @param[in] seq the ytp_sequence_t object
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_sequence_get_it(ytp_sequence_t *seq);

/**
 * @brief Sets the current data iterator
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] iterator
 */
FMMODFUNC void ytp_sequence_set_it(ytp_sequence_t *seq,
                                   ytp_iterator_t iterator);

/**
 * @brief Removes all of the callbacks of the sequence
 *
 * @param sequence
 */
FMMODFUNC void ytp_sequence_cb_rm(ytp_sequence_t *seq);

/**
 * @brief Returns an iterator given a serializable offset
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] offset from the head of yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 *
 * Sets data offset to offset. All control messages from the
 * beginning up to the offset are guarantied do have been processed
 * and the callbacks to be called.
 */
FMMODFUNC ytp_iterator_t ytp_sequence_seek(ytp_sequence_t *seq,
                                           ytp_mmnode_offs offset,
                                           fmc_error_t **error);

/**
 * @brief Returns serializable offset given an iterator
 *
 * @param[in] seq the ytp_sequence_t object
 * @param[in] iterator
 * @param[out] error out-parameter for error handling
 * @return offset from the head of yamal
 */
FMMODFUNC ytp_mmnode_offs ytp_sequence_tell(ytp_sequence_t *seq,
                                            ytp_iterator_t iterator,
                                            fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_sequence_shared object with a
 * reference counter equal to one
 *
 * @param[in] filename a yamal file path
 * @param[out] error out-parameter for error handling
 * @return ytp_sequence_shared_t object
 */
FMMODFUNC ytp_sequence_shared_t *ytp_sequence_shared_new(const char *filename,
                                                         fmc_fmode mode,
                                                         fmc_error_t **error);

/**
 * @brief Increases the reference counter
 *
 * @param[in] shared_seq
 */
FMMODFUNC void ytp_sequence_shared_inc(ytp_sequence_shared_t *shared_seq);

/**
 * @brief Decreases the reference counter and call ytp_sequence_del the sequence
 * if the reference counter is zero
 *
 * @param[in] shared_seq
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_sequence_shared_dec(ytp_sequence_shared_t *shared_seq,
                                       fmc_error_t **error);

/**
 * @brief Returns the ytp_sequence_t object
 *
 * @param[in] shared_seq
 * @return ytp_sequence_t object
 */
FMMODFUNC ytp_sequence_t *
ytp_sequence_shared_get(ytp_sequence_shared_t *shared_seq);

#ifdef __cplusplus
}
#endif
