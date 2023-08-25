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

#include <string>
#include <unordered_map>
#include <vector>

#include <ytp/announcement.h>
#include <ytp/data.h>
#include <ytp/timeline.h>

#include <fmc++/error.hpp>

#include "control.hpp"
#include "timeline.hpp"

template <typename F>
static void prfx_for_each(ytp_timeline_t *timeline, const std::string &namestr,
                          const F &f) {
  if (namestr == "/") {
    for (auto &&it : timeline->ctrl->name_to_channelid) {
      f(timeline->idx_cb[it.second]);
    }
  } else if (*namestr.rbegin() != '/') {
    if (auto it = timeline->ctrl->name_to_channelid.find(namestr);
        it != timeline->ctrl->name_to_channelid.end()) {
      f(timeline->idx_cb[it->second]);
    }
  } else {
    auto it = timeline->ctrl->name_to_channelid.lower_bound(namestr);
    for (; it != timeline->ctrl->name_to_channelid.end() &&
           it->first.substr(0, namestr.size()) == namestr;
         ++it) {
      f(timeline->idx_cb[it->second]);
    }
  }
}

static void channel_announcement(ytp_timeline_t *timeline, ytp_peer_t peer,
                                 ytp_channel_t channel, int64_t read_time,
                                 size_t name_sz, const char *name_ptr,
                                 fmc_error_t **error) {
  auto &dest = timeline->idx_cb[channel];
  auto current_name_key = std::string(name_ptr, name_sz);
  if (auto it = timeline->prfx_cb.find("/"); it != timeline->prfx_cb.end()) {
    for (auto &e : it->second) {
      fmc::push_unique(dest, e);
    }
  }
  do {
    if (auto it = timeline->prfx_cb.find(current_name_key);
        it != timeline->prfx_cb.end()) {
      for (auto &e : it->second) {
        fmc::push_unique(dest, e);
      }
    }
    if (current_name_key.size() < 2) {
      break;
    }
    auto pos = current_name_key.rfind('/', current_name_key.size() - 2);
    if (pos == std::string::npos) {
      break;
    }
    current_name_key.resize(pos + 1);
  } while (true);
}

static void channel_announcement_wrapper(void *closure, ytp_peer_t peer,
                                         ytp_channel_t channel, uint64_t ts,
                                         size_t sz, const char *name) {
  auto *seq = (ytp_timeline_t *)closure;
  fmc_error_t *error;
  channel_announcement(seq, peer, channel, ts, sz, name, &error);
}

ytp_timeline::ytp_timeline(ytp_control_t *ctrl) : ctrl(ctrl), ann_processed(0) {
  fmc_error_t *error;

  it_data = ytp_data_begin(&ctrl->yamal, &error);
  if (error) {
    throw fmc::error(*error);
  }

  it_ann = ytp_announcement_begin(&ctrl->yamal, &error);
  if (error) {
    throw fmc::error(*error);
  }

  ytp_timeline_ch_cb(this, channel_announcement_wrapper, this, &error);
  if (error) {
    throw fmc::error(*error);
  }
}

ytp_timeline_t *ytp_timeline_new(ytp_control_t *ctrl, fmc_error_t **error) {
  auto *timeline = static_cast<ytp_timeline_t *>(
      aligned_alloc(alignof(ytp_timeline_t), sizeof(ytp_timeline_t)));
  if (!timeline) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return {};
  }

  ytp_timeline_init(timeline, ctrl, error);
  if (*error) {
    free(timeline);
    return {};
  }

  return timeline;
}

void ytp_timeline_init(ytp_timeline_t *timeline, ytp_control_t *ctrl,
                       fmc_error_t **error) {
  try {
    new (timeline) ytp_timeline(ctrl);
    fmc_error_clear(error);
  } catch (fmc::error &e) {
    *error = fmc_error_inst();
    fmc_error_mov(*error, &e);
  }
}

void ytp_timeline_destroy(ytp_timeline_t *timeline, fmc_error_t **error) {
  fmc_error_clear(error);
  timeline->~ytp_timeline();
}

void ytp_timeline_del(ytp_timeline_t *timeline, fmc_error_t **error) {
  ytp_timeline_destroy(timeline, error);
  if (error) {
    return;
  }

  free(timeline);
}

void ytp_timeline_ch_cb(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb,
                        void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc::push_unique(timeline->cb_ch, ytp_timeline_ch_cb_cl_t(cb, closure));
}

void ytp_timeline_ch_cb_rm(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb,
                           void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_ch_cb_cl_t(cb, closure);
  auto &v = timeline->cb_ch;

  std::erase_if(v, [&](const decltype(p) &item) { return p == item; });
}

void ytp_timeline_peer_cb(ytp_timeline_t *timeline, ytp_timeline_peer_cb_t cb,
                          void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc::push_unique(timeline->cb_peer, ytp_timeline_peer_cb_cl_t(cb, closure));
}

void ytp_timeline_peer_cb_rm(ytp_timeline_t *timeline,
                             ytp_timeline_peer_cb_t cb, void *closure,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_peer_cb_cl_t(cb, closure);
  auto &v = timeline->cb_peer;
  std::erase_if(v, [&](const decltype(p) &item) { return p == item; });
}

void ytp_timeline_prfx_cb(ytp_timeline_t *timeline, size_t sz, const char *prfx,
                          ytp_timeline_data_cb_t cb, void *closure,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  std::string namestr(prfx, sz);

  auto c = ytp_timeline_data_cb_cl_t(cb, closure);

  timeline->prfx_cb[namestr].emplace_back(c);

  using V = decltype(timeline->idx_cb)::value_type;
  prfx_for_each(timeline, namestr, [&c](V &v) { fmc::push_unique(v, c); });
}

void ytp_timeline_prfx_cb_rm(ytp_timeline_t *timeline, size_t sz,
                             const char *prfx, ytp_timeline_data_cb_t cb,
                             void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  std::string namestr(prfx, sz);

  auto c = ytp_timeline_data_cb_cl_t(cb, closure);

  timeline->prfx_cb.erase(namestr);

  using V = decltype(timeline->idx_cb)::value_type;
  prfx_for_each(timeline, namestr, [&c](V &v) {
    std::erase_if(v, [&](const decltype(c) &item) { return c == item; });
  });
}

void ytp_timeline_indx_cb(ytp_timeline_t *timeline, ytp_channel_t channel,
                          ytp_timeline_data_cb_t cb, void *closure,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  auto c = ytp_timeline_data_cb_cl_t(cb, closure);
  fmc::push_unique(timeline->idx_cb[channel], c);
}

void ytp_timeline_indx_cb_rm(ytp_timeline_t *timeline, ytp_channel_t channel,
                             ytp_timeline_data_cb_t cb, void *closure,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_data_cb_cl_t(cb, closure);

  if (auto it_data = timeline->idx_cb.find(channel);
      it_data != timeline->idx_cb.end()) {
    auto &v = it_data->second;
    std::erase_if(v, [&](const decltype(p) &item) { return p == item; });
  }
}

bool ytp_timeline_term(ytp_timeline_t *timeline) {
  if (!ytp_yamal_term(timeline->it_data)) {
    return false;
  }

  fmc_error_t *error;
  auto ann_term =
      ytp_announcement_term(&timeline->ctrl->yamal, timeline->it_data, &error);
  return ann_term || error;
}

ytp_iterator_t ytp_timeline_iter_get(ytp_timeline_t *timeline) {
  return timeline->it_data;
}

void ytp_timeline_iter_set(ytp_timeline_t *timeline, ytp_iterator_t iterator) {
  timeline->it_data = iterator;
}

ytp_iterator_t ytp_timeline_seek(ytp_timeline_t *timeline, ytp_mmnode_offs ptr,
                                 fmc_error_t **error) {
  auto it = ytp_yamal_seek(&timeline->ctrl->yamal, ptr, error);
  if (*error) {
    return NULL;
  }

  timeline->it_data = it;
  return it;
}

ytp_mmnode_offs ytp_timeline_tell(ytp_timeline_t *timeline,
                                  ytp_iterator_t iterator,
                                  fmc_error_t **error) {
  return ytp_yamal_tell(&timeline->ctrl->yamal, iterator, error);
}

void ytp_timeline_idle_cb(ytp_timeline_t *timeline, ytp_timeline_idle_cb_t cb,
                          void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  auto c = ytp_timeline_idle_cb_cl_t(cb, closure);
  fmc::push_unique(timeline->cb_idle, c);
}

void ytp_timeline_idle_cb_rm(ytp_timeline_t *timeline,
                             ytp_timeline_idle_cb_t cb, void *closure,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_idle_cb_cl_t(cb, closure);
  auto &v = timeline->cb_idle;
  std::erase_if(v, [&](const decltype(p) &item) { return p == item; });
}

bool was_announced(std::vector<uint8_t> &announced_vec, size_t id) {
  if (id >= announced_vec.size()) {
    announced_vec.resize(id + 1);
  }

  auto &announced = announced_vec[id];
  if (announced) {
    return true;
  }
  announced = 1;

  return false;
}

bool was_announced(std::unordered_set<std::string_view> &announced_set,
                   std::string_view id) {
  if (auto it = announced_set.find(id); it != announced_set.end()) {
    return true;
  }
  announced_set.emplace(id);
  return false;
}

static bool ytp_timeline_poll_ann(ytp_timeline_t *timeline,
                                  fmc_error_t **error) {
  uint64_t seqno;
  ytp_mmnode_offs stream;
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
  size_t esz;
  const char *encoding;
  ytp_mmnode_offs *original;
  ytp_mmnode_offs *subscribed;
  auto has_ann = ytp_announcement_next(
      &timeline->ctrl->yamal, &timeline->it_ann, &seqno, &stream, &psz, &peer,
      &csz, &channel, &esz, &encoding, &original, &subscribed, error);
  if (!has_ann || *error) {
    return false;
  }

  ytp_control_poll_until(timeline->ctrl, seqno, error);
  if (*error) {
    return false;
  }

  std::string_view peer_name{peer, psz};
  std::string_view channel_name{channel, csz};

  ytp_peer_t peerid;
  ytp_channel_t channelid;

  if (auto it = timeline->ctrl->name_to_peerid.find(peer_name);
      it != timeline->ctrl->name_to_peerid.end()) {
    peerid = it->second;
  } else {
    fmc_error_set(error, "invalid peer announcement");
    return false;
  }

  if (auto it = timeline->ctrl->name_to_channelid.find(channel_name);
      it != timeline->ctrl->name_to_channelid.end()) {
    channelid = it->second;
  } else {
    fmc_error_set(error, "invalid channel announcement");
    return false;
  }

  if (!was_announced(timeline->peer_announced, peerid - YTP_PEER_OFF)) {
    timeline->cb_peer.lock();
    for (auto it = timeline->cb_peer.begin(); it != timeline->cb_peer.end();
         ++it) {
      if (it.was_removed()) {
        continue;
      }
      auto &c = *it;
      c.first(c.second, peerid, peer_name.size(), peer_name.data());
    }
    timeline->cb_peer.release();
  }

  if (channel_name != "" &&
      !was_announced(timeline->ch_announced, channelid - YTP_CHANNEL_OFF)) {
    timeline->cb_ch.lock();
    for (auto it = timeline->cb_ch.begin(); it != timeline->cb_ch.end(); ++it) {
      if (it.was_removed()) {
        continue;
      }
      auto &c = *it;
      c.first(c.second, peerid, channelid, {}, channel_name.size(),
              channel_name.data());
    }
    timeline->cb_ch.release();
  }

  timeline->ann_processed = seqno;
  return true;
}

static bool ytp_timeline_poll_data(ytp_timeline_t *timeline,
                                   fmc_error_t **error) {
  fmc_error_clear(error);

  if (ytp_yamal_term(timeline->it_data)) {
    return false;
  }

  uint64_t seqno;
  int64_t ts;
  ytp_mmnode_offs stream;
  size_t sz;
  const char *data;
  ytp_data_read(&timeline->ctrl->yamal, timeline->it_data, &seqno, &ts, &stream,
                &sz, &data, error);
  if (*error) {
    return false;
  }

  uint64_t stream_seqno;
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
  size_t esz;
  const char *encoding;
  ytp_mmnode_offs *original;
  ytp_mmnode_offs *subscribed;
  ytp_announcement_lookup(&timeline->ctrl->yamal, stream, &stream_seqno, &psz,
                          &peer, &csz, &channel, &esz, &encoding, &original,
                          &subscribed, error);
  if (timeline->ann_processed < stream_seqno) {
    bool polled = ytp_timeline_poll_ann(timeline, error);
    if (!*error && !polled) {
      fmc_error_set(error, "data message is using an invalid stream id");
    }
    return polled;
  }

  ytp_control_poll_until(timeline->ctrl, stream_seqno, error);
  if (*error) {
    return false;
  }

  auto stream_it = timeline->ctrl->streams.find(stream);
  if (stream_it == timeline->ctrl->streams.end()) {
    fmc_error_set(error, "data message is using an invalid stream id");
    return false;
  }

  auto next = ytp_yamal_next(&timeline->ctrl->yamal, timeline->it_data, error);
  if (*error) {
    return false;
  }

  timeline->it_data = next;

  auto &stream_data = stream_it->second;
  if (auto it = timeline->idx_cb.find(stream_data.channel);
      it != timeline->idx_cb.end()) {
    it->second.lock();
    for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
      if (it2.was_removed()) {
        continue;
      }
      auto &c = *it2;
      c.first(c.second, stream_data.peer, stream_data.channel, ts, sz, data);
    }
    it->second.release();
  }

  return true;
}

static bool ytp_timeline_poll_noidle(ytp_timeline_t *timeline,
                                     fmc_error_t **error) {
  auto ann = ytp_timeline_poll_ann(timeline, error);
  if (ann || *error) {
    return ann;
  }

  return ytp_timeline_poll_data(timeline, error);
}

static bool ytp_timeline_poll_until_noidle(ytp_timeline_t *timeline,
                                           const ytp_timeline_t *src_timeline,
                                           fmc_error_t **error) {
  fmc_error_clear(error);

  if (timeline->it_ann == src_timeline->it_ann) {
    return ytp_timeline_poll_data(timeline, error);
  }

  return ytp_timeline_poll(timeline, error);
}

static void ytp_timeline_poll_idle(ytp_timeline_t *timeline) {
  timeline->cb_idle.lock();
  for (auto it = timeline->cb_idle.begin(); it != timeline->cb_idle.end();
       ++it) {
    if (it.was_removed()) {
      continue;
    }
    auto &c = *it;
    c.first(c.second);
  }
  timeline->cb_idle.release();
}

bool ytp_timeline_poll(ytp_timeline_t *timeline, fmc_error_t **error) {
  if (ytp_timeline_poll_noidle(timeline, error)) {
    return true;
  }
  if (*error) {
    return false;
  }
  ytp_timeline_poll_idle(timeline);
  return false;
}

bool ytp_timeline_poll_until(ytp_timeline_t *timeline,
                             const ytp_timeline_t *src_timeline,
                             fmc_error_t **error) {
  if (ytp_timeline_poll_until_noidle(timeline, src_timeline, error)) {
    return true;
  }
  if (*error) {
    return false;
  }
  ytp_timeline_poll_idle(timeline);
  return false;
}

bool ytp_timeline_consume(ytp_timeline_t *dest, ytp_timeline_t *src) {
  if (dest->it_data != src->it_data || dest->it_ann != src->it_ann) {
    return false;
  }

  for (auto &&[channel, src_callbacks] : src->idx_cb) {
    auto &dest_callbacks = dest->idx_cb[channel];
    for (auto &&src_callback : src_callbacks) {
      dest_callbacks.emplace_back(src_callback);
    }
  }

  for (auto &&[prfx, src_callbacks] : src->prfx_cb) {
    auto &dest_callbacks = dest->prfx_cb[prfx];
    for (auto &&src_callback : src_callbacks) {
      dest_callbacks.emplace_back(src_callback);
    }
  }

  {
    auto &src_callbacks = src->cb_ch;
    auto &dest_callbacks = dest->cb_ch;
    for (auto &&src_callback : src_callbacks) {
      if (src_callback.first != channel_announcement_wrapper) {
        dest_callbacks.emplace_back(src_callback);
      }
    }
  }

  {
    auto &src_callbacks = src->cb_peer;
    auto &dest_callbacks = dest->cb_peer;
    for (auto &&src_callback : src_callbacks) {
      dest_callbacks.emplace_back(src_callback);
    }
  }

  {
    auto &src_callbacks = src->cb_idle;
    auto &dest_callbacks = dest->cb_idle;
    for (auto &&src_callback : src_callbacks) {
      dest_callbacks.emplace_back(src_callback);
    }
  }

  ytp_timeline_cb_rm(src);

  return true;
}

void ytp_timeline_cb_rm(ytp_timeline_t *timeline) {
  {
    auto &v = timeline->cb_ch;
    std::erase_if(v, [&](const ytp_timeline_ch_cb_cl_t &item) {
      return item.first != channel_announcement_wrapper;
    });
  }

  timeline->idx_cb.clear();
  timeline->prfx_cb.clear();
  timeline->cb_peer.clear();
  timeline->cb_idle.clear();
}
