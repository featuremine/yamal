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
 * @file yamal.h
 * @date 23 Apr 2021
 * @brief File contains C declaration of yamal layer of YTP
 *
 * This file contains C declaration of yamal layer of YTP.
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_YAMAL_H__
#define __FM_YTP_YAMAL_H__

#include <fmc/files.h>
#include <stddef.h>
#include <ytp/api.h>

#include <fmc/error.h>

#define YTP_MMLIST_PAGE_SIZE (1024 * 1024 * 8)
#define YTP_MMLIST_PREALLOC_SIZE (1024 * 1024 * 3)
#define YTP_MMNODE_HEADER_SIZE 32
#define YTP_YAMAL_HEADER_SIZE 536
#define YTP_YAMAL_LISTS 16

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ytp_yamal ytp_yamal_t;

typedef enum { CLOSABLE = 1, UNCLOSABLE = 2 } FMC_CLOSABLE;

/**
 * @brief Initializes a ytp_yamal_t object
 *
 * @param[out] yamal
 * @param[in] fd a yamal file descriptor
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_yamal_init(ytp_yamal_t *yamal, int fd, fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_yamal_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[out] error out-parameter for error handling
 * @return ytp_yamal_t object
 */
FMMODFUNC ytp_yamal_t *ytp_yamal_new(int fd, fmc_error_t **error);

/**
 * @brief Sets CPU affinity for the auxillary yamal thread
 *
 * @param[in] cpuid a CPU ID to use for the affinity
 */
FMMODFUNC void ytp_yamal_set_aux_thread_affinity(int cpuid);

/**
 * @brief Clears CPU affinity for the auxillary yamal thread
 */
FMMODFUNC void ytp_yamal_clear_aux_thread_affinity();

/**
 * @brief Initializes a ytp_yamal_t object
 *
 * @param[out] yamal
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enables the auxiliary thread
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_yamal_init_2(ytp_yamal_t *yamal, int fd, bool enable_thread,
                                fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_yamal_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enables the auxiliary thread
 * @param[out] error out-parameter for error handling
 * @return ytp_yamal_t object
 */
FMMODFUNC ytp_yamal_t *ytp_yamal_new_2(int fd, bool enable_thread,
                                       fmc_error_t **error);

/**
 * @brief Initializes a ytp_yamal_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enables the auxiliary thread
 * @param[in] closable closable mode
 * @param[out] error out-parameter for error handling
 * @return ytp_yamal_t object
 */
FMMODFUNC void ytp_yamal_init_3(ytp_yamal_t *yamal, int fd, bool enable_thread,
                                FMC_CLOSABLE closable, fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_yamal_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enables the auxiliary thread
 * @param[in] closable closable mode
 * @param[out] error out-parameter for error handling
 * @return ytp_yamal_t object
 */
FMMODFUNC ytp_yamal_t *ytp_yamal_new_3(int fd, bool enable_thread,
                                       FMC_CLOSABLE closable,
                                       fmc_error_t **error);

/**
 * @brief Returns the file descriptor from a ytp_yamal_t object
 *
 * @param[in] yamal the ytp_yamal_t object
 * @return fmc_fd
 */
FMMODFUNC fmc_fd ytp_yamal_fd(ytp_yamal_t *yamal);

/**
 * @brief Reserves memory for data in the memory mapped list
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] sz the size of the data payload
 * @param[out] error out-parameter for error handling
 * @return a writable pointer for data
 */
FMMODFUNC char *ytp_yamal_reserve(ytp_yamal_t *yamal, size_t sz,
                                  fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped list
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] data the value returned by ytp_yamal_reserve
 * @param[in] lstidx the list index to commit to
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_yamal_commit(ytp_yamal_t *yamal, void *data,
                                          size_t lstidx, fmc_error_t **error);

/**
 * @brief Reads a message on yamal level
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] iterator
 * @param[out] seqno
 * @param[out] sz
 * @param[out] data
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_yamal_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                              uint64_t *seqno, size_t *sz, const char **data,
                              fmc_error_t **error);

/**
 * @brief Destroys a ytp_yamal_t object
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_yamal_destroy(ytp_yamal_t *yamal, fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_yamal_t object
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_yamal_del(ytp_yamal_t *yamal, fmc_error_t **error);

/**
 * @brief Returns an iterator to the beginning of the list
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] lstidx
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_yamal_begin(ytp_yamal_t *yamal, size_t lstidx,
                                         fmc_error_t **error);

/**
 * @brief Returns an iterator to the end of the list
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] lstidx
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_yamal_end(ytp_yamal_t *yamal, size_t lstidx,
                                       fmc_error_t **error);

/**
 * @brief Checks if there are not more messages
 *
 * @param[in] iterator
 * @return true if there are not more messages, false otherwise
 */
FMMODFUNC bool ytp_yamal_term(ytp_iterator_t iterator);

/**
 * @brief Returns the next iterator
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] iterator
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_yamal_next(ytp_yamal_t *yamal,
                                        ytp_iterator_t iterator,
                                        fmc_error_t **error);

/**
 * @brief Returns the previous iterator
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] iterator
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_yamal_prev(ytp_yamal_t *yamal,
                                        ytp_iterator_t iterator,
                                        fmc_error_t **error);

/**
 * @brief Returns an iterator given a serializable ptr
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] ptr
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t
 */
FMMODFUNC ytp_iterator_t ytp_yamal_seek(ytp_yamal_t *yamal, uint64_t ptr,
                                        fmc_error_t **error);

/**
 * @brief Returns serializable ptr given an iterator
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] iterator
 * @param[out] error out-parameter for error handling
 * @return serializable
 */
FMMODFUNC uint64_t ytp_yamal_tell(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                  fmc_error_t **error);

/**
 * @brief Closes the yamal lists
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_yamal_close(ytp_yamal_t *yamal, fmc_error_t **error);

/**
 * @brief Determines if a list is closed
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[out] error out-parameter for error handling
 * @return true if the list is closed, false otherwise
 */
FMMODFUNC bool ytp_yamal_closed(ytp_yamal_t *yamal, size_t lstidx,
                                fmc_error_t **error);

/**
 * @brief Allocates a specific page
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[in] page
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_yamal_allocate_page(ytp_yamal_t *yamal, size_t page,
                                       fmc_error_t **error);

/**
 * @brief Returns the reserved size
 *
 * @param[in] yamal the ytp_yamal_t object
 * @param[out] error out-parameter for error handling
 * @return yamal size
 */
FMMODFUNC size_t ytp_yamal_reserved_size(ytp_yamal_t *yamal,
                                         fmc_error_t **error);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_YAMAL_H__
