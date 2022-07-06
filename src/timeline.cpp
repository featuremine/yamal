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

#include <apr.h> // apr_size_t APR_DECLARE
#include <stdbool.h> 
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <ytp/timeline.h>

#include <ytp/control.h>
#include <ytp/channel.h> // ytp_channel_t
#include <ytp/peer.h> // ytp_peer_t
#include <ytp/yamal.h> // ytp_yamal_term
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
                                 apr_size_t name_sz, const char *name_ptr) {
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
                                         apr_size_t sz, const char *name) {
  auto *seq = (ytp_timeline_t *)closure;
  channel_announcement(seq, peer, channel, time, sz, name);
}

static void channel_announcement_msg(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, uint64_t time,
                                     apr_size_t sz, const char *data) {
  ytp_timeline_t *timeline = (ytp_timeline_t *)closure;
  std::string_view channel_name{data, sz};
  auto it = timeline->ctrl->name_to_channel.find(channel_name);
  if(it == timeline->ctrl->name_to_channel.end()) {
    return;
  }
  ytp_channel_t ch = it->second;
  timeline->cb_ch.lock();
  for (auto it = timeline->cb_ch.begin(); it != timeline->cb_ch.end(); ++it) {
    if (it.was_removed()) {
      continue;
    }
    auto &c = *it;
    c.first(c.second, peer, ch, time, sz, data);
  }
  timeline->cb_ch.release();
}

APR_DECLARE(ytp_status_t) ytp_timeline_new(ytp_timeline_t **timeline, ytp_control_t *ctrl) {
  *timeline = new ytp_timeline_t;
  ytp_status_t rv = ytp_timeline_init(*timeline, ctrl);
  if(rv) {
    delete *timeline;
    *timeline = nullptr;
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_timeline_init(ytp_timeline_t *timeline, ytp_control_t *ctrl) {
  timeline->ctrl = ctrl;
  ytp_status_t rv = ytp_control_begin(ctrl, &(timeline->read));
  if(rv) {
    return rv;
  }
  ytp_timeline_ch_cb(timeline, channel_announcement_wrapper, timeline);
  ytp_timeline_indx_cb(timeline, YTP_CHANNEL_ANN, channel_announcement_msg,
                       timeline);
  return rv;
}

APR_DECLARE(void) ytp_timeline_destroy(ytp_timeline_t *timeline) {
  ytp_timeline_indx_cb_rm(timeline, YTP_CHANNEL_ANN, channel_announcement_msg,
                          timeline);
  ytp_timeline_ch_cb_rm(timeline, channel_announcement_wrapper, timeline);
}

APR_DECLARE(void) ytp_timeline_del(ytp_timeline_t *timeline) {
  ytp_timeline_destroy(timeline);
  delete timeline;
}

APR_DECLARE(void) ytp_timeline_ch_cb(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb, void *closure) {
  timeline->cb_ch.push_unique(ytp_timeline_ch_cb_cl_t(cb, closure));
}

APR_DECLARE(void) ytp_timeline_ch_cb_rm(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb, void *closure) {
  auto p = ytp_timeline_ch_cb_cl_t(cb, closure);
  auto &v = timeline->cb_ch;
  v.erase_if([&](const decltype(p) &item) { return p == item; });
}

APR_DECLARE(void) ytp_timeline_peer_cb(ytp_timeline_t *timeline, ytp_timeline_peer_cb_t cb, void *closure) {
  timeline->cb_peer.push_unique(ytp_timeline_peer_cb_cl_t(cb, closure));
}

APR_DECLARE(void) ytp_timeline_peer_cb_rm(ytp_timeline_t *timeline, ytp_timeline_peer_cb_t cb, void *closure) {
  auto p = ytp_timeline_peer_cb_cl_t(cb, closure);
  auto &v = timeline->cb_peer;
  v.erase_if([&](const decltype(p) &item) { return p == item; });
}

APR_DECLARE(void) ytp_timeline_prfx_cb(ytp_timeline_t *timeline, apr_size_t sz, const char *prfx, ytp_timeline_data_cb_t cb, void *closure) {
  std::string namestr(prfx, sz);
  auto c = ytp_timeline_data_cb_cl_t(cb, closure);
  timeline->prfx_cb[namestr].emplace_back(c);
  using V = decltype(timeline->idx_cb)::value_type;
  prfx_for_each(timeline, namestr, [&c](V &v) { v.push_unique(c); });
}

APR_DECLARE(void) ytp_timeline_prfx_cb_rm(ytp_timeline_t *timeline, apr_size_t sz, const char *prfx, ytp_timeline_data_cb_t cb, void *closure) {
  std::string namestr(prfx, sz);
  auto c = ytp_timeline_data_cb_cl_t(cb, closure);
  timeline->prfx_cb.erase(namestr);
  using V = decltype(timeline->idx_cb)::value_type;
  prfx_for_each(timeline, namestr, [&c](V &v) {
    v.erase_if([&](const decltype(c) &item) { return c == item; });
  });
}

APR_DECLARE(void) ytp_timeline_indx_cb(ytp_timeline_t *timeline, ytp_channel_t channel, ytp_timeline_data_cb_t cb, void *closure) {
  auto c = ytp_timeline_data_cb_cl_t(cb, closure);
  timeline->idx_cb[channel].push_unique(c);
}

APR_DECLARE(void) ytp_timeline_indx_cb_rm(ytp_timeline_t *timeline, ytp_channel_t channel, ytp_timeline_data_cb_t cb, void *closure) {
  auto p = ytp_timeline_data_cb_cl_t(cb, closure);
  if (auto it_data = timeline->idx_cb.find(channel);
      it_data != timeline->idx_cb.end()) {
    auto &v = it_data->second;
    v.erase_if([&](const decltype(p) &item) { return p == item; });
  }
}

APR_DECLARE(bool) ytp_timeline_term(ytp_timeline_t *timeline) {
  return ytp_yamal_term(timeline->read);
}

APR_DECLARE(ytp_iterator_t) ytp_timeline_iter_get(ytp_timeline_t *timeline) {
  return timeline->read;
}

APR_DECLARE(void) ytp_timeline_iter_set(ytp_timeline_t *timeline, ytp_iterator_t iterator) {
  timeline->read = iterator;
}

APR_DECLARE(ytp_status_t) ytp_timeline_seek(ytp_timeline_t *timeline, ytp_iterator_t *it_ptr, apr_size_t ptr) {
  ytp_status_t rv = ytp_control_seek(timeline->ctrl, it_ptr, ptr);
  timeline->read = *it_ptr;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_timeline_tell(ytp_timeline_t *timeline, apr_size_t *ptr, ytp_iterator_t iterator) {
  return ytp_control_tell(timeline->ctrl, ptr, iterator);
}

APR_DECLARE(void) ytp_timeline_idle_cb(ytp_timeline_t *timeline, ytp_timeline_idle_cb_t cb, void *closure) {
  auto c = ytp_timeline_idle_cb_cl_t(cb, closure);
  timeline->cb_idle.push_unique(c);
}

APR_DECLARE(void) ytp_timeline_idle_cb_rm(ytp_timeline_t *timeline,
                             ytp_timeline_idle_cb_t cb, void *closure) {
  auto p = ytp_timeline_idle_cb_cl_t(cb, closure);
  auto &v = timeline->cb_idle;
  v.erase_if([&](const decltype(p) &item) { return p == item; });
}

static bool was_announced(std::vector<uint8_t> &announced_vec, apr_size_t id) {
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

static bool was_announced(std::unordered_set<std::string_view> &announced_set,
                   std::string_view id) {
  if (auto it = announced_set.find(id); it != announced_set.end()) {
    return true;
  }
  announced_set.emplace(id);
  return false;
}

APR_DECLARE(ytp_status_t) ytp_timeline_poll(ytp_timeline_t *timeline, bool *new_data) {
  ytp_status_t rv = YTP_STATUS_OK;
  if (!ytp_timeline_term(timeline)) {
    ytp_peer_t read_peer;
    ytp_channel_t read_channel;
    uint64_t read_time;
    apr_size_t read_sz;
    const char *read_data;

    rv = ytp_control_read(timeline->ctrl, timeline->read, &read_peer, &read_channel,
                                       &read_time, &read_sz, &read_data);
    if (!rv) {
      ytp_iterator_t next_it;
      rv = ytp_control_next(timeline->ctrl, &next_it, timeline->read);
      if (rv) {
        *new_data = false;
        return rv;
      }
      timeline->read = next_it;
      if (ytp_peer_ann(read_peer)) {
        std::string_view peer_name{read_data, read_sz};

        ytp_peer_t peer;
        if (auto it = timeline->ctrl->name_to_peer.find(peer_name);
            it != timeline->ctrl->name_to_peer.end()) {
          peer = it->second;
        } else {
          *new_data = false;
          return YTP_STATUS_EPEERANN;
        }

        if (was_announced(timeline->peer_announced, peer - YTP_PEER_OFF)) {
          *new_data = true;
          return rv;
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
              *new_data = false;
              return YTP_STATUS_ECHANN;
            }

            if (was_announced(timeline->ch_announced,
                              channel - YTP_CHANNEL_OFF)) {
              *new_data = true;
              return rv;
            }
          }

          else if (read_channel == YTP_CHANNEL_SUB) {
            std::string_view prefix_name{read_data, read_sz};
            if (was_announced(timeline->sub_announced, prefix_name)) {
              *new_data = true;
              return rv;
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
      *new_data = true;
      return rv;
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

  *new_data = false;
  return rv;
}

APR_DECLARE(bool) ytp_timeline_consume(ytp_timeline_t *dest, ytp_timeline_t *src) {
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

APR_DECLARE(void) ytp_timeline_cb_rm(ytp_timeline_t *timeline) {
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
