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
 * @file api.h
 * @date 4 Oct 2022
 * @brief File contains C declaration of yamal sequence API
 *
 * File contains C declaration of yamal sequence API
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/error.h>
#include <fmc/files.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void shared_sequence;
typedef uint64_t ytp_peer_t;
typedef uint64_t ytp_channel_t;
typedef void *ytp_iterator_t;
typedef void (*ytp_sequence_peer_cb_t)(void *closure, ytp_peer_t peer,
                                       size_t sz, const char *name);
typedef void (*ytp_sequence_ch_cb_t)(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, uint64_t msgtime,
                                     size_t sz, const char *name);
typedef void (*ytp_sequence_data_cb_t)(void *closure, ytp_peer_t peer,
                                       ytp_channel_t channel, uint64_t msgtime,
                                       size_t sz, const char *data);

typedef char *(*sharedseqfunc_reserve)(shared_sequence *, size_t,
                                       fmc_error_t **);
typedef ytp_iterator_t (*sharedseqfunc_commit)(shared_sequence *, ytp_peer_t,
                                               ytp_channel_t, uint64_t, void *,
                                               fmc_error_t **);
typedef void (*sharedseqfunc_sub)(shared_sequence *, ytp_peer_t, uint64_t,
                                  size_t, const char *, fmc_error_t **);
typedef void (*sharedseqfunc_dir)(shared_sequence *, ytp_peer_t, uint64_t,
                                  size_t, const char *, fmc_error_t **);
typedef void (*sharedseqfunc_ch_name)(shared_sequence *, ytp_channel_t,
                                      size_t *, const char **, fmc_error_t **);
typedef ytp_channel_t (*sharedseqfunc_ch_decl)(shared_sequence *, ytp_peer_t,
                                               uint64_t, size_t, const char *,
                                               fmc_error_t **);
typedef void (*sharedseqfunc_ch_cb)(shared_sequence *, ytp_sequence_ch_cb_t,
                                    void *, fmc_error_t **);
typedef void (*sharedseqfunc_ch_cb_rm)(shared_sequence *, ytp_sequence_ch_cb_t,
                                       void *, fmc_error_t **);
typedef void (*sharedseqfunc_peer_name)(shared_sequence *, ytp_peer_t, size_t *,
                                        const char **, fmc_error_t **);
typedef ytp_peer_t (*sharedseqfunc_peer_decl)(shared_sequence *, size_t,
                                              const char *, fmc_error_t **);
typedef void (*sharedseqfunc_peer_cb)(shared_sequence *, ytp_sequence_peer_cb_t,
                                      void *, fmc_error_t **);
typedef void (*sharedseqfunc_peer_cb_rm)(shared_sequence *,
                                         ytp_sequence_peer_cb_t, void *,
                                         fmc_error_t **);
typedef void (*sharedseqfunc_prfx_cb)(shared_sequence *, size_t, const char *,
                                      ytp_sequence_data_cb_t, void *,
                                      fmc_error_t **);
typedef void (*sharedseqfunc_prfx_cb_rm)(shared_sequence *, size_t,
                                         const char *, ytp_sequence_data_cb_t,
                                         void *, fmc_error_t **);
typedef void (*sharedseqfunc_indx_cb)(shared_sequence *, ytp_channel_t,
                                      ytp_sequence_data_cb_t, void *,
                                      fmc_error_t **);
typedef void (*sharedseqfunc_indx_cb_rm)(shared_sequence *, ytp_channel_t,
                                         ytp_sequence_data_cb_t, void *,
                                         fmc_error_t **);
typedef bool (*sharedseqfunc_poll)(shared_sequence *, fmc_error_t **);
typedef bool (*sharedseqfunc_term)(shared_sequence *);
typedef ytp_iterator_t (*sharedseqfunc_end)(shared_sequence *, fmc_error_t **);
typedef ytp_iterator_t (*sharedseqfunc_cur)(shared_sequence *);
typedef ytp_iterator_t (*sharedseqfunc_get_it)(shared_sequence *);
typedef void (*sharedseqfunc_set_it)(shared_sequence *, ytp_iterator_t);
typedef ytp_iterator_t (*sharedseqfunc_seek)(shared_sequence *, uint64_t,
                                             fmc_error_t **);
typedef uint64_t (*sharedseqfunc_tell)(shared_sequence *, ytp_iterator_t,
                                       fmc_error_t **);
typedef void (*sharedseqfunc_inc)(shared_sequence *);
typedef void (*sharedseqfunc_dec)(shared_sequence *, fmc_error_t **);

struct ytp_sequence_api_v1 {
  // Reserves memory for data in the memory mapped list
  sharedseqfunc_reserve sequence_reserve;
  // Commits the data to the memory mapped list
  sharedseqfunc_commit sequence_commit;
  // Publishes a subscription message
  sharedseqfunc_sub sequence_sub;
  // Publishes a directory message
  sharedseqfunc_dir sequence_dir;
  // Returns the name of the channel, given the channel reference
  sharedseqfunc_ch_name sequence_ch_name;
  // Declares an existing/new channel
  sharedseqfunc_ch_decl sequence_ch_decl;
  // Registers a channel announcement callback
  sharedseqfunc_ch_cb sequence_ch_cb;
  // Unregisters a channel announcement callback
  sharedseqfunc_ch_cb_rm sequence_ch_cb_rm;
  // Returns the name of the peer, given the peer reference
  sharedseqfunc_peer_name sequence_peer_name;
  // Declares an existing/new peer
  sharedseqfunc_peer_decl sequence_peer_decl;
  // Registers a peer announcement callback
  sharedseqfunc_peer_cb sequence_peer_cb;
  // Unregisters a peer announcement callback
  sharedseqfunc_peer_cb_rm sequence_peer_cb_rm;
  // Registers a channel data callback by channel name or prefix
  sharedseqfunc_prfx_cb sequence_prfx_cb;
  // Unregisters a channel data callback by channel name or prefix
  sharedseqfunc_prfx_cb_rm sequence_prfx_cb_rm;
  // Registers a channel data callback by channel handler
  sharedseqfunc_indx_cb sequence_indx_cb;
  // Unregisters a channel data callback by channel handler
  sharedseqfunc_indx_cb_rm sequence_indx_cb_rm;
  // Reads one message and executes the callbacks that applies.
  sharedseqfunc_poll sequence_poll;
  // Checks if there are not more messages
  sharedseqfunc_term sequence_term;
  // Returns the iterator to the end of yamal
  sharedseqfunc_end sequence_end;
  // Returns the current data iterator
  sharedseqfunc_cur sequence_cur;
  // Returns the current data iterator
  sharedseqfunc_get_it sequence_get_it;
  // Sets the current data iterator
  sharedseqfunc_set_it sequence_set_it;
  // Returns an iterator given a serializable offset
  sharedseqfunc_seek sequence_seek;
  // Returns serializable offset given an iterator
  sharedseqfunc_tell sequence_tell;
  // Increases the reference counter
  sharedseqfunc_inc sequence_shared_inc;
  // Decreases the reference counter and call ytp_sequence_del the sequence
  sharedseqfunc_dec sequence_shared_dec;
};

// function that you can call to return the actual sequence api structure
// pointer
struct ytp_sequence_api_v1 *ytp_sequence_api_v1_get();

#ifdef __cplusplus
}
#endif
