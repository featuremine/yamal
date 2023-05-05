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
 * @file time.h
 * @date 23 Apr 2021
 * @brief File contains C declaration of time layer of YTP
 *
 * Timestamp is the original time of the message. Whenever the message is copied
 * or forwarded, it should maintain the timestamp.
 *
 * <table>
 * <caption id="multi_row">Time message</caption>
 * <tr><th colspan="4">peer/channel/time
 * <tr><th>8 bytes  <th>8 bytes  <th>8 bytes  <th>  variable
 * <tr><td>Peer ID  <td>Channel ID  <td>Timestmap  <td>Data
 * </table>
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reserves memory for data in the memory mapped list
 *
 * @param[in] yamal
 * @param[in] sz the size of the data payload
 * @param[out] error
 * @return a writable pointer for data
 */
FMMODFUNC char *ytp_time_reserve(ytp_yamal_t *yamal, size_t sz,
                                 fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list
 *
 * @param[in] yamal
 * @param[in] time
 * @param[in] data the value returned by ytp_time_reserve
 * @param[out] error
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, uint64_t msgtime,
                                         void *data, size_t listidx,
                                         fmc_error_t **error);

/**
 * @brief Reads a message on channel level
 *
 * @param[in] yamal
 * @param[in] iterator
 * @param[out] seqno
 * @param[out] time
 * @param[out] sz
 * @param[out] data
 * @param[out] error
 */
FMMODFUNC void ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                             size_t *seqno, uint64_t *time, size_t *sz,
                             const char **data, fmc_error_t **error);

#ifdef __cplusplus
}
#endif
