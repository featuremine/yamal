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
 * @date 6 Jun 2023
 * @brief File contains C declaration of data API
 *
 * This file contains C declaration of data API of stream protocol level of YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <ytp/yamal.h>

#include <fmc/platform.h>

#include <stddef.h>
#include <stdint.h>

typedef struct ytp_cursor ytp_cursor_t;

typedef void (*ytp_cursor_ann_cb_t)(void *closure, uint64_t seqno,
                                    ytp_mmnode_offs stream, size_t peer_sz,
                                    const char *peer_name, size_t ch_sz,
                                    const char *ch_name, size_t encoding_sz,
                                    const char *encoding_data);
typedef void (*ytp_cursor_data_cb_t)(void *closure, uint64_t seqno, int64_t ts,
                                     ytp_mmnode_offs stream, size_t sz,
                                     const char *data);

struct ytp_cursor_ann_cb_cl_t {
  ytp_cursor_ann_cb_t cb;
  void *cl;
};

struct ytp_cursor_data_cb_cl_t {
  ytp_cursor_data_cb_t cb;
  void *cl;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocates and initializes a ytp_cursor object
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[out] error out-parameter for error handling
 * @return ytp_cursor_t object
 */
FMMODFUNC ytp_cursor_t *ytp_cursor_new(ytp_yamal_t *yamal, fmc_error_t **error);

/**
 * @brief Initializes a ytp_cursor object
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[in] yamal the ytp_yamal_t object
 * @param[out] error out-parameter for error handling
 * @return ytp_cursor_t object
 */
FMMODFUNC void ytp_cursor_init(ytp_cursor_t *cursor, ytp_yamal_t *yamal,
                               fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_cursor_t object
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_cursor_del(ytp_cursor_t *cursor, fmc_error_t **error);

/**
 * @brief Destroys a ytp_cursor_t object
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_cursor_destroy(ytp_cursor_t *cursor, fmc_error_t **error);

/**
 * @brief Registers a stream announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[in] cb the callback pointer
 * @param[in] closure the closure pointer
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_cursor_ann_cb(ytp_cursor_t *cursor, ytp_cursor_ann_cb_t cb,
                                 void *closure, fmc_error_t **error);

/**
 * @brief Unregisters a stream announcement callback
 *
 * Complexity: Linear with the number of registered callbacks.
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[in] cb the callback pointer
 * @param[in] closure the closure pointer
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_cursor_ann_cb_rm(ytp_cursor_t *cursor,
                                    ytp_cursor_ann_cb_t cb, void *closure,
                                    fmc_error_t **error);

/**
 * @brief Registers a stream data callback by stream handler
 *
 * Complexity: Linear with the number of callbacks on that stream.
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[in] stream the stream id
 * @param[in] cb the callback pointer
 * @param[in] closure the closure pointer
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_cursor_data_cb(ytp_cursor_t *cursor, ytp_mmnode_offs stream,
                                  ytp_cursor_data_cb_t cb, void *closure,
                                  fmc_error_t **error);

/**
 * @brief Unregisters a stream data callback by stream handler
 *
 * Complexity: Linear with the number of callbacks on that stream.
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[in] stream the stream id
 * @param[in] cb the callback pointer
 * @param[in] closure the closure pointer
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_cursor_data_cb_rm(ytp_cursor_t *cursor,
                                     ytp_mmnode_offs stream,
                                     ytp_cursor_data_cb_t cb, void *closure,
                                     fmc_error_t **error);

/**
 * @brief Checks if there are not more messages
 *
 * Complexity: Constant.
 *
 * @param[in] cursor the ytp_cursor_t object
 * @return true if there are not more messages, false otherwise
 */
FMMODFUNC bool ytp_cursor_term(ytp_cursor_t *cursor, fmc_error_t **error);

/**
 * @brief Reads one message and executes the callbacks that applies.
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[out] error out-parameter for error handling
 * @return true if a message was processed, false otherwise
 */
FMMODFUNC bool ytp_cursor_poll(ytp_cursor_t *cursor, fmc_error_t **error);

/**
 * @brief Moves all of the callbacks of the source cursor into destination if
 * both cursors have the same iterator
 *
 * @param dest the destination ytp_cursor_t
 * @param src the source ytp_cursor_t
 * @return true if both cursors have the same iterator and all the callbacks
 * were moved, false otherwise.
 */
FMMODFUNC bool ytp_cursor_consume(ytp_cursor_t *dest, ytp_cursor_t *src);

/**
 * @brief Removes all of the callbacks of the cursor
 *
 * @param[in] cursor the ytp_cursor_t object
 */
FMMODFUNC void ytp_cursor_all_cb_rm(ytp_cursor_t *cursor);

/**
 * @brief Returns an iterator given a serializable offset
 *
 * Moves data pointer to the specified offset.
 * @param[in] cursor the ytp_cursor_t object
 * @param[in] offset from the head of yamal
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_cursor_seek(ytp_cursor_t *cursor, ytp_mmnode_offs offset,
                               fmc_error_t **error);

/**
 * @brief Returns serializable offset of the current data iterator
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[in] iterator
 * @param[out] error out-parameter for error handling
 * @return serializable
 */
FMMODFUNC ytp_mmnode_offs ytp_cursor_tell(ytp_cursor_t *cursor,
                                          fmc_error_t **error);

#ifdef __cplusplus
}
#endif
