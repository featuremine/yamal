/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file glob.h
 * @date 15 Aug 2023
 * @brief File contains C declaration of glob API
 *
 * This file contains C declaration of glob API of stream protocol level of YTP.
 * @see http://www.featuremine.com
 */

#pragma once

#include <ytp/cursor.h>
#include <ytp/yamal.h>

#include <fmc/platform.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ytp_glob ytp_glob_t;

/**
 * @brief Allocates and initializes a ytp_glob object
 *
 * @param[in] cursor the ytp_cursor_t object
 * @param[out] error out-parameter for error handling
 * @return ytp_glob_t object
 */
FMMODFUNC ytp_glob_t *ytp_glob_new(ytp_cursor_t *cursor, fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_glob_t object
 *
 * @param[in] glob the ytp_glob_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_glob_del(ytp_glob_t *glob, fmc_error_t **error);

/**
 * @brief Registers a stream data callback by channel name prefix
 *
 * @param[in] glob the ytp_glob_t object
 * @param[in] sz
 * @param[in] prfx
 * @param[in] cb the callback pointer
 * @param[in] closure the closure pointer
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_glob_prefix_cb(ytp_glob_t *glob, size_t sz, const char *prfx,
                                  ytp_cursor_data_cb_t cb, void *closure,
                                  fmc_error_t **error);

/**
 * @brief Unregisters a stream data callback by channel name prefix
 *
 * @param[in] glob the ytp_glob_t object
 * @param[in] sz
 * @param[in] prfx
 * @param[in] cb the callback pointer
 * @param[in] closure the closure pointer
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_glob_prefix_cb_rm(ytp_glob_t *glob, size_t sz,
                                     const char *prfx, ytp_cursor_data_cb_t cb,
                                     void *closure, fmc_error_t **error);

/**
 * @brief Moves all of the callbacks of the source glob into destination
 *
 * @param[in] dest the destination ytp_glob_t
 * @param[in] src the source ytp_glob_t
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_glob_consume(ytp_glob_t *dest, ytp_glob_t *src,
                                fmc_error_t **error);

#ifdef __cplusplus
}
#endif
