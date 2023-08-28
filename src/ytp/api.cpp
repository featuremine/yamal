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
 * @file api.cpp
 * @date 4 Oct 2022
 * @brief File contains implementation of yamal sequence API
 *
 * File contains implementation of yamal sequence API
 * @see http://www.featuremine.com
 */

#include "ytp/api.h"
#include "ytp/sequence.h"
#include "ytp/stream.h"

// Reserves memory for data in the memory mapped list
char *ytp_sequence_shared_reserve(shared_sequence *sh_seq, size_t sz,
                                  fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_reserve(seq, sz, error);
}
// Commits the data to the memory mapped list
ytp_iterator_t ytp_sequence_shared_commit(shared_sequence *sh_seq,
                                          ytp_peer_t peer,
                                          ytp_channel_t channel, uint64_t time,
                                          void *data, fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_commit(seq, peer, channel, time, data, error);
}
// Commits the multiple data messages to the memory mapped list
void ytp_sequence_shared_sublist_commit(shared_sequence *sh_seq,
                                        ytp_peer_t peer, ytp_channel_t channel,
                                        uint64_t time, void **first_ptr,
                                        void **last_ptr, void *new_ptr,
                                        fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_sublist_commit(seq, peer, channel, time, first_ptr, last_ptr,
                              new_ptr, error);
}
// Commits the multiple data messages to the memory mapped list
ytp_iterator_t ytp_sequence_shared_sublist_finalize(shared_sequence *sh_seq,
                                                    void *first_ptr,
                                                    fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_sublist_finalize(seq, first_ptr, error);
}
// Publishes a subscription message
void ytp_sequence_shared_sub(shared_sequence *sh_seq, ytp_peer_t peer,
                             uint64_t time, size_t sz, const char *payload,
                             fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_sub(seq, peer, time, sz, payload, error);
}
// Publishes a directory message
void ytp_sequence_shared_dir(shared_sequence *sh_seq, ytp_peer_t peer,
                             uint64_t time, size_t sz, const char *payload,
                             fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_dir(seq, peer, time, sz, payload, error);
}
// Returns the name of the channel, given the channel reference
void ytp_sequence_shared_ch_name(shared_sequence *sh_seq, ytp_channel_t channel,
                                 size_t *sz, const char **name,
                                 fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_ch_name(seq, channel, sz, name, error);
}
// Declares an existing/new channel
ytp_channel_t ytp_sequence_shared_ch_decl(shared_sequence *sh_seq,
                                          ytp_peer_t peer, uint64_t time,
                                          size_t sz, const char *name,
                                          fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_ch_decl(seq, peer, time, sz, name, error);
}
// Registers a channel announcement callback
void ytp_sequence_shared_ch_cb(shared_sequence *sh_seq, ytp_sequence_ch_cb_t cb,
                               void *closure, fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_ch_cb(seq, cb, closure, error);
}
// Unregisters a channel announcement callback
void ytp_sequence_shared_ch_cb_rm(shared_sequence *sh_seq,
                                  ytp_sequence_ch_cb_t cb, void *closure,
                                  fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_ch_cb_rm(seq, cb, closure, error);
}
// Returns the name of the peer, given the peer reference
void ytp_sequence_shared_peer_name(shared_sequence *sh_seq, ytp_peer_t peer,
                                   size_t *sz, const char **name,
                                   fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_peer_name(seq, peer, sz, name, error);
}
// Declares an existing/new peer
ytp_peer_t ytp_sequence_shared_peer_decl(shared_sequence *sh_seq, size_t sz,
                                         const char *name,
                                         fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_peer_decl(seq, sz, name, error);
}
// Registers a peer announcement callback
void ytp_sequence_shared_peer_cb(shared_sequence *sh_seq,
                                 ytp_sequence_peer_cb_t cb, void *closure,
                                 fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_peer_cb(seq, cb, closure, error);
}
// Unregisters a peer announcement callback
void ytp_sequence_shared_peer_cb_rm(shared_sequence *sh_seq,
                                    ytp_sequence_peer_cb_t cb, void *closure,
                                    fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_peer_cb_rm(seq, cb, closure, error);
}
// Registers a channel data callback by channel name or prefix
void ytp_sequence_shared_prfx_cb(shared_sequence *sh_seq, size_t sz,
                                 const char *prfx, ytp_sequence_data_cb_t cb,
                                 void *closure, fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_prfx_cb(seq, sz, prfx, cb, closure, error);
}
// Unregisters a channel data callback by channel name or prefix
void ytp_sequence_shared_prfx_cb_rm(shared_sequence *sh_seq, size_t sz,
                                    const char *prfx, ytp_sequence_data_cb_t cb,
                                    void *closure, fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_prfx_cb_rm(seq, sz, prfx, cb, closure, error);
}

// Registers a channel data callback by channel handler
void ytp_sequence_shared_indx_cb(shared_sequence *sh_seq, ytp_channel_t channel,
                                 ytp_sequence_data_cb_t cb, void *closure,
                                 fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_indx_cb(seq, channel, cb, closure, error);
}
// Unregisters a channel data callback by channel handler
void ytp_sequence_shared_indx_cb_rm(shared_sequence *sh_seq,
                                    ytp_channel_t channel,
                                    ytp_sequence_data_cb_t cb, void *closure,
                                    fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_indx_cb_rm(seq, channel, cb, closure, error);
}
// Reads one message and executes the callbacks that applies.
bool ytp_sequence_shared_poll(shared_sequence *sh_seq, fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_poll(seq, error);
}
// Checks if there are not more messages
bool ytp_sequence_shared_term(shared_sequence *sh_seq) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_term(seq);
}
// Returns the iterator to the end of yamal
ytp_iterator_t ytp_sequence_shared_end(shared_sequence *sh_seq,
                                       fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_end(seq, error);
}
// Returns the current data iterator
ytp_iterator_t ytp_sequence_shared_cur(shared_sequence *sh_seq) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_cur(seq);
}
// Returns the current data iterator
ytp_iterator_t ytp_sequence_shared_get_it(shared_sequence *sh_seq) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_get_it(seq);
}
// Sets the current data iterator
void ytp_sequence_shared_set_it(shared_sequence *sh_seq,
                                ytp_iterator_t iterator) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  ytp_sequence_set_it(seq, iterator);
}
// Returns an iterator given a serializable offset
ytp_iterator_t ytp_sequence_shared_seek(shared_sequence *sh_seq, size_t off,
                                        fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_seek(seq, off, error);
}
// Returns serializable offset given an iterator
size_t ytp_sequence_shared_tell(shared_sequence *sh_seq,
                                ytp_iterator_t iterator, fmc_error_t **error) {
  ytp_sequence_t *seq =
      ytp_sequence_shared_get((ytp_sequence_shared_t *)sh_seq);
  return ytp_sequence_tell(seq, iterator, error);
}

static struct ytp_sequence_api_v1 api_v1 {
  ytp_sequence_shared_reserve, ytp_sequence_shared_commit,
      ytp_sequence_shared_sub, ytp_sequence_shared_dir,
      ytp_sequence_shared_ch_name, ytp_sequence_shared_ch_decl,
      ytp_sequence_shared_ch_cb, ytp_sequence_shared_ch_cb_rm,
      ytp_sequence_shared_peer_name, ytp_sequence_shared_peer_decl,
      ytp_sequence_shared_peer_cb, ytp_sequence_shared_peer_cb_rm,
      ytp_sequence_shared_prfx_cb, ytp_sequence_shared_prfx_cb_rm,
      ytp_sequence_shared_indx_cb, ytp_sequence_shared_indx_cb_rm,
      ytp_sequence_shared_poll, ytp_sequence_shared_term,
      ytp_sequence_shared_end, ytp_sequence_shared_cur,
      ytp_sequence_shared_get_it, ytp_sequence_shared_set_it,
      ytp_sequence_shared_seek, ytp_sequence_shared_tell,
      (sharedseqfunc_inc)ytp_sequence_shared_inc,
      (sharedseqfunc_dec)ytp_sequence_shared_dec
};

static struct ytp_sequence_api_v2 api_v2 {
  api_v1.sequence_reserve, api_v1.sequence_commit,
      ytp_sequence_shared_sublist_commit, ytp_sequence_shared_sublist_finalize,
      api_v1.sequence_sub, api_v1.sequence_dir, api_v1.sequence_ch_name,
      api_v1.sequence_ch_decl, api_v1.sequence_ch_cb, api_v1.sequence_ch_cb_rm,
      api_v1.sequence_peer_name, api_v1.sequence_peer_decl,
      api_v1.sequence_peer_cb, api_v1.sequence_peer_cb_rm,
      api_v1.sequence_prfx_cb, api_v1.sequence_prfx_cb_rm,
      api_v1.sequence_indx_cb, api_v1.sequence_indx_cb_rm, api_v1.sequence_poll,
      api_v1.sequence_term, api_v1.sequence_end, api_v1.sequence_cur,
      api_v1.sequence_get_it, api_v1.sequence_set_it, api_v1.sequence_seek,
      api_v1.sequence_tell, api_v1.sequence_shared_inc,
      api_v1.sequence_shared_dec,
};

struct ytp_sequence_api_v1 *ytp_sequence_api_v1_get() {
  return &api_v1;
}

struct ytp_sequence_api_v2 *ytp_sequence_api_v2_get() {
  return &api_v2;
}
