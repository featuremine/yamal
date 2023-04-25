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

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/time.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t ytp_stream_t;

#define YTP_STREAM_ANN 0
#define YTP_STREAM_SUB 1

#define YTP_STREAM_OFF 0x100

#define YTP_PEERNAME_MAXLEN 65535
#define YTP_CHANNELNAME_MAXLEN 65535

FMMODFUNC ytp_stream_t *ytp_stream_new(fmc_fd fd, fmc_error_t **error);

FMMODFUNC void ytp_stream_init(ytp_stream_t *ctrl, fmc_fd fd,
                               fmc_error_t **error);

FMMODFUNC void ytp_stream_del(ytp_stream_t *ctrl, fmc_error_t **error);

FMMODFUNC void ytp_stream_destroy(ytp_stream_t *ctrl, fmc_error_t **error);

FMMODFUNC char *ytp_stream_reserve(ytp_stream_t *ctrl, size_t sz,
                                   fmc_error_t **error);

FMMODFUNC ytp_iterator_t ytp_stream_commit(ytp_stream_t *ctrl, uint64_t msgtime,
                                           ytp_stream_t stream, void *data,
                                           fmc_error_t **error);

FMMODFUNC void ytp_stream_sub(ytp_stream_t *ctrl, uint64_t msgtime,
                              ytp_stream_t stream, fmc_error_t **error);

FMMODFUNC ytp_stream_t ytp_stream_stream_decl(
    ytp_stream_t *ctrl, uint64_t time, size_t peername_sz, const char *peername,
    size_t chname_sz, const char *chname, size_t encoding_sz,
    const char *encoding, fmc_error_t **error);

FMMODFUNC ytp_iterator_t ytp_stream_next(ytp_stream_t *ctrl,
                                         ytp_iterator_t iter,
                                         fmc_error_t **error);

FMMODFUNC void ytp_stream_read(ytp_stream_t *ctrl, ytp_iterator_t iter,
                               uint64_t *time, ytp_stream_t *stream, size_t *sz,
                               const char **data, fmc_error_t **error);

FMMODFUNC ytp_iterator_t ytp_stream_begin(ytp_stream_t *ctrl,
                                          fmc_error_t **error);

FMMODFUNC ytp_iterator_t ytp_stream_end(ytp_stream_t *ctrl,
                                        fmc_error_t **error);

FMMODFUNC bool ytp_stream_term(ytp_iterator_t iterator);

FMMODFUNC ytp_iterator_t ytp_stream_seek(ytp_stream_t *ctrl, size_t off,
                                         fmc_error_t **error);

FMMODFUNC size_t ytp_stream_tell(ytp_stream_t *ctrl, ytp_iterator_t iterator,
                                 fmc_error_t **error);

#ifdef __cplusplus
}
#endif
