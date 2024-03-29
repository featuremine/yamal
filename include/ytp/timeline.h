/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file timeline.h
 * @date 04 Jan 2022
 * @brief File contains C declaration of timeline API of YTP
 *
 * This file contains C declaration of timeline API of YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/control.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interface for reading and writing to ytp
 *
 * The ytp_timeline interface provides the ability to read and write to ytp
 * using callbacks for reading, on top of the control layer of ytp
 *
 * Internally, it uses the ytp_control API.
 */
struct ytp_timeline;
typedef struct ytp_timeline ytp_timeline_t;

typedef void (*ytp_timeline_peer_cb_t)(void *closure, ytp_peer_t peer,
                                       size_t sz, const char *name);
typedef void (*ytp_timeline_ch_cb_t)(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, uint64_t ts,
                                     size_t sz, const char *name);
typedef void (*ytp_timeline_data_cb_t)(void *closure, ytp_peer_t peer,
                                       ytp_channel_t channel, uint64_t ts,
                                       size_t sz, const char *data);
typedef void (*ytp_timeline_idle_cb_t)(void *closure);

/**
 * @brief Allocates and initializes a ytp_timeline object
 *
 * @param[in] timeline
 * @param[out] error out-parameter for error handling
 * @return ytp_timeline_t object
 */
FMMODFUNC ytp_timeline_t *ytp_timeline_new(ytp_control_t *ctrl,
                                           fmc_error_t **error);

/**
 * @brief Initializes a ytp_timeline object
 *
 * @param[in] timeline
 * @param[out] error out-parameter for error handling
 * @return ytp_timeline_t object
 */
FMMODFUNC void ytp_timeline_init(ytp_timeline_t *timeline, ytp_control_t *ctrl,
                                 fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_timeline_t object
 *
 * @param[in] timeline
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_del(ytp_timeline_t *timeline, fmc_error_t **error);

/**
 * @brief Destroys a ytp_timeline_t object
 *
 * @param[in] timeline the ytp_timeline_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_destroy(ytp_timeline_t *timeline,
                                    fmc_error_t **error);

/**
 * @brief Registers a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] timeline
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_ch_cb(ytp_timeline_t *timeline,
                                  ytp_timeline_ch_cb_t cb, void *closure,
                                  fmc_error_t **error);

/**
 * @brief Unregisters a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] timeline
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_ch_cb_rm(ytp_timeline_t *timeline,
                                     ytp_timeline_ch_cb_t cb, void *closure,
                                     fmc_error_t **error);

/**
 * @brief Registers a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] timeline
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_peer_cb(ytp_timeline_t *timeline,
                                    ytp_timeline_peer_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Unregisters a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] timeline
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_peer_cb_rm(ytp_timeline_t *timeline,
                                       ytp_timeline_peer_cb_t cb, void *closure,
                                       fmc_error_t **error);

/**
 * @brief Registers a channel data callback by channel name or prefix
 *
 * Subscribes to data on any channel matching the prefix if chprfx terminates
 * with '/' or exact channel name instead
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[in] timeline
 * @param[in] sz
 * @param[in] prfx
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_prfx_cb(ytp_timeline_t *timeline, size_t sz,
                                    const char *prfx, ytp_timeline_data_cb_t cb,
                                    void *closure, fmc_error_t **error);
/**
 * @brief Unregisters a channel data callback by channel name or prefix
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[in] timeline
 * @param[in] sz
 * @param[in] prfx
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_prfx_cb_rm(ytp_timeline_t *timeline, size_t sz,
                                       const char *prfx,
                                       ytp_timeline_data_cb_t cb, void *closure,
                                       fmc_error_t **error);
/**
 * @brief Registers a channel data callback by channel handler
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[in] timeline
 * @param[in] channel
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_indx_cb(ytp_timeline_t *timeline,
                                    ytp_channel_t channel,
                                    ytp_timeline_data_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Unregisters a channel data callback by channel handler
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[in] timeline
 * @param[in] channel
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_indx_cb_rm(ytp_timeline_t *timeline,
                                       ytp_channel_t channel,
                                       ytp_timeline_data_cb_t cb, void *closure,
                                       fmc_error_t **error);

/**
 * @brief Returns the current data iterator
 *
 * @param[in] timeline
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_timeline_iter_get(ytp_timeline_t *timeline);

/**
 * @brief Sets the current data iterator
 *
 * @param[in] timeline
 * @param[in] iterator
 */
FMMODFUNC void ytp_timeline_iter_set(ytp_timeline_t *timeline,
                                     ytp_iterator_t iterator);

/**
 * @brief Reads one message and executes the callbacks that applies.
 *
 * @param[in] timeline
 * @param[out] error out-parameter for error handling
 * @return true if a message was processed, false otherwise
 */
FMMODFUNC bool ytp_timeline_poll(ytp_timeline_t *timeline, fmc_error_t **error);

/**
 * @brief Reads one message and executes the callbacks that applies if timeline
 * is behind src_timeline
 *
 * @param[in] timeline
 * @param[in] src_timeline
 * @param[out] error out-parameter for error handling
 * @return true if a message was processed, false otherwise
 */
FMMODFUNC bool ytp_timeline_poll_until(ytp_timeline_t *timeline,
                                       const ytp_timeline_t *src_timeline,
                                       fmc_error_t **error);

/**
 * @brief Moves all of the callbacks of the source timeline into destination if
 * both timelines have the same iterator
 *
 * @param dest
 * @param src
 * @return true if both timelines have the same iterator and all the callbacks
 * were moved, false otherwise.
 */
FMMODFUNC bool ytp_timeline_consume(ytp_timeline_t *dest, ytp_timeline_t *src);

/**
 * @brief Removes all of the callbacks of the timeline
 *
 * @param timeline
 */
FMMODFUNC void ytp_timeline_cb_rm(ytp_timeline_t *timeline);

/**
 * @brief Returns an iterator given a serializable offset
 *
 * Moves control pointer to catch up with iterator.
 * @param[in] timeline
 * @param[in] off
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_timeline_seek(ytp_timeline_t *timeline,
                                           ytp_mmnode_offs off,
                                           fmc_error_t **error);

/**
 * @brief Returns serializable offset given an iterator
 *
 * @param[in] timeline
 * @param[in] iterator
 * @param[out] error out-parameter for error handling
 * @return serializable
 */
FMMODFUNC ytp_mmnode_offs ytp_timeline_tell(ytp_timeline_t *timeline,
                                            ytp_iterator_t iterator,
                                            fmc_error_t **error);

/**
 * @brief Registers an idle callback
 *
 * @param[in] timeline
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_idle_cb(ytp_timeline_t *timeline,
                                    ytp_timeline_idle_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Unregisters an idle callback
 *
 * @param[in] timeline
 * @param[in] cb
 * @param[in] closure
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_timeline_idle_cb_rm(ytp_timeline_t *timeline,
                                       ytp_timeline_idle_cb_t cb, void *closure,
                                       fmc_error_t **error);

#ifdef __cplusplus
}
#endif
