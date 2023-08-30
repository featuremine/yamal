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
 * @file index.h
 * @date 6 Jun 2023
 * @brief File contains C declaration of index API
 *
 * This file contains C declaration of index API
 * @see http://www.featuremine.com
 */

#pragma once

#include <ytp/api.h>
#include <ytp/stream.h>
#include <ytp/yamal.h>

#include <fmc/error.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Writes a raw stream index message
 *
 * @param[in] yamal
 * @param[in] stream
 * @param[in] offset
 * @param[in] sz index payload size
 * @param[in] payload index payload
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_index_write(ytp_yamal_t *yamal,
                                         ytp_mmnode_offs stream,
                                         ytp_mmnode_offs offset, size_t sz,
                                         const void *payload,
                                         fmc_error_t **error);

/**
 * @brief Reads a raw stream index message
 *
 * @param[in] yamal
 * @param[in] iterator
 * @param[out] seqno
 * @param[out] stream
 * @param[out] data_offset
 * @param[out] sz index payload size
 * @param[out] payload index payload
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_index_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                              uint64_t *seqno, ytp_mmnode_offs *stream,
                              ytp_mmnode_offs *data_offset, size_t *sz,
                              const char **payload, fmc_error_t **error);

/**
 * @brief Lookups a raw stream index message
 *
 * @param[in] yamal
 * @param[in] offset
 * @param[out] seqno
 * @param[out] stream
 * @param[out] data_offset
 * @param[out] sz index payload size
 * @param[out] payload index payload
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_index_lookup(ytp_yamal_t *yamal, ytp_mmnode_offs offset,
                                uint64_t *seqno, ytp_mmnode_offs *stream,
                                ytp_mmnode_offs *data_offset, size_t *sz,
                                const char **payload, fmc_error_t **error);

/**
 * @brief Returns an iterator to the beginning of the list
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
static inline ytp_iterator_t ytp_index_begin(ytp_yamal_t *yamal,
                                             fmc_error_t **error) {
  return ytp_yamal_begin(yamal, YTP_STREAM_LIST_INDX, error);
}

/**
 * @brief Returns an iterator to the end of the list
 *
 * @param[in] yamal
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
static inline ytp_iterator_t ytp_index_end(ytp_yamal_t *yamal,
                                           fmc_error_t **error) {
  return ytp_yamal_end(yamal, YTP_STREAM_LIST_INDX, error);
}

#ifdef __cplusplus
}
#endif
