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
#include <ytp/stream.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ytp_cursor;
typedef struct ytp_cursor ytp_cursor_t;

typedef void (*ytp_cursor_announcement_cb_t)(
    void *closure, uint64_t msgtime, ytp_stream_t stream, size_t peername_sz,
    const char *peername, size_t chname_sz, const char *chname,
    size_t encoding_sz, const char *encoding);
typedef void (*ytp_cursor_data_cb_t)(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, ytp_stream_t stream,
                                     uint64_t msgtime, size_t sz,
                                     const char *data);
typedef void (*ytp_cursor_sub_cb_t)(void *closure, uint64_t msgtime,
                                    ytp_stream_t stream);

FMMODFUNC ytp_cursor_t *ytp_cursor_new(ytp_stream_t *ctrl, fmc_error_t **error);

FMMODFUNC void ytp_cursor_init(ytp_cursor_t *timeline, ytp_stream_t *ctrl,
                               fmc_error_t **error);

FMMODFUNC void ytp_cursor_del(ytp_cursor_t *timeline, fmc_error_t **error);

FMMODFUNC void ytp_cursor_destroy(ytp_cursor_t *timeline, fmc_error_t **error);

FMMODFUNC void ytp_cursor_sub_cb(ytp_cursor_t *timeline, ytp_cursor_sub_cb_t cb,
                                 void *closure, fmc_error_t **error);

FMMODFUNC void ytp_cursor_sub_cb_rm(ytp_cursor_t *timeline,
                                    ytp_cursor_sub_cb_t cb, void *closure,
                                    fmc_error_t **error);

FMMODFUNC void ytp_cursor_announcement_cb(ytp_cursor_t *timeline,
                                          ytp_cursor_announcement_cb_t cb,
                                          void *closure, fmc_error_t **error);

FMMODFUNC void ytp_cursor_announcement_cb_rm(ytp_cursor_t *timeline,
                                             ytp_cursor_announcement_cb_t cb,
                                             void *closure,
                                             fmc_error_t **error);

FMMODFUNC void ytp_cursor_data_cb(ytp_cursor_t *timeline, uint64_t time,
                                  ytp_stream_t stream, ytp_cursor_data_cb_t cb,
                                  void *closure, fmc_error_t **error);

FMMODFUNC void ytp_cursor_data_cb_rm(ytp_cursor_t *timeline, ytp_peer_t peer,
                                     ytp_channel_t channel,
                                     ytp_cursor_data_cb_t cb, void *closure,
                                     fmc_error_t **error);

FMMODFUNC bool ytp_cursor_term(ytp_cursor_t *timeline);

FMMODFUNC ytp_iterator_t ytp_cursor_iter_get(ytp_cursor_t *timeline);

FMMODFUNC void ytp_cursor_iter_set(ytp_cursor_t *timeline,
                                   ytp_iterator_t iterator);

FMMODFUNC bool ytp_cursor_poll(ytp_cursor_t *timeline, fmc_error_t **error);

FMMODFUNC bool ytp_cursor_consume(ytp_cursor_t *dest, ytp_cursor_t *src);

FMMODFUNC void ytp_cursor_cb_rm(ytp_cursor_t *timeline);

FMMODFUNC ytp_iterator_t ytp_cursor_seek(ytp_cursor_t *timeline, size_t off,
                                         fmc_error_t **error);

FMMODFUNC size_t ytp_cursor_tell(ytp_cursor_t *timeline,
                                 ytp_iterator_t iterator, fmc_error_t **error);

#ifdef __cplusplus
}
#endif
