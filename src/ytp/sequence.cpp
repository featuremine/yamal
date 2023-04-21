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

#include "control.hpp"
#include "timeline.hpp"
#include "yamal.hpp"
#include "sequence.hpp"

#include <vector>
#include <ytp/control.h>
#include <ytp/sequence.h>
#include <ytp/yamal.h>

struct ytp_sequence_shared {
  uint64_t ref_counter = 1;
  fmc_fd fd;
  ytp_sequence_t seq;
};

ytp_sequence_t *ytp_sequence_new(fmc_fd fd, fmc_error_t **error) {
  return ytp_sequence_new_2(fd, true, error);
}

void ytp_sequence_init(ytp_sequence_t *seq, fmc_fd fd, fmc_error_t **error) {
  ytp_sequence_init_2(seq, fd, true, error);
}

ytp_sequence_t *ytp_sequence_new_2(fmc_fd fd, bool enable_thread,
                                   fmc_error_t **error) {
  auto *seq = new ytp_sequence_t;
  ytp_sequence_init_2(seq, fd, enable_thread, error);
  if (*error) {
    delete seq;
    return nullptr;
  }

  return seq;
}

void ytp_sequence_init_2(ytp_sequence_t *seq, fmc_fd fd, bool enable_thread,
                         fmc_error_t **error) {
  ytp_control_init_2(&seq->ctrl, fd, enable_thread, error);
  if (*error) {
    return;
  }

  ytp_timeline_init(&seq->timeline, &seq->ctrl, error);
  if (*error) {
    std::string err1_msg{fmc_error_msg(*error)};
    ytp_control_destroy(&seq->ctrl, error);
    if (*error) {
      fmc_error_set(error, "%s. %s", err1_msg.c_str(), fmc_error_msg(*error));
    } else {
      fmc_error_set(error, "%s", err1_msg.c_str());
    }
  }
}

void ytp_sequence_del(ytp_sequence_t *seq, fmc_error_t **error) {
  ytp_sequence_destroy(seq, error);
  delete seq;
}

void ytp_sequence_destroy(ytp_sequence_t *seq, fmc_error_t **error) {
  ytp_timeline_destroy(&seq->timeline, error);
  ytp_control_destroy(&seq->ctrl, error);
}

void ytp_sequence_peer_name(ytp_sequence_t *seq, ytp_peer_t peer, size_t *sz,
                            const char **name, fmc_error_t **error) {
  return ytp_control_peer_name(&seq->ctrl, peer, sz, name, error);
}

ytp_peer_t ytp_sequence_peer_decl(ytp_sequence_t *seq, size_t sz,
                                  const char *name, fmc_error_t **error) {
  return ytp_control_peer_decl(&seq->ctrl, sz, name, error);
}

void ytp_sequence_peer_cb(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb,
                          void *closure, fmc_error_t **error) {
  ytp_timeline_peer_cb(&seq->timeline, cb, closure, error);
}

void ytp_sequence_peer_cb_rm(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb,
                             void *closure, fmc_error_t **error) {
  ytp_timeline_peer_cb_rm(&seq->timeline, cb, closure, error);
}

void ytp_sequence_ch_name(ytp_sequence_t *seq, ytp_channel_t channel,
                          size_t *sz, const char **name, fmc_error_t **error) {
  ytp_control_ch_name(&seq->ctrl, channel, sz, name, error);
}

ytp_channel_t ytp_sequence_ch_decl(ytp_sequence_t *seq, ytp_peer_t peer,
                                   uint64_t time, size_t sz, const char *name,
                                   fmc_error_t **error) {
  return ytp_control_ch_decl(&seq->ctrl, peer, time, sz, name, error);
}

void ytp_sequence_ch_cb(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb,
                        void *closure, fmc_error_t **error) {
  ytp_timeline_ch_cb(&seq->timeline, cb, closure, error);
}

void ytp_sequence_ch_cb_rm(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb,
                           void *closure, fmc_error_t **error) {
  ytp_timeline_ch_cb_rm(&seq->timeline, cb, closure, error);
}

void ytp_sequence_prfx_cb(ytp_sequence_t *seq, size_t sz, const char *prfx,
                          ytp_sequence_data_cb_t cb, void *closure,
                          fmc_error_t **error) {
  ytp_timeline_prfx_cb(&seq->timeline, sz, prfx, cb, closure, error);
}

void ytp_sequence_prfx_cb_rm(ytp_sequence_t *seq, size_t sz, const char *prfx,
                             ytp_sequence_data_cb_t cb, void *closure,
                             fmc_error_t **error) {
  ytp_timeline_prfx_cb_rm(&seq->timeline, sz, prfx, cb, closure, error);
}

void ytp_sequence_indx_cb(ytp_sequence_t *seq, ytp_channel_t channel,
                          ytp_sequence_data_cb_t cb, void *closure,
                          fmc_error_t **error) {
  ytp_timeline_indx_cb(&seq->timeline, channel, cb, closure, error);
}

void ytp_sequence_indx_cb_rm(ytp_sequence_t *seq, ytp_channel_t channel,
                             ytp_sequence_data_cb_t cb, void *closure,
                             fmc_error_t **error) {
  ytp_timeline_indx_cb_rm(&seq->timeline, channel, cb, closure, error);
}

char *ytp_sequence_reserve(ytp_sequence_t *seq, size_t size,
                           fmc_error_t **error) {
  return ytp_control_reserve(&seq->ctrl, size, error);
}

ytp_iterator_t ytp_sequence_commit(ytp_sequence_t *seq, ytp_peer_t peer,
                                   ytp_channel_t channel, uint64_t time,
                                   void *data, fmc_error_t **error) {
  return ytp_control_commit(&seq->ctrl, peer, channel, time, data, error);
}

bool ytp_sequence_poll(ytp_sequence_t *seq, fmc_error_t **error) {
  return ytp_timeline_poll(&seq->timeline, error);
}

bool ytp_sequence_term(ytp_sequence_t *seq) {
  return ytp_timeline_term(&seq->timeline);
}

ytp_iterator_t ytp_sequence_end(ytp_sequence_t *seq, fmc_error_t **error) {
  return ytp_control_end(&seq->ctrl, error);
}

ytp_iterator_t ytp_sequence_cur(ytp_sequence_t *seq) {
  return ytp_timeline_iter_get(&seq->timeline);
}

ytp_iterator_t ytp_sequence_get_it(ytp_sequence_t *seq) {
  return ytp_timeline_iter_get(&seq->timeline);
}

void ytp_sequence_set_it(ytp_sequence_t *seq, ytp_iterator_t iterator) {
  ytp_timeline_iter_set(&seq->timeline, iterator);
}

void ytp_sequence_cb_rm(ytp_sequence_t *seq) {
  ytp_timeline_cb_rm(&seq->timeline);
}

ytp_iterator_t ytp_sequence_seek(ytp_sequence_t *seq, size_t off,
                                 fmc_error_t **error) {
  return ytp_timeline_seek(&seq->timeline, off, error);
}

size_t ytp_sequence_tell(ytp_sequence_t *seq, ytp_iterator_t iterator,
                         fmc_error_t **error) {
  return ytp_timeline_tell(&seq->timeline, iterator, error);
}

void ytp_sequence_close(ytp_sequence_t *seq, fmc_error_t **error) {
  ytp_control_close(&seq->ctrl, error);
}

bool ytp_sequence_closed(ytp_sequence_t *seq, fmc_error_t **error) {
  return ytp_control_closed(&seq->ctrl, error);
}

ytp_sequence_shared_t *ytp_sequence_shared_new(const char *filename,
                                               fmc_fmode mode,
                                               fmc_error_t **error) {
  fmc_error_clear(error);

  auto fd = fmc_fopen(filename, mode, error);
  if (*error) {
    return nullptr;
  }

  auto *shared_seq = new ytp_sequence_shared_t;
  ytp_sequence_init(&shared_seq->seq, fd, error);
  if (*error) {
    delete shared_seq;
    std::string tmp_error = fmc_error_msg(*error);
    fmc_fclose(fd, error);
    FMC_ERROR_REPORT(error, tmp_error.c_str());
    return nullptr;
  }

  shared_seq->fd = fd;
  return shared_seq;
}

void ytp_sequence_shared_inc(ytp_sequence_shared_t *shared_seq) {
  ++shared_seq->ref_counter;
}

void ytp_sequence_shared_dec(ytp_sequence_shared_t *shared_seq,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  if (--shared_seq->ref_counter == 0) {
    ytp_sequence_destroy(&shared_seq->seq, error);
    fmc_fclose(shared_seq->fd, error);
    delete shared_seq;
  }
}

ytp_sequence_t *ytp_sequence_shared_get(ytp_sequence_shared_t *shared_seq) {
  return &shared_seq->seq;
}
