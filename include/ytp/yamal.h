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
 * @author Featuremine Corporation
 * @date 10 May 2022
 * @brief File contains C declaration of yamal layer of YTP
 *
 * This file contains C declaration of yamal layer of YTP.
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_YAMAL_H__
#define __FM_YTP_YAMAL_H__

#include <stdbool.h> 
#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_file_io.h> // apr_file_t
#include <ytp/errno.h> // ytp_status_t

#define YTP_MMLIST_PAGE_SIZE (1024 * 1024 * 8)
#define YTP_MMLIST_PREALLOC_SIZE (1024 * 1024 * 3)

#ifdef __cplusplus
extern "C" {
#endif

#define YAMAL_PAGES (1024 * 64 * 8)

typedef struct ytp_yamal ytp_yamal_t;
typedef void *ytp_iterator_t;

/**
 * @brief This MUST be the first function called before any Yamal library
 * function.
 * It is safe to call ytp_initialize() several times as long as ytp_terminate()
 * is called the same number of times.
 *
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_initialize(void);

/**
 * @brief Terminates the ytp library. ytp_terminate() must be called once for
 * every call to ytp_initialize().
 *
 */
APR_DECLARE(void) ytp_terminate(void);

/**
 * @brief Initializes a ytp_yamal_t object
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] f APR file object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_init(ytp_yamal_t *yamal, apr_file_t *f);

/**
 * @brief Allocates and initializes a ytp_yamal_t object
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] f APR file object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_new(ytp_yamal_t **yamal, apr_file_t *f);

/**
 * @brief Initializes a ytp_yamal_t object
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] f APR file object
 * @param[in] enable_thread enable or disable the preallocation and sync thread
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_init2(ytp_yamal_t *yamal, apr_file_t *f, bool enable_thread);

/**
 * @brief Allocates and initializes a ytp_yamal_t object
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] f APR file object
 * @param[in] enable_thread enable or disable the preallocation and sync thread
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_new2(ytp_yamal_t **yamal, apr_file_t *f, bool enable_thread);

/**
 * @brief Returns the APR file object from a ytp_yamal_t object
 *
 * @param[out] yamal ytp_yamal_t object
 * @return APR file object
 */
APR_DECLARE(apr_file_t *) ytp_yamal_file(ytp_yamal_t *yamal);

/**
 * @brief Reserves memory for data in the memory mapped list
 * to be used to write to the file.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] buf a buffer to hold the reserved memory.
 * @param[out] bufsize size of the buffer to hold the memory.
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_reserve(ytp_yamal_t *yamal, char **buf, 
                                            apr_size_t bufsize);

/**
 * @brief Commits the data to the memory mapped node to write
 * it to the file.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[out] it iterator to the next memory mapped node
 * @param[in] data the value returned by ytp_yamal_reserve
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_commit(ytp_yamal_t *yamal,
                                           ytp_iterator_t *it, void *data);

/**
 * @brief Reads a message of the memory mapped node.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[in] iterator iterator that points to the memory mapped node to read
 * from
 * @param[out] sz size of the read data
 * @param[out] data pointer to the read data
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                         apr_size_t *sz, const char **data);

/**
 * @brief Destroys a ytp_yamal_t object
 *
 * @param[out] yamal ytp_yamal_t object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_destroy(ytp_yamal_t *yamal);

/**
 * @brief Destroys and deallocates a ytp_yamal_t object
 *
 * @param[out] yamal ytp_yamal_t object
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_del(ytp_yamal_t *yamal);

/**
 * @brief Obtains an iterator to the beginning of the list, the first node.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[out] iterator iterator to the beginning of the list
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_begin(ytp_yamal_t *yamal,
                                         ytp_iterator_t *iterator);

/**
 * @brief Obtains an iterator to the end of the list, the last node.
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[out] iterator iterator to the end of the list
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_end(ytp_yamal_t *yamal, ytp_iterator_t *iterator);

/**
 * @brief Checks if there are not more messages
 *
 * @param[in] iterator iterator to test
 * @return true if there are no more messages, false otherwise
 */
APR_DECLARE(bool) ytp_yamal_term(ytp_iterator_t iterator);

/**
 * @brief Obtains the next iterator
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[out] it_ptr next iterator
 * @param[in] iterator input iterator
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_next(ytp_yamal_t *yamal,
                                         ytp_iterator_t *it_ptr,
                                         ytp_iterator_t iterator);

/**
 * @brief Obtains the previous iterator
 *
 * @param[out] yamal ytp_yamal_t object
 * @param[out] it_ptr previous iterator
 * @param[in] iterator input iterator
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_prev(ytp_yamal_t *yamal,
                                         ytp_iterator_t *it_ptr,
                                         ytp_iterator_t iterator);

/**
 * @brief Removes a memory mapped node from the list
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[out] it_ptr the next iterator
 * @param[in] iterator input iterator to the node to be removed
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_remove(ytp_yamal_t *yamal,
                                           ytp_iterator_t *it_ptr,
                                           ytp_iterator_t iterator);

/**
 * @brief Obtains an iterator given a serializable ptr
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[out] it_ptr the iterator of the serializable ptr
 * @param[in] ptr the serializable ptr offset
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_seek(ytp_yamal_t *yamal,
                                         ytp_iterator_t *it_ptr,
                                         apr_size_t ptr);

/**
 * @brief Obtains a serializable ptr offset given an iterator
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[out] ptr the serializable ptr offset
 * @param[in] iterator the iterator of the serializable ptr
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_tell(ytp_yamal_t *yamal,
                                         apr_size_t *ptr,
                                         ytp_iterator_t iterator);

/**
 * @brief Allocates a specific page
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[in] page
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_allocate_page(ytp_yamal_t *yamal, apr_size_t page);

/**
 * @brief Returns the reserved size
 *
 * @param[in] yamal ytp_yamal_t object
 * @param[out] size reserved size
 * @return ytp_status_t with the outcome of the function
 */
APR_DECLARE(ytp_status_t) ytp_yamal_reserved_size(ytp_yamal_t *yamal, apr_size_t *size);

#ifdef __cplusplus
}
#endif

#endif // __FM_YTP_YAMAL_H__
