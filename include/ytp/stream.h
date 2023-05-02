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
 * @file cursor.h
 * @date 04 Jan 2022
 * @brief File contains C declaration of cursor API of YTP
 *
 * This file contains C declaration of cursor API of YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/channel.h>
#include <ytp/control.h>
#include <ytp/peer.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interface for reading and writing to ytp
 *
 * The ytp_cursor interface provides the ability to read and write to ytp
 * using callbacks for reading, on top of the control layer of ytp
 *
 * Internally, it uses the ytp_control API.
 */
struct ytp_cursor;
typedef struct ytp_cursor ytp_cursor_t;

typedef void (*ytp_cursor_peer_cb_t)(void *closure, ytp_peer_t peer,
                                       size_t sz, const char *name);
typedef void (*ytp_cursor_ch_cb_t)(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, uint64_t time,
                                     size_t sz, const char *name);
typedef void (*ytp_cursor_data_cb_t)(void *closure, ytp_peer_t peer,
                                       ytp_channel_t channel, uint64_t time,
                                       size_t sz, const char *data);
typedef void (*ytp_cursor_idle_cb_t)(void *closure);

/**
 * @brief Allocates and initializes a ytp_cursor object
 *
 * @param[in] cursor
 * @param[out] error
 * @return ytp_cursor_t object
 */
FMMODFUNC ytp_cursor_t *ytp_cursor_new(ytp_yamal_t *yamal,
                                       fmc_error_t **error);

/**
 * @brief Initializes a ytp_cursor object
 *
 * @param[in] cursor
 * @param[out] error
 * @return ytp_cursor_t object
 */
FMMODFUNC void ytp_cursor_init(ytp_cursor_t *cursor, ytp_yamal_t *yamal,
                                 fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_cursor_t object
 *
 * @param[in] cursor
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_del(ytp_cursor_t *cursor, fmc_error_t **error);

/**
 * @brief Destroys a ytp_cursor_t object
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_destroy(ytp_cursor_t *cursor,
                                    fmc_error_t **error);

/**
 * @brief Registers a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] cursor
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_ann_cb(ytp_cursor_t *cursor,
                                  ytp_cursor_ch_cb_t cb, void *closure,
                                  fmc_error_t **error);

/**
 * @brief Unregisters a channel announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] cursor
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_ann_cb_rm(ytp_cursor_t *cursor,
                                     ytp_cursor_ch_cb_t cb, void *closure,
                                     fmc_error_t **error);

/**
 * @brief Registers a channel data callback by channel handler
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[in] cursor
 * @param[in] channel
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_data_cb(ytp_cursor_t *cursor,
                                  ytp_stream_t channel,
                                  ytp_cursor_data_cb_t cb, void *closure,
                                  fmc_error_t **error);

/**
 * @brief Unregisters a channel data callback by channel handler
 *
 * Complexity: Linear with the number of callbacks on that channel.
 *
 * @param[in] cursor
 * @param[in] channel
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_data_cb_rm(ytp_cursor_t *cursor,
                                       ytp_channel_t channel,
                                       ytp_cursor_data_cb_t cb, void *closure,
                                       fmc_error_t **error);

/**
 * @brief Checks if there are not more messages
 *
 * Complexity: Constant.
 *
 * @param[in] cursor
 * @return true if there are not more messages, false otherwise
 */
FMMODFUNC bool ytp_cursor_term(ytp_cursor_t *cursor);

/**
 * @brief Returns number of data messages read.
 *
 * @param[in] cursor
 * @param[out] error
 * @return true if a message was processed, false otherwise
 */
FMMODFUNC uint64_t ytp_cursor_count(ytp_cursor_t *cursor, fmc_error_t **error);

/**
 * @brief Reads one message and executes the callbacks that applies.
 *
 * @param[in] cursor
 * @param[out] error
 * @return true if a message was processed, false otherwise
 */
FMMODFUNC bool ytp_cursor_poll(ytp_cursor_t *cursor, fmc_error_t **error);

/**
 * @brief Moves all of the callbacks of the source cursor into destination if
 * both cursors have the same iterator
 *
 * @param dest
 * @param src
 * @return true if both cursors have the same iterator and all the callbacks
 * were moved, false otherwise.
 */
FMMODFUNC bool ytp_cursor_consume(ytp_cursor_t *dest, ytp_cursor_t *src);

/**
 * @brief Removes all of the callbacks of the cursor
 *
 * @param cursor
 */
FMMODFUNC void ytp_cursor_all_cb_rm(ytp_cursor_t *cursor);

/**
 * @brief Returns an iterator given a serializable offset
 *
 * Moves data pointer to the specified offset.
 * @param[in] cursor
 * @param[in] off
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_seek(ytp_cursor_t *cursor, size_t off,
                               fmc_error_t **error);

/**
 * @brief Returns serializable offset of the current data iterator
 *
 * @param[in] cursor
 * @param[in] iterator
 * @param[out] error
 * @return serializable
 */
FMMODFUNC size_t ytp_cursor_tell(ytp_cursor_t *cursor,
                                 fmc_error_t **error);


/**
 * @brief Allocates and initializes a ytp_subs object
 *
 * yamal needs to be writable since subscription object is used by publishers only
 * @param[in] cursor
 * @param[out] error
 * @return ytp_subs_t object
 */
FMMODFUNC ytp_subs_t *ytp_subs_new(ytp_yamal_t *yamal,
                                       fmc_error_t **error);

/**
 * @brief Initializes a ytp_subs object
 *
 * @param[in] cursor
 * @param[out] error
 * @return ytp_subs_t object
 */
FMMODFUNC void ytp_subs_init(ytp_subs_t *cursor, ytp_yamal_t *yamal,
                                 fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_subs_t object
 *
 * @param[in] cursor
 * @param[out] error
 */
FMMODFUNC void ytp_subs_del(ytp_subs_t *cursor, fmc_error_t **error);

/**
 * @brief Destroys a ytp_subs_t object
 *
 * @param[in] cursor the ytp_subs_t object
 * @param[out] error
 */
FMMODFUNC void ytp_subs_destroy(ytp_subs_t *cursor,
                                fmc_error_t **error);

/**
 * @brief Advances the iterator to the next subscription message
 *
 * Enforces protocol by setting stream announcement subscription flag is not set.
 * Skips duplicate subscription messages.
 * @param[in] cursor the ytp_subs_t object
 * @param[out] id of the stream
 * @param[out] error
 * @return true if advanced, false if at the end of the list
 */
FMMODFUNC bool ytp_subs_next(ytp_subs_t *cursor, ytp_stream_t *id, fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_anns object
 *
 * yamal needs to be writable since announcementscription object is used by publishers only
 * @param[in] cursor
 * @param[out] error
 * @return ytp_anns_t object
 */
FMMODFUNC ytp_anns_t *ytp_anns_new(ytp_yamal_t *yamal,
                                   fmc_error_t **error);

/**
 * @brief Initializes a ytp_anns object
 *
 * @param[in] cursor
 * @param[out] error
 * @return ytp_anns_t object
 */
FMMODFUNC void ytp_anns_init(ytp_anns_t *cursor, ytp_yamal_t *yamal,
                                 fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_anns_t object
 *
 * @param[in] cursor
 * @param[out] error
 */
FMMODFUNC void ytp_anns_del(ytp_anns_t *cursor, fmc_error_t **error);

/**
 * @brief Destroys a ytp_anns_t object
 *
 * @param[in] cursor the ytp_anns_t object
 * @param[out] error
 */
FMMODFUNC void ytp_anns_destroy(ytp_anns_t *cursor,
                                fmc_error_t **error);

/**
 * @brief Returns stream id for with the given peer, channel and encoding.
 *
 * Enforces stream sequence number for all of the announcements.
 * @param[in] cursor the ytp_anns_t object
 * @param[out] error
 */
FMMODFUNC ytp_stream_t ytp_anns_stream(ytp_anns_t *cursor,
                                       size_t pz, char* pn, size_t cz, char* cn,
                                       size_t ez, char* en, fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list using stream level protocol
 *
 * @param[in] yamal
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] time
 * @param[in] data the value returned by ytp_sequence_reserve
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_stream_commit(ytp_yamal_t *yamal, uint64_t time,
                                           ytp_stream_t id, void *data,
                                           fmc_error_t **error);


/**
 * @brief Returns the data corresponding to a given stream announcement.
 *
 */
FMMODFUNC void ytp_stream_ann(ytp_yamal_t *yamal, ytp_stream_t id,
                              size_t pz, char* pn, size_t cz, char* cn,
                              size_t ez, char* en, uint64_t *seq, uint64_t *sub,
                              fmc_error_t **error);
#ifdef __cplusplus
}
#endif
