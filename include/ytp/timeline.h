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
 * @file timeline.h
 * @author Featuremine Corporation
 * @date 04 Jan 2022
 * @brief File contains C declaration of timeline API of YTP
 *
 * This file contains C declaration of timeline API of YTP.
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_TIMELINE_H__
#define __FM_YTP_TIMELINE_H__

#include <stdbool.h> 
#include <stddef.h>
#include <stdint.h>

#include <apr.h> // apr_size_t APR_DECLARE
#include <ytp/channel.h> // ytp_channel_t
#include <ytp/control.h> // ytp_control_t
#include <ytp/peer.h> // ytp_peer_t
#include <ytp/errno.h> // ytp_status_t

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
                                       apr_size_t sz, const char *name);
typedef void (*ytp_timeline_ch_cb_t)(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, uint64_t time,
                                     apr_size_t sz, const char *name);
typedef void (*ytp_timeline_data_cb_t)(void *closure, ytp_peer_t peer,
                                               ytp_channel_t channel, uint64_t time,
                                               apr_size_t sz, const char *data);
typedef void (*ytp_timeline_idle_cb_t)(void *closure);

/**
 * @brief Allocates and initializes a ytp_timeline object
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] ctrl ytp_control_t object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_timeline_new(ytp_timeline_t **timeline, ytp_control_t *ctrl);

/**
 * @brief Initializes a ytp_timeline object
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] ctrl ytp_control_t object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_timeline_init(ytp_timeline_t *timeline, ytp_control_t *ctrl);

/**
 * @brief Destroys and deallocate a ytp_timeline_t object
 *
 * @param[out] timeline ytp_timeline_t object
 */
APR_DECLARE(void) ytp_timeline_del(ytp_timeline_t *timeline);

/**
 * @brief Destroys a ytp_timeline_t object
 *
 * @param[out] timeline ytp_timeline_t object
 */
APR_DECLARE(void) ytp_timeline_destroy(ytp_timeline_t *timeline);

/**
 * @brief Registers a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] cb channel announcement callback
 * @param[in] closure closure data for the channel announcement callback
 */
APR_DECLARE(void) ytp_timeline_ch_cb(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb, void *closure);

/**
 * @brief Unregisters a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] cb channel announcement callback
 * @param[in] closure closure data for the channel announcement callback
 */
APR_DECLARE(void) ytp_timeline_ch_cb_rm(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb, void *closure);

/**
 * @brief Registers a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] cb peer announcement callback
 * @param[in] closure closure data for the peer announcement callback
 */
APR_DECLARE(void) ytp_timeline_peer_cb(ytp_timeline_t *timeline, ytp_timeline_peer_cb_t cb, void *closure);

/**
 * @brief Unregisters a peer announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] cb peer announcement callback
 * @param[in] closure closure data for the peer announcement callback
 */
APR_DECLARE(void) ytp_timeline_peer_cb_rm(ytp_timeline_t *timeline, ytp_timeline_peer_cb_t cb, void *closure);

/**
 * @brief Registers a channel data callback by channel name or prefix
 *
 * Subscribes to data on any channel matching the prefix if chprfx terminates
 * with '/' or exact channel name instead
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] sz size of the channel prefix
 * @param[in] prfx channel prefix
 * @param[in] cb channel data callback
 * @param[in] closure closure data for the channel data callback
 */
APR_DECLARE(void) ytp_timeline_prfx_cb(ytp_timeline_t *timeline, apr_size_t sz, const char *prfx, ytp_timeline_data_cb_t cb, void *closure);

/**
 * @brief Unregisters a channel data callback by channel name or prefix
 *
 * Complexity: Linear with the number of channels.
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] sz size of the channel prefix
 * @param[in] prfx channel prefix
 * @param[in] cb channel data callback
 * @param[in] closure closure data for the channel data callback
 */
APR_DECLARE(void) ytp_timeline_prfx_cb_rm(ytp_timeline_t *timeline, apr_size_t sz, const char *prfx, ytp_timeline_data_cb_t cb, void *closure);

/**
 * @brief Registers a channel data callback by channel reference
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] channel channel reference
 * @param[in] cb channel data callback
 * @param[in] closure closure data for the channel data callback
 */
APR_DECLARE(void) ytp_timeline_indx_cb(ytp_timeline_t *timeline, ytp_channel_t channel, ytp_timeline_data_cb_t cb, void *closure);

/**
 * @brief Unregisters a channel data callback by channel reference
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] channel channel reference
 * @param[in] cb channel data callback
 * @param[in] closure closure data for the channel data callback
 */
APR_DECLARE(void) ytp_timeline_indx_cb_rm(ytp_timeline_t *timeline, ytp_channel_t channel, ytp_timeline_data_cb_t cb, void *closure);

/**
 * @brief Checks if there are not more messages
 *
 * Complexity: Constant.
 *
 * @param[in] timeline ytp_timeline_t object
 * @return true if there are not more messages, false otherwise
 */
APR_DECLARE(bool) ytp_timeline_term(ytp_timeline_t *timeline);

/**
 * @brief Returns the current data iterator
 *
 * @param[in] timeline ytp_timeline_t object
 * @return the current data iterator
 */
APR_DECLARE(ytp_iterator_t) ytp_timeline_iter_get(ytp_timeline_t *timeline);

/**
 * @brief Sets the current data iterator
 *
 * @param[out] timeline ytp_timeline_t object
 * @param[in] iterator iterator to be set
 */
APR_DECLARE(void) ytp_timeline_iter_set(ytp_timeline_t *timeline, ytp_iterator_t iterator);

/**
 * @brief Reads one message and executes the callbacks that applies.
 *
 * @param[in] timeline ytp_timeline_t object
 * @param[out] new_data true if a message was processed, false otherwise
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_timeline_poll(ytp_timeline_t *timeline, bool *new_data);

/**
 * @brief Moves all of the callbacks of the source timeline into destination if
 * both timelines have the same iterator
 *
 * @param[out] dest ytp_timeline_t object to move the callbacks to
 * @param[in] src ytp_timeline_t object to move the callbacks from
 * @return true if both timelines have the same iterator and all the callbacks
 * were moved, false otherwise.
 */
APR_DECLARE(bool) ytp_timeline_consume(ytp_timeline_t *dest, ytp_timeline_t *src);

/**
 * @brief Removes all of the callbacks of the timeline
 *
 * @param[out] timeline tp_timeline_t object 
 */
APR_DECLARE(void) ytp_timeline_cb_rm(ytp_timeline_t *timeline);

/**
 * @brief Obtains an iterator given a serializable ptr offset.
 * Also moves control pointer to catch up with iterator.
 *
 * @param[in] timeline ytp_timeline_t object 
 * @param[out] it_ptr the iterator of the serializable ptr
 * @param[in] ptr the serializable ptr offset
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_timeline_seek(ytp_timeline_t *timeline, ytp_iterator_t *it_ptr, apr_size_t ptr);

/**
 * @brief Obtains the serializable offset of a given iterator
 *
 * @param[in] timeline ytp_timeline_t object 
 * @param[out] ptr the serializable ptr offset
 * @param[in] iterator the iterator of the serializable ptr
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_timeline_tell(ytp_timeline_t *timeline, apr_size_t *ptr, ytp_iterator_t iterator);

/**
 * @brief Registers an idle callback
 *
 * @param[out] timeline ytp_timeline_t object 
 * @param[in] cb idle callback
 * @param[in] closure closure data for the idle callback
 */
APR_DECLARE(void) ytp_timeline_idle_cb(ytp_timeline_t *timeline, ytp_timeline_idle_cb_t cb, void *closure);

/**
 * @brief Unregisters an idle callback
 *
 * @param[out] timeline ytp_timeline_t object 
 * @param[in] cb idle callback
 * @param[in] closure closure data for the idle callback
 */
APR_DECLARE(void) ytp_timeline_idle_cb_rm(ytp_timeline_t *timeline, ytp_timeline_idle_cb_t cb, void *closure);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_TIMELINE_H__
