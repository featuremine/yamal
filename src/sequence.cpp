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

#include <stdbool.h> 
#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_file_io.h> // apr_file_t
#include <apr_pools.h> // apr_pool_t
#include <ytp/timeline.h> // ytp_timeline_t
#include <ytp/control.h> // ytp_control_t
#include <ytp/channel.h> // ytp_channel_t
#include <ytp/peer.h> // ytp_peer_t
#include <ytp/sequence.h>
#include <ytp/yamal.h> // ytp_yamal_term
#include <ytp/errno.h> // ytp_status_t
#include "control.hpp"
#include "timeline.hpp"

struct ytp_sequence {
  ytp_control_t ctrl;
  ytp_timeline_t timeline;
};

struct ytp_sequence_shared {
  uint64_t ref_counter = 1;
  apr_pool_t *pool;
  apr_file_t *f;
  ytp_sequence_t seq;
};

APR_DECLARE(ytp_status_t) ytp_sequence_new(ytp_sequence_t **seq, apr_file_t *f) {
  return ytp_sequence_new2(seq, f, true);
}

APR_DECLARE(ytp_status_t) ytp_sequence_init(ytp_sequence_t *seq, apr_file_t *f) {
  return ytp_sequence_init2(seq, f, true);
}

APR_DECLARE(ytp_status_t) ytp_sequence_new2(ytp_sequence_t **seq, apr_file_t *f, bool enable_thread) {
  *seq = new ytp_sequence_t;
  ytp_status_t rv = ytp_sequence_init2(*seq, f, enable_thread);
  if(rv) {
    delete *seq;
    *seq = nullptr;
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_sequence_init2(ytp_sequence_t *seq, apr_file_t *f, bool enable_thread) {
  ytp_status_t rv = ytp_control_init2(&seq->ctrl, f, enable_thread);
  if (rv) {
    return rv;
  }

  rv = ytp_timeline_init(&seq->timeline, &seq->ctrl);
  if(rv) {
    (void)ytp_control_destroy(&seq->ctrl);
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_sequence_del(ytp_sequence_t *seq) {
  ytp_status_t rv = ytp_sequence_destroy(seq);
  if(!rv) {
    delete seq;
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_sequence_destroy(ytp_sequence_t *seq) {
  ytp_timeline_destroy(&seq->timeline);
  return ytp_control_destroy(&seq->ctrl);
}

APR_DECLARE(ytp_status_t) ytp_sequence_peer_name(ytp_sequence_t *seq, ytp_peer_t peer, apr_size_t *sz, const char **name) {
  return ytp_control_peer_name(&seq->ctrl, peer, sz, name);
}

APR_DECLARE(ytp_status_t) ytp_sequence_peer_decl(ytp_sequence_t *seq, ytp_peer_t *peer, apr_size_t sz, const char *name) {
  return ytp_control_peer_decl(&seq->ctrl, peer, sz, name);
}

APR_DECLARE(void) ytp_sequence_peer_cb(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb, void *closure) {
  ytp_timeline_peer_cb(&seq->timeline, cb, closure);
}

APR_DECLARE(void) ytp_sequence_peer_cb_rm(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb, void *closure) {
  ytp_timeline_peer_cb_rm(&seq->timeline, cb, closure);
}

APR_DECLARE(ytp_status_t) ytp_sequence_ch_name(ytp_sequence_t *seq, ytp_channel_t channel, apr_size_t *sz, const char **name) {
  return ytp_control_ch_name(&seq->ctrl, channel, sz, name);
}

APR_DECLARE(ytp_status_t) ytp_sequence_ch_decl(ytp_sequence_t *seq, ytp_channel_t *channel, ytp_peer_t peer,
                                               uint64_t time, apr_size_t sz, const char *name) {
  return ytp_control_ch_decl(&seq->ctrl, channel, peer, time, sz, name);
}

APR_DECLARE(void) ytp_sequence_ch_cb(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb, void *closure) {
  ytp_timeline_ch_cb(&seq->timeline, cb, closure);
}

APR_DECLARE(void) ytp_sequence_ch_cb_rm(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb, void *closure) {
  ytp_timeline_ch_cb_rm(&seq->timeline, cb, closure);
}

APR_DECLARE(ytp_status_t) ytp_sequence_sub(ytp_sequence_t *seq, ytp_peer_t peer, uint64_t time,
                                           apr_size_t sz, const char *payload) {
  return ytp_control_sub(&seq->ctrl, peer, time, sz, payload);
}

APR_DECLARE(ytp_status_t) ytp_sequence_dir(ytp_sequence_t *seq, ytp_peer_t peer, uint64_t time,
                                           apr_size_t sz, const char *payload) {
  return ytp_control_dir(&seq->ctrl, peer, time, sz, payload);
}

APR_DECLARE(void) ytp_sequence_prfx_cb(ytp_sequence_t *seq, apr_size_t sz, const char *prfx,
                                       ytp_sequence_data_cb_t cb, void *closure) {
  ytp_timeline_prfx_cb(&seq->timeline, sz, prfx, cb, closure);
}

APR_DECLARE(void) ytp_sequence_prfx_cb_rm(ytp_sequence_t *seq, apr_size_t sz, const char *prfx,
                                          ytp_sequence_data_cb_t cb, void *closure) {
  ytp_timeline_prfx_cb_rm(&seq->timeline, sz, prfx, cb, closure);
}

APR_DECLARE(void) ytp_sequence_indx_cb(ytp_sequence_t *seq, ytp_channel_t channel,
                                       ytp_sequence_data_cb_t cb, void *closure) {
  ytp_timeline_indx_cb(&seq->timeline, channel, cb, closure);
}

APR_DECLARE(void) ytp_sequence_indx_cb_rm(ytp_sequence_t *seq, ytp_channel_t channel,
                                          ytp_sequence_data_cb_t cb, void *closure) {
  ytp_timeline_indx_cb_rm(&seq->timeline, channel, cb, closure);
}

APR_DECLARE(ytp_status_t) ytp_sequence_reserve(ytp_sequence_t *seq, char **buf, apr_size_t size) {
  return ytp_control_reserve(&seq->ctrl, buf, size);
}

APR_DECLARE(ytp_status_t) ytp_sequence_commit(ytp_sequence_t *seq, ytp_iterator_t *it, ytp_peer_t peer,
                                   ytp_channel_t channel, uint64_t time, void *data) {
  return ytp_control_commit(&seq->ctrl, it, peer, channel, time, data);
}

APR_DECLARE(ytp_status_t) ytp_sequence_poll(ytp_sequence_t *seq, bool *new_data) {
  return ytp_timeline_poll(&seq->timeline, new_data);
}

APR_DECLARE(bool) ytp_sequence_term(ytp_sequence_t *seq) {
  return ytp_timeline_term(&seq->timeline);
}

APR_DECLARE(ytp_status_t) ytp_sequence_end(ytp_sequence_t *seq, ytp_iterator_t *iterator) {
  return ytp_control_end(&seq->ctrl, iterator);
}

APR_DECLARE(ytp_iterator_t) ytp_sequence_get_it(ytp_sequence_t *seq) {
  return ytp_timeline_iter_get(&seq->timeline);
}

APR_DECLARE(void) ytp_sequence_set_it(ytp_sequence_t *seq, ytp_iterator_t iterator) {
  ytp_timeline_iter_set(&seq->timeline, iterator);
}

APR_DECLARE(ytp_status_t) ytp_sequence_seek(ytp_sequence_t *seq, ytp_iterator_t *it_ptr, apr_size_t ptr) {
  return ytp_timeline_seek(&seq->timeline, it_ptr, ptr);
}

APR_DECLARE(ytp_status_t) ytp_sequence_tell(ytp_sequence_t *seq, apr_size_t *ptr, ytp_iterator_t iterator) {
  return ytp_timeline_tell(&seq->timeline, ptr, iterator);
}

APR_DECLARE(ytp_status_t) ytp_sequence_shared_new(ytp_sequence_shared_t **shared_seq, const char *filename, apr_int32_t flag) {
  ytp_status_t rv = ytp_initialize();
  if (rv) {
    return rv;
  }

  *shared_seq = new ytp_sequence_shared_t;
  rv = apr_pool_create(&((*shared_seq)->pool), NULL);
  if (rv) {
    delete *shared_seq;
    *shared_seq = nullptr;
    ytp_terminate();
    return rv;
  }
  
  rv = apr_file_open(&((*shared_seq)->f), filename, flag, APR_UREAD | APR_UWRITE | APR_GREAD | APR_WREAD, (*shared_seq)->pool);
  if (rv) {
    apr_pool_destroy((*shared_seq)->pool);
    delete *shared_seq;
    *shared_seq = nullptr;
    ytp_terminate();
    return rv;
  }

  rv = ytp_sequence_init(&((*shared_seq)->seq), (*shared_seq)->f);
  if (rv) {
    (void)apr_file_close((*shared_seq)->f);
    apr_pool_destroy((*shared_seq)->pool);
    delete *shared_seq;
    *shared_seq = nullptr;
    ytp_terminate();
    return rv;
  }

  return rv;
}

APR_DECLARE(void) ytp_sequence_shared_inc(ytp_sequence_shared_t *shared_seq) {
  ++shared_seq->ref_counter;
}

APR_DECLARE(void) ytp_sequence_shared_dec(ytp_sequence_shared_t *shared_seq) {
  if (--shared_seq->ref_counter == 0) {
    (void)ytp_sequence_destroy(&(shared_seq->seq));
    (void)apr_file_close(shared_seq->f);
    apr_pool_destroy(shared_seq->pool);
    delete shared_seq;
    ytp_terminate();
  }
}

APR_DECLARE(ytp_sequence_t *) ytp_sequence_shared_get(ytp_sequence_shared_t *shared_seq) {
  return &shared_seq->seq;
}
