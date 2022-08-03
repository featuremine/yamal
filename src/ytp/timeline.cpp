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

template <typename F>
static void prfx_for_each(ytp_timeline_t *timeline, const std::string &namestr,
                          const F &f) {
  if (namestr == "/") {
    for (auto &&it : timeline->ctrl->name_to_channel) {
      f(timeline->idx_cb[it.second]);
    }
  } else if (*namestr.rbegin() != '/') {
    if (auto it = timeline->ctrl->name_to_channel.find(namestr);
        it != timeline->ctrl->name_to_channel.end()) {
      f(timeline->idx_cb[it->second]);
    }
  } else {
    auto it = timeline->ctrl->name_to_channel.lower_bound(namestr);
    for (; it != timeline->ctrl->name_to_channel.end() &&
           it->first.substr(0, namestr.size()) == namestr;
         ++it) {
      f(timeline->idx_cb[it->second]);
    }
  }
}

static void channel_announcement(ytp_timeline_t *timeline, ytp_peer_t peer,
                                 ytp_channel_t channel, uint64_t read_time,
                                 size_t name_sz, const char *name_ptr,
                                 fmc_error_t **error) {
  auto &dest = timeline->idx_cb[channel];
  auto current_name_key = std::string(name_ptr, name_sz);
  if (auto it = timeline->prfx_cb.find("/"); it != timeline->prfx_cb.end()) {
    for (auto &e : it->second) {
      dest.push_unique(e);
    }
  }
  do {
    if (auto it = timeline->prfx_cb.find(current_name_key);
        it != timeline->prfx_cb.end()) {
      for (auto &e : it->second) {
        dest.push_unique(e);
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
                                         ytp_channel_t channel, uint64_t time,
                                         size_t sz, const char *name) {
  auto *seq = (ytp_timeline_t *)closure;
  fmc_error_t *error;
  channel_announcement(seq, peer, channel, time, sz, name, &error);
}

static void channel_announcement_msg(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, uint64_t time,
                                     size_t sz, const char *data) {
  auto *timeline = (ytp_timeline_t *)closure;
  fmc_error_t *error;
  channel = ytp_control_ch_decl(timeline->ctrl, peer, time, sz, data, &error);
  timeline->cb_ch.lock();
  for (auto it = timeline->cb_ch.begin(); it != timeline->cb_ch.end(); ++it) {
    if (it.was_removed()) {
      continue;
    }
    auto &c = *it;
    c.first(c.second, peer, channel, time, sz, data);
  }
  timeline->cb_ch.release();
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

  ytp_timeline_ch_cb(timeline, channel_announcement_wrapper, timeline, error);
  if (*error) {
    return;
  }

  ytp_timeline_indx_cb(timeline, YTP_CHANNEL_ANN, channel_announcement_msg,
                       timeline, error);
  if (*error) {
    return;
  }
}

void ytp_timeline_destroy(ytp_timeline_t *timeline, fmc_error_t **error) {
  ytp_timeline_indx_cb_rm(timeline, YTP_CHANNEL_ANN, channel_announcement_msg,
                          timeline, error);
  ytp_timeline_ch_cb_rm(timeline, channel_announcement_wrapper, timeline,
                        error);
}

void ytp_timeline_del(ytp_timeline_t *timeline, fmc_error_t **error) {
  ytp_timeline_destroy(timeline, error);
  delete timeline;
}

void ytp_timeline_ch_cb(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb,
                        void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  timeline->cb_ch.push_unique(ytp_timeline_ch_cb_cl_t(cb, closure));
}

void ytp_timeline_ch_cb_rm(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb,
                           void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_ch_cb_cl_t(cb, closure);
  auto &v = timeline->cb_ch;
  v.erase_if([&](const decltype(p) &item) { return p == item; });
}

void ytp_timeline_peer_cb(ytp_timeline_t *timeline, ytp_timeline_peer_cb_t cb,
                          void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  timeline->cb_peer.push_unique(ytp_timeline_peer_cb_cl_t(cb, closure));
}

void ytp_timeline_peer_cb_rm(ytp_timeline_t *timeline,
                             ytp_timeline_peer_cb_t cb, void *closure,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_peer_cb_cl_t(cb, closure);
  auto &v = timeline->cb_peer;
  v.erase_if([&](const decltype(p) &item) { return p == item; });
}

void ytp_timeline_prfx_cb(ytp_timeline_t *timeline, size_t sz, const char *prfx,
                          ytp_timeline_data_cb_t cb, void *closure,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  std::string namestr(prfx, sz);

  auto c = ytp_timeline_data_cb_cl_t(cb, closure);

  timeline->prfx_cb[namestr].emplace_back(c);

  using V = decltype(timeline->idx_cb)::value_type;
  prfx_for_each(timeline, namestr, [&c](V &v) { v.push_unique(c); });
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
    v.erase_if([&](const decltype(c) &item) { return c == item; });
  });
}

void ytp_timeline_indx_cb(ytp_timeline_t *timeline, ytp_channel_t channel,
                          ytp_timeline_data_cb_t cb, void *closure,
                          fmc_error_t **error) {
  fmc_error_clear(error);
  auto c = ytp_timeline_data_cb_cl_t(cb, closure);
  timeline->idx_cb[channel].push_unique(c);
}

void ytp_timeline_indx_cb_rm(ytp_timeline_t *timeline, ytp_channel_t channel,
                             ytp_timeline_data_cb_t cb, void *closure,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_data_cb_cl_t(cb, closure);

  if (auto it_data = timeline->idx_cb.find(channel);
      it_data != timeline->idx_cb.end()) {
    auto &v = it_data->second;
    v.erase_if([&](const decltype(p) &item) { return p == item; });
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
  timeline->cb_idle.push_unique(c);
}

void ytp_timeline_idle_cb_rm(ytp_timeline_t *timeline,
                             ytp_timeline_idle_cb_t cb, void *closure,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  auto p = ytp_timeline_idle_cb_cl_t(cb, closure);
  auto &v = timeline->cb_idle;
  v.erase_if([&](const decltype(p) &item) { return p == item; });
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
        if (auto it = timeline->ctrl->name_to_peer.find(peer_name);
            it != timeline->ctrl->name_to_peer.end()) {
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
        if (read_channel < YTP_CHANNEL_OFF) {
          if (read_channel == YTP_CHANNEL_ANN) {
            ytp_channel_t channel;
            std::string_view channel_name{read_data, read_sz};
            if (auto it = timeline->ctrl->name_to_channel.find(channel_name);
                it != timeline->ctrl->name_to_channel.end()) {
              channel = it->second;
            } else {
              fmc_error_set(error, "invalid channel announcement");
              return true;
            }

            if (was_announced(timeline->ch_announced,
                              channel - YTP_CHANNEL_OFF)) {
              return true;
            }
          }

          else if (read_channel == YTP_CHANNEL_SUB) {
            std::string_view prefix_name{read_data, read_sz};
            if (was_announced(timeline->sub_announced, prefix_name)) {
              return true;
            }
          }
        }

        if (auto it = timeline->idx_cb.find(read_channel);
            it != timeline->idx_cb.end()) {
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

  for (auto &&[channel, src_callbacks] : src->idx_cb) {
    auto &dest_callbacks = dest->idx_cb[channel];
    for (auto &&src_callback : src_callbacks) {
      if (src_callback.first != channel_announcement_msg) {
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
  for (auto &&[channel, v] : timeline->idx_cb) {
    v.erase_if([&](const ytp_timeline_data_cb_cl_t &item) {
      return item.first != channel_announcement_msg;
    });
  }

  {
    auto &v = timeline->cb_ch;
    v.erase_if([&](const ytp_timeline_ch_cb_cl_t &item) {
      return item.first != channel_announcement_wrapper;
    });
  }

  timeline->prfx_cb.clear();
  timeline->cb_peer.clear();
  timeline->cb_idle.clear();
}
