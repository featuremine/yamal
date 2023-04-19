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
#include <ytp/timeline.h>

#include "control.hpp"
#include "timeline.hpp"

static bool was_announced(std::vector<uint8_t> &announced_vec, size_t id) {
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

template<typename T>
static bool was_announced(T &announced_set, const typename T::key_type &key) {
  if (auto it = announced_set.find(key); it != announced_set.end()) {
    return true;
  }
  announced_set.emplace(key);
  return false;
}

static void stream_announcement(ytp_timeline_t *timeline, ytp_channel_t channel,
                                size_t name_sz, const char *name_ptr,
                                size_t encoding_sz, const char *encoding) {
}

static void stream_announcement_wrapper(void *closure, ytp_peer_t peer,
                                        ytp_channel_t channel, uint64_t msgtime,
                                        size_t chname_sz, const char *chname,
                                        size_t encoding_sz, const char *encoding) {
  auto *seq = (ytp_timeline_t *)closure;
  stream_announcement(seq, channel, chname_sz, chname, encoding_sz, encoding);
}

static void stream_announcement_msg(void *closure, ytp_peer_t peer,
                                    ytp_channel_t channel, uint64_t time,
                                    size_t sz, const char *data) {
  auto *timeline = (ytp_timeline_t *)closure;

  auto &hdr = *reinterpret_cast<const stream_announcement_msg_t *>(data);
  auto channel_name_sz = ye64toh(hdr.channel_name_sz);
  const char *encoding = data + channel_name_sz + sizeof(stream_announcement_msg_t);
  size_t encoding_sz = sz - channel_name_sz - sizeof(stream_announcement_msg_t);

  channel = timeline->ctrl->name_to_channelid.find({hdr.channel_name, channel_name_sz})->second;

  if (!was_announced(timeline->channel_announced, channel - YTP_CHANNEL_OFF)) {
    timeline->cb_ch.lock();
    for (auto it = timeline->cb_ch.begin(); it != timeline->cb_ch.end(); ++it) {
      if (it.was_removed()) {
        continue;
      }
      auto &c = *it;
      c.first(c.second, peer, channel, time, channel_name_sz, hdr.channel_name);
    }
    timeline->cb_ch.release();
  }

  timeline->cb_stream.lock();
  for (auto it = timeline->cb_stream.begin(); it != timeline->cb_stream.end(); ++it) {
    if (it.was_removed()) {
      continue;
    }
    auto &c = *it;
    c.first(c.second, peer, channel, time, channel_name_sz, hdr.channel_name, encoding_sz, encoding);
  }
  timeline->cb_stream.release();
}

static void sub_announcement_msg(void *closure, ytp_peer_t peer,
                                 ytp_channel_t channel, uint64_t time,
                                 size_t sz, const char *data) {
  auto *timeline = (ytp_timeline_t *)closure;

  channel = ytp_channel_t(ye64toh(*reinterpret_cast<const ytp_channel_t *>(data)));

  timeline->cb_sub.lock();
  for (auto it = timeline->cb_sub.begin(); it != timeline->cb_sub.end(); ++it) {
    if (it.was_removed()) {
      continue;
    }
    auto &c = *it;
    c.first(c.second, time, peer, channel);
  }
  timeline->cb_sub.release();
}

ytp_timeline_t *ytp_timeline_new(ytp_control_t *ctrl, fmc_error_t **error) {
  auto *timeline = new ytp_timeline_t;
  ytp_timeline_init(timeline, ctrl, error);
  if (*error) {
    delete timeline;
    return nullptr;
  }
  return timeline;
}

void ytp_timeline_init(ytp_timeline_t *timeline, ytp_control_t *ctrl,
                       fmc_error_t **error) {
  timeline->ctrl = ctrl;

  timeline->read = ytp_control_begin(ctrl, error);
  if (*error) {
    return;
  }

  ytp_timeline_stream_cb(timeline, stream_announcement_wrapper, timeline, error);
  if (*error) {
    return;
  }

  fmc::push_unique(timeline->data_cb[{{}, YTP_CHANNEL_ANN}],
      ytp_timeline_data_cb_cl_t(stream_announcement_msg, timeline));
  fmc::push_unique(timeline->data_cb[{{}, YTP_CHANNEL_SUB}],
      ytp_timeline_data_cb_cl_t(sub_announcement_msg, timeline));
}

void ytp_timeline_destroy(ytp_timeline_t *timeline, fmc_error_t **error) {
}

void ytp_timeline_del(ytp_timeline_t *timeline, fmc_error_t **error) {
  ytp_timeline_destroy(timeline, error);
  delete timeline;
}

void ytp_timeline_sub_cb(ytp_timeline_t *timeline, ytp_timeline_sub_cb_t cb,
                        void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc::push_unique(timeline->cb_sub, ytp_timeline_sub_cb_cl_t(cb, closure));
}

void ytp_timeline_sub_cb_rm(ytp_timeline_t *timeline, ytp_timeline_sub_cb_t cb,
                           void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_sub_cb_cl_t(cb, closure);
  auto &v = timeline->cb_sub;
  std::erase_if(v, [&](const decltype(p) &item) { return p == item; });
}

void ytp_timeline_ch_cb(ytp_timeline_t *timeline,
                        ytp_timeline_ch_cb_t cb, void *closure,
                        fmc_error_t **error) {
  fmc_error_clear(error);
  fmc::push_unique(timeline->cb_ch, ytp_timeline_ch_cb_cl_t(cb, closure));
}

void ytp_timeline_ch_cb_rm(ytp_timeline_t *timeline,
                           ytp_timeline_ch_cb_t cb, void *closure,
                           fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_ch_cb_cl_t(cb, closure);
  auto &v = timeline->cb_ch;
  std::erase_if(v, [&](const decltype(p) &item) { return p == item; });
}

void ytp_timeline_stream_cb(ytp_timeline_t *timeline, ytp_timeline_stream_cb_t cb,
                            void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc::push_unique(timeline->cb_stream, ytp_timeline_stream_cb_cl_t(cb, closure));
}

void ytp_timeline_stream_cb_rm(ytp_timeline_t *timeline, ytp_timeline_stream_cb_t cb,
                               void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_stream_cb_cl_t(cb, closure);
  auto &v = timeline->cb_stream;
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

void ytp_timeline_data_cb(ytp_timeline_t *timeline,
                          uint64_t time, ytp_peer_t peer,
                          ytp_channel_t channel,
                          ytp_timeline_data_cb_t cb,
                          void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  auto c = ytp_timeline_data_cb_cl_t(cb, closure);
  fmc::push_unique(timeline->data_cb[{peer, channel}], c);
  if (!timeline->ctrl->yamal.readonly_) {
    ytp_control_sub(timeline->ctrl, time, peer, channel, error);
  }
}

void ytp_timeline_data_cb_rm(ytp_timeline_t *timeline,
                             ytp_peer_t peer, ytp_channel_t channel,
                             ytp_timeline_data_cb_t cb,
                             void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_data_cb_cl_t(cb, closure);

  if (auto it_data = timeline->data_cb.find({peer, channel});
      it_data != timeline->data_cb.end()) {
    auto &v = it_data->second;
    std::erase_if(v, [&](const decltype(p) &item) { return p == item; });
  }
}

bool ytp_timeline_term(ytp_timeline_t *timeline) {
  return ytp_yamal_term(timeline->read);
}

ytp_iterator_t ytp_timeline_iter_get(ytp_timeline_t *timeline) {
  return timeline->read;
}

void ytp_timeline_iter_set(ytp_timeline_t *timeline, ytp_iterator_t iterator) {
  timeline->read = iterator;
}

ytp_iterator_t ytp_timeline_seek(ytp_timeline_t *timeline, size_t ptr,
                                 fmc_error_t **error) {
  auto it = ytp_control_seek(timeline->ctrl, ptr, error);
  timeline->read = it;
  return it;
}

size_t ytp_timeline_tell(ytp_timeline_t *timeline, ytp_iterator_t iterator,
                         fmc_error_t **error) {
  return ytp_control_tell(timeline->ctrl, iterator, error);
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

static bool ytp_timeline_poll_impl(ytp_timeline_t *timeline,
                                   fmc_error_t **error) {
  fmc_error_clear(error);

  if (!ytp_timeline_term(timeline)) {
    ytp_peer_t read_peer;
    ytp_channel_t read_channel;
    uint64_t read_time;
    size_t read_sz;
    const char *read_data;

    ytp_control_read(timeline->ctrl, timeline->read, &read_peer, &read_channel,
                     &read_time, &read_sz, &read_data, error);
    if (!*error) {
      auto next_it = ytp_control_next(timeline->ctrl, timeline->read, error);
      if (*error) {
        return false;
      }
      timeline->read = next_it;
      if (ytp_peer_ann(read_peer)) {
        std::string_view peer_name{read_data, read_sz};

        ytp_peer_t peer;
        if (auto it = timeline->ctrl->name_to_peerid.find(peer_name);
            it != timeline->ctrl->name_to_peerid.end()) {
          peer = it->second;
        } else {
          fmc_error_set(error, "invalid peer announcement");
          return true;
        }

        if (was_announced(timeline->peer_announced, peer - YTP_PEER_OFF)) {
          return true;
        }

        timeline->cb_peer.lock();
        for (auto it = timeline->cb_peer.begin(); it != timeline->cb_peer.end();
             ++it) {
          if (it.was_removed()) {
            continue;
          }
          auto &c = *it;
          c.first(c.second, peer, peer_name.size(), peer_name.data());
        }
        timeline->cb_peer.release();
      } else {
        stream_key key = {read_peer, read_channel};
        if (read_channel < YTP_CHANNEL_OFF) {
          if (read_channel == YTP_CHANNEL_ANN) {
            ytp_channel_t channel;
            auto &hdr = *reinterpret_cast<const stream_announcement_msg_t *>(read_data);
            auto channel_name_sz = ye64toh(hdr.channel_name_sz);
            auto channel_name = std::string_view(hdr.channel_name, channel_name_sz);
            if (auto it = timeline->ctrl->name_to_channelid.find(channel_name);
                it != timeline->ctrl->name_to_channelid.end()) {
              channel = it->second;
            } else {
              fmc_error_set(error, "invalid channel announcement");
              return true;
            }

            if (was_announced(timeline->stream_announced, stream_key{read_peer, channel})) {
              return true;
            }
          }

          else if (read_channel == YTP_CHANNEL_SUB) {
            std::string_view prefix_name{read_data, read_sz};
            if (was_announced(timeline->sub_announced, prefix_name)) {
              return true;
            }
          }

          key.first = ytp_peer_t{};
        }

        if (auto it = timeline->data_cb.find(key);
            it != timeline->data_cb.end()) {
          it->second.lock();
          for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (it2.was_removed()) {
              continue;
            }
            auto &c = *it2;
            c.first(c.second, read_peer, read_channel, read_time, read_sz,
                    read_data);
          }
          it->second.release();
        }
      }
      return true;
    }
  }

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

  return false;
}

bool ytp_timeline_poll(ytp_timeline_t *timeline, fmc_error_t **error) {
  return ytp_timeline_poll_impl(timeline, error);
}

bool ytp_timeline_consume(ytp_timeline_t *dest, ytp_timeline_t *src) {
  if (dest->read != src->read) {
    return false;
  }

  for (auto &&[channel, src_callbacks] : src->data_cb) {
    auto &dest_callbacks = dest->data_cb[channel];
    for (auto &&src_callback : src_callbacks) {
      if (src_callback.first != stream_announcement_msg) {
        dest_callbacks.emplace_back(src_callback);
      }
    }
  }

  for (auto &&[prfx, src_callbacks] : src->prfx_cb) {
    auto &dest_callbacks = dest->prfx_cb[prfx];
    for (auto &&src_callback : src_callbacks) {
      dest_callbacks.emplace_back(src_callback);
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
  for (auto &&[channel, v] : timeline->data_cb) {
    std::erase_if(v, [&](const ytp_timeline_data_cb_cl_t &item) {
      return item.first != stream_announcement_msg;
    });
  }

  timeline->prfx_cb.clear();
  timeline->cb_peer.clear();
  timeline->cb_idle.clear();
}
