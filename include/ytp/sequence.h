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
#include <ytp/channel.h>
#include <ytp/peer.h>
#include <ytp/yamal.h>
#include <ytp/api.h>

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
 * @param[out] error
 * @return ytp_sequence_t object
 */
FMMODFUNC ytp_sequence_t *ytp_sequence_new(fmc_fd fd, fmc_error_t **error);

/**
 * @brief Initializes a ytp_sequence_t object
 *
 * @param[out] seq
 * @param[in] fd
 * @param[out] error
 */

FMMODFUNC void ytp_sequence_init(ytp_sequence_t *seq, fmc_fd fd,
                                 fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_sequence_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enable the preallocation and sync thread
 * @param[out] error
 * @return ytp_sequence_t object
 */
FMMODFUNC ytp_sequence_t *ytp_sequence_new_2(fmc_fd fd, bool enable_thread,
                                             fmc_error_t **error);

/**
 * @brief Initializes a ytp_sequence_t object
 *
 * @param[out] seq
 * @param[in] fd
 * @param[in] enable_thread enable the preallocation and sync thread
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_init_2(ytp_sequence_t *seq, fmc_fd fd,
                                   bool enable_thread, fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_sequence_t object
 *
 * @param[in] seq
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_del(ytp_sequence_t *seq, fmc_error_t **error);

/**
 * @brief Destroys a ytp_sequence_t object
 *
 * @param[in] seq
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_destroy(ytp_sequence_t *seq, fmc_error_t **error);

/**
 * @brief Reserves memory for data in the memory mapped list
 *
 * @param[in] seq
 * @param[in] sz the size of the data payload
 * @param[out] error
 * @return
 */
FMMODFUNC char *ytp_sequence_reserve(ytp_sequence_t *seq, size_t sz,
                                     fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list
 *
 * @param[in] seq
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] time
 * @param[in] data the value returned by ytp_sequence_reserve
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_sequence_commit(ytp_sequence_t *seq,
                                             ytp_peer_t peer,
                                             ytp_channel_t channel,
                                             uint64_t time, void *data,
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
 * @param[in] time
 * @param[in] sz
 * @param[in] payload
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_sub(ytp_sequence_t *seq, ytp_peer_t peer,
                                uint64_t time, size_t sz, const char *payload,
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
 * @param[in] time
 * @param[in] sz
 * @param[in] payload
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_dir(ytp_sequence_t *seq, ytp_peer_t peer,
                                uint64_t time, size_t sz, const char *payload,
                                fmc_error_t **error);

/**
 * @brief Returns the name of the channel, given the channel reference
 *
 * Complexity: Constant on average, worst case linear in the number of channels.
 *
 * @param[in] seq
 * @param[in] channel
 * @param[out] sz
 * @param[out] name
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_ch_name(ytp_sequence_t *seq, ytp_channel_t channel,
                                    size_t *sz, const char **name,
                                    fmc_error_t **error);

/**
 * @brief Declares an existing/new channel
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] seq
 * @param[in] peer the peer that publishes the channel announcement
 * @param[in] time
 * @param[in] sz
 * @param[in] name
 * @param[out] error
 * @return channel reference
 */
FMMODFUNC ytp_channel_t ytp_sequence_ch_decl(ytp_sequence_t *seq,
                                             ytp_peer_t peer, uint64_t time,
                                             size_t sz, const char *name,
                                             fmc_error_t **error);

/**
 * @brief Registers a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] seq
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_ch_cb(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb,
                                  void *closure, fmc_error_t **error);

/**
 * @brief Unregisters a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] seq
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_ch_cb_rm(ytp_sequence_t *seq,
                                     ytp_sequence_ch_cb_t cb, void *closure,
                                     fmc_error_t **error);

/**
 * @brief Returns the name of the peer, given the peer reference
 *
 * Complexity: Constant on average, worst case linear in the number of peers.
 *
 * @param[in] seq
 * @param[in] peer the peer that publishes the channel announcement
 * @param[out] sz
 * @param[out] name
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_peer_name(ytp_sequence_t *seq, ytp_peer_t peer,
                                      size_t *sz, const char **name,
                                      fmc_error_t **error);

/**
 * @brief Declares an existing/new peer
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] seq
 * @param[in] sz
 * @param[in] name
 * @param[out] error
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
 * @param[in] seq
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_peer_cb(ytp_sequence_t *seq,
                                    ytp_sequence_peer_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Unregisters a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] seq
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
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
 * @param[in] seq
 * @param[in] sz
 * @param[in] prfx
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_prfx_cb(ytp_sequence_t *seq, size_t sz,
                                    const char *prfx, ytp_sequence_data_cb_t cb,
                                    void *closure, fmc_error_t **error);
/**
 * @brief Unregisters a channel data callback by channel name or prefix
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[in] seq
 * @param[in] sz
 * @param[in] prfx
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
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
 * @param[in] seq
 * @param[in] channel
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_indx_cb(ytp_sequence_t *seq, ytp_channel_t channel,
                                    ytp_sequence_data_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Unregisters a channel data callback by channel handler
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[in] seq
 * @param[in] channel
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_sequence_indx_cb_rm(ytp_sequence_t *seq,
                                       ytp_channel_t channel,
                                       ytp_sequence_data_cb_t cb, void *closure,
                                       fmc_error_t **error);

/**
 * @brief Reads one message and executes the callbacks that applies.
 *
 * @param[in] seq
 * @param[out] error
 * @return true if a message was processed, false otherwise
 */
FMMODFUNC bool ytp_sequence_poll(ytp_sequence_t *seq, fmc_error_t **error);

/**
 * @brief Checks if there are not more messages
 *
 * Complexity: Constant.
 *
 * @param[in] seq
 * @return true if there are not more messages, false otherwise
 */
FMMODFUNC bool ytp_sequence_term(ytp_sequence_t *seq);

/**
 * @brief Returns the iterator to the end of yamal
 *
 * Complexity: Constant.
 *
 * @param[in] seq
 * @param[out] error
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_sequence_end(ytp_sequence_t *seq,
                                          fmc_error_t **error);
/**
 * @brief Returns the current data iterator
 *
 * Complexity: Constant.
 *
 * @param[in] seq
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_sequence_cur(ytp_sequence_t *seq);

/**
 * @brief Returns the current data iterator
 *
 * @param[in] seq
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_sequence_get_it(ytp_sequence_t *seq);

/**
 * @brief Sets the current data iterator
 *
 * @param[in] seq
 * @param[in] iterator
 */
FMMODFUNC void ytp_sequence_set_it(ytp_sequence_t *seq,
                                   ytp_iterator_t iterator);

/**
 * @brief Returns an iterator given a serializable offset
 *
 * @param[in] seq
 * @param[in] offset from the head of yamal
 * @param[out] error
 * @return ytp_iterator_t
 *
 * Sets data offset to offset. All control messages from the
 * beginning up to the offset are guarantied do have been processed
 * and the callbacks to be called.
 */
FMMODFUNC ytp_iterator_t ytp_sequence_seek(ytp_sequence_t *seq, size_t off,
                                           fmc_error_t **error);

/**
 * @brief Returns serializable offset given an iterator
 *
 * @param[in] seq
 * @param[in] iterator
 * @param[out] error
 * @return size_t offset from the head of yamal
 */
FMMODFUNC size_t ytp_sequence_tell(ytp_sequence_t *seq, ytp_iterator_t iterator,
                                   fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_sequence_shared object with a
 * reference counter equal to one
 *
 * @param[in] filename a yamal file path
 * @param[out] error
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
 * @param[out] error
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
