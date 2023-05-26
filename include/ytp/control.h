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
 * @file control.h
 * @date 23 Apr 2021
 * @brief File contains C declaration of control layer of YTP.
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fmc/error.h>
#include <ytp/time.h>
#include <ytp/yamal.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Implements ytp control protocol
 *
 * Guaranties: control iterator is always at or ahead of
 * any ytp_iterator returned by the control API.
 */
typedef struct ytp_control ytp_control_t;

#define YTP_PEER_ANN 0
#define YTP_CHANNEL_ANN 0
#define YTP_CHANNEL_SUB 1
#define YTP_CHANNEL_DIR 2

#define YTP_PEER_OFF 0x100
#define YTP_CHANNEL_OFF 0x100

/**
 * @brief Allocates and initializes a ytp_control_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[out] error out-parameter for error handling
 * @return ytp_control_t object
 */
FMMODFUNC ytp_control_t *ytp_control_new(fmc_fd fd, fmc_error_t **error);

/**
 * @brief Initializes a ytp_control_t object
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] fd a yamal file descriptor
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_init(ytp_control_t *ctrl, fmc_fd fd,
                                fmc_error_t **error);

/**
 * @brief Allocates and initializes a ytp_control_t object
 *
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enables the auxiliary thread
 * @param[out] error out-parameter for error handling
 * @return ytp_control_t object
 */
FMMODFUNC ytp_control_t *ytp_control_new_2(fmc_fd fd, bool enable_thread,
                                           fmc_error_t **error);

/**
 * @brief Initializes a ytp_control_t object
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] fd a yamal file descriptor
 * @param[in] enable_thread enables the auxiliary thread
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_init_2(ytp_control_t *ctrl, fmc_fd fd,
                                  bool enable_thread, fmc_error_t **error);

/**
 * @brief Destroys and deallocate a ytp_control_t object
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_del(ytp_control_t *ctrl, fmc_error_t **error);

/**
 * @brief Destroys a ytp_control_t object
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_destroy(ytp_control_t *ctrl, fmc_error_t **error);

/**
 * @brief Reserves memory for data in the memory mapped list
 * to be used to write to the file.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] sz size of the buffer to hold the memory
 * @param[out] error out-parameter for error handling
 * @return buffer to hold the reserved memory
 */
FMMODFUNC char *ytp_control_reserve(ytp_control_t *ctrl, size_t sz,
                                    fmc_error_t **error);

/**
 * @brief Commits the data to the memory mapped node to write
 * it to the file.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the data
 * @param[in] channel the channel to publish the data
 * @param[in] msgtime the time to publish the data
 * @param[in] data the value returned by ytp_control_reserve()
 * @param[out] error out-parameter for error handling
 * @return ytp_iterator_t for the message
 */
FMMODFUNC ytp_iterator_t ytp_control_commit(ytp_control_t *ctrl,
                                            ytp_peer_t peer,
                                            ytp_channel_t channel,
                                            uint64_t msgtime, void *data,
                                            fmc_error_t **error);

/**
 * @brief Publishes a subscription message
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the subscription message
 * @param[in] msgtime the time to publish the subscription message
 * @param[in] sz size of the payload
 * @param[in] payload a prefix or channel name
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_sub(ytp_control_t *ctrl, ytp_peer_t peer,
                               uint64_t time, size_t sz, const char *payload,
                               fmc_error_t **error);

/**
 * @brief Publishes a directory message
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the directory message
 * @param[in] msgtime the time to publish the directory message
 * @param[in] sz size of the payload
 * @param[in] payload a SCDP encoded string
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_dir(ytp_control_t *ctrl, ytp_peer_t peer,
                               uint64_t time, size_t sz, const char *payload,
                               fmc_error_t **error);

/**
 * @brief Returns the name of the channel, given the channel reference
 *
 * Complexity: Constant on average, worst case linear in the number of channels.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] channel channel reference to obtain the name
 * @param[out] sz size of the channel name
 * @param[out] name name of the channel
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel,
                                   size_t *sz, const char **name,
                                   fmc_error_t **error);

/**
 * @brief Declares an existing/new channel
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer the peer that publishes the channel announcement
 * @param[in] msgtime the time to publish the channel announcement
 * @param[in] sz size of the channel name
 * @param[in] name name of the channel
 * @param[out] error out-parameter for error handling
 * @return channel id
 */
FMMODFUNC ytp_channel_t ytp_control_ch_decl(ytp_control_t *ctrl,
                                            ytp_peer_t peer, uint64_t msgtime,
                                            size_t sz, const char *name,
                                            fmc_error_t **error);

/**
 * @brief Returns the name of the peer, given the peer reference
 *
 * Complexity: Constant on average, worst case linear in the number of peers.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] peer peer reference to obtain the name
 * @param[out] sz size of the peer name
 * @param[out] name name of the peer
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer,
                                     size_t *sz, const char **name,
                                     fmc_error_t **error);

/**
 * @brief Declares an existing/new peer
 *
 * Complexity: Constant on average, worst case linear in the size of the list.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] sz size of the peer name
 * @param[in] name name of the peer
 * @param[out] error out-parameter for error handling
 * @return peer id
 */
FMMODFUNC ytp_peer_t ytp_control_peer_decl(ytp_control_t *ctrl, size_t sz,
                                           const char *name,
                                           fmc_error_t **error);

/**
 * @brief moves iterator forward
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] iter input iterator
 * @param[out] error out-parameter for error handling
 * @return the iterator of the next element
 */
FMMODFUNC ytp_iterator_t ytp_control_next(ytp_control_t *ctrl,
                                          ytp_iterator_t iter,
                                          fmc_error_t **error);

/**
 * @brief Reads a message on control level
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[in] iter iterator that points to the message node to read from
 * @param[out] peer peer associated of the read message
 * @param[out] channel channel associated of the read message
 * @param[out] time time associated of the read message
 * @param[out] sz size of the read message data
 * @param[out] data pointer to the read message data
 * @param[out] error out-parameter for error handling
 */
FMMODFUNC void ytp_control_read(ytp_control_t *ctrl, ytp_iterator_t iter,
                                ytp_peer_t *peer, ytp_channel_t *channel,
                                uint64_t *msgtime, size_t *sz,
                                const char **data, fmc_error_t **error);

/**
 * @brief Returns the iterator to the beginning of the list, the first node.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[out] error out-parameter for error handling
 * @return iterator to the beginning of the list
 */
FMMODFUNC ytp_iterator_t ytp_control_begin(ytp_control_t *ctrl,
                                           fmc_error_t **error);

/**
 * @brief Returns the iterator to the end of the list, the last node.
 *
 * @param[in] ctrl the ytp_control_t object
 * @param[out] error out-parameter for error handling
 * @return iterator to the end of the list
 */
FMMODFUNC ytp_iterator_t ytp_control_end(ytp_control_t *ctrl,
                                         fmc_error_t **error);
/**
 * @brief Checks if there are not more messages
 *
 * @param[in] iterator iterator to test
 * @return true if there are not more messages, false otherwise
 */
FMMODFUNC bool ytp_control_term(ytp_iterator_t iterator);

/**
 * @brief Returns an iterator given a serializable ptr.
 *
 * @param[in] ctrl ytp_control_t object
 * @param[in] off the serializable ptr offset
 * @param[out] error out-parameter for error handling
 * @return the iterator of the serializable ptr
 */
FMMODFUNC ytp_iterator_t ytp_control_seek(ytp_control_t *ctrl, uint64_t off,
                                          fmc_error_t **error);

/**
 * @brief Returns serializable offset given an iterator
 *
 * @param[in] ctrl ytp_control_t object
 * @param[in] iterator the iterator of the serializable ptr
 * @param[out] error out-parameter for error handling
 * @return the serializable ptr offset
 */
FMMODFUNC uint64_t ytp_control_tell(ytp_control_t *ctrl,
                                    ytp_iterator_t iterator,
                                    fmc_error_t **error);

#ifdef __cplusplus
}
#endif
