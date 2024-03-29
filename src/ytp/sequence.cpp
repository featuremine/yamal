/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include "control.hpp"
#include "timeline.hpp"
#include "yamal.hpp"

#include <fmc++/error.hpp>

#include <ytp/control.h>
#include <ytp/sequence.h>
#include <ytp/streams.h>
#include <ytp/yamal.h>

struct ytp_sequence {
  ytp_sequence(fmc_fd fd, bool enable_thread);
  ytp_control_t ctrl;
  ytp_timeline_t timeline;
};

struct initdestroy_t {
  void init(fmc_fd &fd, const char *path, fmc_fmode mode) {
    fmc_error_t *error;
    fd = fmc_fopen(path, mode, &error);
    if (error) {
      throw fmc::error(*error);
    }
  }
  void destroy(fmc_fd fd) {
    if (fd != -1) {
      fmc_error_t *error;
      fmc_fclose(fd, &error);
      if (error) {
        throw fmc::error(*error);
      }
    }
  }
};

template <typename T, typename InitDestroy> struct scopevar_t {
  template <typename... Args> scopevar_t(Args &&...args) {
    InitDestroy().init(value, std::forward<Args>(args)...);
  }
  ~scopevar_t() { InitDestroy().destroy(value); }
  scopevar_t(const scopevar_t &) = delete;
  T value;
};

using file_ptr = scopevar_t<fmc_fd, initdestroy_t>;

struct ytp_sequence_shared {
  ytp_sequence_shared(const char *filename, fmc_fmode mode);
  uint64_t ref_counter = 1;
  file_ptr file;
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
  auto *sequence =
      static_cast<ytp_sequence_t *>(malloc(sizeof(ytp_sequence_t)));
  if (!sequence) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return {};
  }

  ytp_sequence_init_2(sequence, fd, enable_thread, error);
  if (*error) {
    free(sequence);
    return {};
  }

  return sequence;
}

void ytp_sequence_init_2(ytp_sequence_t *seq, fmc_fd fd, bool enable_thread,
                         fmc_error_t **error) {
  try {
    new (seq) ytp_sequence(fd, enable_thread);
    fmc_error_clear(error);
  } catch (fmc::error &e) {
    *error = fmc_error_inst();
    fmc_error_mov(*error, &e);
  }
}

ytp_sequence::ytp_sequence(fmc_fd fd, bool enable_thread)
    : ctrl(fd, enable_thread), timeline(&ctrl) {}

void ytp_sequence_del(ytp_sequence_t *seq, fmc_error_t **error) {
  ytp_sequence_destroy(seq, error);
  if (error) {
    return;
  }

  free(seq);
}

void ytp_sequence_destroy(ytp_sequence_t *seq, fmc_error_t **error) {
  fmc_error_clear(error);
  seq->~ytp_sequence();
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
                                   int64_t ts, size_t sz, const char *name,
                                   fmc_error_t **error) {
  return ytp_control_ch_decl(&seq->ctrl, peer, ts, sz, name, error);
}

void ytp_sequence_ch_cb(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb,
                        void *closure, fmc_error_t **error) {
  ytp_timeline_ch_cb(&seq->timeline, cb, closure, error);
}

void ytp_sequence_ch_cb_rm(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb,
                           void *closure, fmc_error_t **error) {
  ytp_timeline_ch_cb_rm(&seq->timeline, cb, closure, error);
}

void ytp_sequence_sub(ytp_sequence_t *seq, ytp_peer_t peer, int64_t ts,
                      size_t sz, const char *payload, fmc_error_t **error) {
  ytp_control_sub(&seq->ctrl, peer, ts, sz, payload, error);
}

void ytp_sequence_dir(ytp_sequence_t *seq, ytp_peer_t peer, int64_t ts,
                      size_t sz, const char *payload, fmc_error_t **error) {
  ytp_control_dir(&seq->ctrl, peer, ts, sz, payload, error);
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
                                   ytp_channel_t channel, int64_t ts,
                                   void *data, fmc_error_t **error) {
  return ytp_control_commit(&seq->ctrl, peer, channel, ts, data, error);
}

void ytp_sequence_sublist_commit(ytp_sequence_t *seq, ytp_peer_t peer,
                                 ytp_channel_t channel, int64_t ts,
                                 void **first_ptr, void **last_ptr,
                                 void *new_ptr, fmc_error_t **error) {
  ytp_control_sublist_commit(&seq->ctrl, peer, channel, ts, first_ptr, last_ptr,
                             new_ptr, error);
}

ytp_iterator_t ytp_sequence_sublist_finalize(ytp_sequence_t *seq,
                                             void *first_ptr,
                                             fmc_error_t **error) {
  return ytp_control_sublist_finalize(&seq->ctrl, first_ptr, error);
}

bool ytp_sequence_poll(ytp_sequence_t *seq, fmc_error_t **error) {
  return ytp_timeline_poll(&seq->timeline, error);
}

ytp_iterator_t ytp_sequence_end(ytp_sequence_t *seq, fmc_error_t **error) {
  return ytp_yamal_end(&seq->ctrl.yamal, YTP_STREAM_LIST_DATA, error);
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

ytp_iterator_t ytp_sequence_seek(ytp_sequence_t *seq, ytp_mmnode_offs offset,
                                 fmc_error_t **error) {
  return ytp_timeline_seek(&seq->timeline, offset, error);
}

ytp_mmnode_offs ytp_sequence_tell(ytp_sequence_t *seq, ytp_iterator_t iterator,
                                  fmc_error_t **error) {
  return ytp_timeline_tell(&seq->timeline, iterator, error);
}

ytp_sequence_shared::ytp_sequence_shared(const char *filename, fmc_fmode mode)
    : file(filename, mode), seq(file.value, true) {}

ytp_sequence_shared_t *ytp_sequence_shared_new(const char *filename,
                                               fmc_fmode mode,
                                               fmc_error_t **error) {
  try {
    auto seq = std::make_unique<ytp_sequence_shared>(filename, mode);
    fmc_error_clear(error);
    return seq.release();
  } catch (fmc::error &e) {
    *error = fmc_error_inst();
    fmc_error_mov(*error, &e);
    return {};
  }
}

void ytp_sequence_shared_inc(ytp_sequence_shared_t *shared_seq) {
  ++shared_seq->ref_counter;
}

void ytp_sequence_shared_dec(ytp_sequence_shared_t *shared_seq,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  if (--shared_seq->ref_counter == 0) {
    shared_seq->~ytp_sequence_shared();
    free(shared_seq);
  }
}

ytp_sequence_t *ytp_sequence_shared_get(ytp_sequence_shared_t *shared_seq) {
  return &shared_seq->seq;
}
