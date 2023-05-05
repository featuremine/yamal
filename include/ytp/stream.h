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
#include <ytp/control.h>
#include <ytp/yamal.h>

#define YTP_STREAM_LIST_DATA 0
#define YTP_STREAM_LIST_ANN 1
#define YTP_STREAM_LIST_SUB 2
#define YTP_STREAM_LIST_IDX 3

#ifdef __cplusplus
extern "C" {
#endif

struct ytp_inds {
  ytp_yamal_t *yamal;
  ytp_iterator_t it_idx;
};

struct ytp_subs {
  ytp_yamal_t *yamal;
  ytp_iterator_t it_subs;
};

typedef struct ytp_cursor ytp_cursor_t;
typedef struct ytp_anns ytp_anns_t;
typedef struct ytp_inds ytp_inds_t;
typedef struct ytp_subs ytp_subs_t;
typedef size_t ytp_stream_t;

typedef void (*ytp_cursor_ann_cb_t)(void *closure, ytp_stream_t stream,
                                    size_t seqno,
                                    size_t peer_sz, const char *peer_name,
                                    size_t ch_sz, const char *ch_name,
                                    size_t encoding_sz,
                                    const char *encoding_data);
typedef void (*ytp_cursor_data_cb_t)(void *closure, size_t seqno,
                                     uint64_t msgtime, ytp_stream_t stream,
                                     size_t sz, const char *data);

/**
 * @brief Allocates and initializes a ytp_cursor object
 *
 * @param[in] yamal
 * @param[out] error
 * @return ytp_cursor_t object
 */
FMMODFUNC ytp_cursor_t *ytp_cursor_new(ytp_yamal_t *yamal,
                                       fmc_error_t **error);

/**
 * @brief Initializes a ytp_cursor object
 *
 * @param[in] cursor
 * @param[in] yamal
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
 * @brief Registers a stream announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] cursor
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_ann_cb(ytp_cursor_t *cursor,
                                 ytp_cursor_ann_cb_t cb, void *closure,
                                 fmc_error_t **error);

/**
 * @brief Unregisters a stream announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] cursor
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_ann_cb_rm(ytp_cursor_t *cursor,
                                    ytp_cursor_ann_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Registers a stream data callback by stream handler
 *
 * Complexity: Linear with the number of callbacks on that stream.
 *
 * @param[in] cursor
 * @param[in] stream
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_data_cb(ytp_cursor_t *cursor,
                                  ytp_stream_t stream,
                                  ytp_cursor_data_cb_t cb, void *closure,
                                  fmc_error_t **error);

/**
 * @brief Unregisters a stream data callback by stream handler
 *
 * Complexity: Linear with the number of callbacks on that stream.
 *
 * @param[in] cursor
 * @param[in] stream
 * @param[in] cb
 * @param[in] closure
 * @param[out] error
 */
FMMODFUNC void ytp_cursor_data_cb_rm(ytp_cursor_t *cursor,
                                       ytp_stream_t stream,
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
 * @brief Returns the seqno of the last data message read.
 *
 * @param[in] cursor
 * @return the seqno of the last data message read
 */
FMMODFUNC uint64_t ytp_cursor_count(ytp_cursor_t *cursor);

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
 * @param[in] pz
 * @param[in] pn
 * @param[in] cz
 * @param[in] cn
 * @param[in] ez
 * @param[in] en
 * @param[out] error
 */
FMMODFUNC ytp_stream_t ytp_anns_stream(ytp_anns_t *cursor,
                                       size_t pz, const char *pn, size_t cz, const char *cn, size_t ez, const char *en,
                                       fmc_error_t **error);


/**
 * @brief Initializes a ytp_inds object
 *
 * @param[in] cursor
 * @param[out] error
 * @return ytp_inds_t object
 */
FMMODFUNC void ytp_inds_init(ytp_inds_t *cursor, ytp_yamal_t *yamal,
                             fmc_error_t **error);

/**
 * @brief Reads the next index
 *
 * Enforces protocol by setting stream announcement indscription flag is not set.
 * Skips duplicate indscription messages.
 * @param[in] cursor the ytp_inds_t object
 * @param[out] id of the stream
 * @param[out] error
 * @return true if advanced, false if at the end of the list
 */
FMMODFUNC bool ytp_inds_next(ytp_inds_t *cursor, ytp_stream_t *id,
                             uint64_t *offset, size_t *sz,
                             const char **data, fmc_error_t **error);

/**
 * @brief Initializes a ytp_subs object
 *
 * @param[in] cursor
 * @param[out] error
 * @return ytp_subs_t object
 */
FMMODFUNC void ytp_subs_init(ytp_subs_t *subs, ytp_yamal_t *yamal,
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
FMMODFUNC bool ytp_subs_next(ytp_subs_t *subs, ytp_stream_t *id, fmc_error_t **error);

FMMODFUNC char *ytp_stream_reserve(ytp_yamal_t *yamal, size_t size, fmc_error_t **error);

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
FMMODFUNC ytp_iterator_t ytp_stream_commit(ytp_yamal_t *yamal, uint64_t msgtime,
                                           ytp_stream_t id, void *data,
                                           fmc_error_t **error);

/**
 * @brief Returns the data corresponding to a given stream announcement.
 *
 */
FMMODFUNC void ytp_stream_ann(ytp_yamal_t *yamal, ytp_stream_t id, uint64_t *seqno, size_t *pz, const char **pn, size_t *cz, const char **cn, size_t *ez, const char **en, uint64_t *sub, fmc_error_t **error);

/**
 * @brief Writes an index
 *
 * Enforces protocol by setting stream announcement idxcription flag is not set.
 * Skips duplicate idxcription messages.
 * @param[in] cursor the ytp_idx_t object
 * @param[out] id of the stream
 * @param[out] error
 * @return true if advanced, false if at the end of the list
 */
FMMODFUNC void ytp_stream_idx(ytp_yamal_t *yamal, ytp_stream_t id,
                              uint64_t offset, size_t sz,
                              char *data, fmc_error_t **error);


#ifdef __cplusplus
}
#endif
