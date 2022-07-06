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

#include <string_view>
#include <cstring> // std::memcpy
#include <map>
#include <unordered_map>
#include <stdbool.h> 

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_file_io.h> // apr_file_t
#include <ytp/channel.h>
#include <ytp/control.h>
#include <ytp/time.h>
#include <ytp/yamal.h>
#include <ytp/errno.h> // ytp_status_t

#include "control.hpp"

static peer_data &process_peer(ytp_control_t *ctrl, std::string_view name) {
  ytp_channel_t id = ctrl->name_to_peer.size() + YTP_PEER_OFF;

  peer_data *peer;
  auto it = ctrl->name_to_peer.emplace(name, id);

  if (it.second) {
    peer = &ctrl->peer_map[id];
    peer->name = name;
  } else {
    peer = &ctrl->peer_map[it.first->second];
  }

  return *peer;
}

static channel_data &process_channel(ytp_control *ctrl, ytp_peer_t peer,
                                     std::string_view name) {
  ytp_channel_t id = ctrl->name_to_channel.size() + YTP_CHANNEL_OFF;

  channel_data *channel;
  auto it = ctrl->name_to_channel.emplace(name, id);

  if (it.second) {
    channel = &ctrl->channel_map[id];
    channel->name = name;
  } else {
    channel = &ctrl->channel_map[it.first->second];
  }

  return *channel;
}

static void process_dir(ytp_control *ctrl, ytp_peer_t peer,
                        std::string_view dir) {}

static sub_data &process_sub(ytp_control *ctrl, ytp_peer_t read_peer,
                             std::string_view payload) {
  return ctrl->subs_announced.emplace(payload, sub_data{}).first->second;
}

template <typename F>
static ytp_status_t process_control_msgs(ytp_control_t *ctrl, bool *ret, const F &found) {
  ytp_peer_t peer;
  ytp_channel_t channel;
  uint64_t time;
  apr_size_t sz;
  const char *data;
  ytp_status_t rv = YTP_STATUS_OK;

  bool f;
  while (!(f = found()) && !ytp_yamal_term(ctrl->ctrl)) {
    rv = ytp_control_read(ctrl, ctrl->ctrl, &peer, &channel, &time, &sz, &data);
    if(rv) {
      *ret = false;
      return rv;
    }
    ytp_iterator_t new_it;
    rv = ytp_control_next(ctrl, &new_it, ctrl->ctrl);
    if(rv) {
      *ret = false;
      return rv;
    }
    ctrl->ctrl = new_it;
  }
  *ret = f;
  return rv;
}

template <typename F, typename I>
static ytp_status_t lookup_or_insert_ctrl_msg(ytp_control_t *ctrl, bool *ret, const F &found, const I &insert) {
  ytp_status_t rv = process_control_msgs(ctrl, ret, found);
  if(rv) {
    return rv;
  }
  if (!(*ret)) {
    rv = insert();
    if(rv) {
      return rv;
    }
    return process_control_msgs(ctrl, ret, found);
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_new(ytp_control_t **ctrl, apr_file_t *f) {
  return ytp_control_new2(ctrl, f, true);
}

APR_DECLARE(ytp_status_t) ytp_control_init(ytp_control_t *ctrl, apr_file_t *f) {
  return ytp_control_init2(ctrl, f, true);
}

APR_DECLARE(ytp_status_t) ytp_control_new2(ytp_control_t **ctrl, apr_file_t *f, bool enable_thread) {
  *ctrl = new ytp_control_t;
  ytp_status_t rv = ytp_control_init2(*ctrl, f, enable_thread);
  if(rv) {
    delete *ctrl;
    *ctrl = nullptr;
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_init2(ytp_control_t *ctrl, apr_file_t *f, bool enable_thread) {
  ytp_status_t rv = ytp_yamal_init2(&ctrl->yamal, f, enable_thread);
  if(rv) {
    return rv;
  }
  rv = ytp_yamal_begin(&ctrl->yamal, &(ctrl->ctrl));
  if(rv) {
    (void)ytp_yamal_destroy(&ctrl->yamal);
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_del(ytp_control_t *ctrl) {
  ytp_status_t rv = ytp_yamal_destroy(&ctrl->yamal);
  if(rv == YTP_STATUS_OK) {
    delete ctrl;
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_destroy(ytp_control_t *ctrl) {
  return ytp_yamal_destroy(&ctrl->yamal);
}

APR_DECLARE(ytp_status_t) ytp_control_reserve(ytp_control_t *ctrl, char **buf, apr_size_t size) {
  return ytp_time_reserve(&ctrl->yamal, buf, size);
}

APR_DECLARE(ytp_status_t) ytp_control_commit(ytp_control_t *ctrl, ytp_iterator_t *it, ytp_peer_t peer,
                                  ytp_channel_t channel, uint64_t time, void *data) {
  return ytp_time_commit(&ctrl->yamal, it, peer, channel, time, data);
}

APR_DECLARE(ytp_status_t) ytp_control_sub(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time,
                     apr_size_t size, const char *payload) {
  auto payload_str = std::string_view(payload, size);
  auto key = subs_key(payload_str);
  bool found;
  return lookup_or_insert_ctrl_msg(
    ctrl, &found,
    [ctrl, key]() {
      return ctrl->subs_announced.find(key) != ctrl->subs_announced.end();
    },
    [ctrl, &key, peer, time]() {
      char *data = NULL;
      ytp_status_t rv = ytp_control_reserve(ctrl, &data, key.size());
      if(rv) {
        return rv;
      }
      std::memcpy(data, key.data(), key.size());
      ytp_channel_t channel = YTP_CHANNEL_SUB;
      ytp_iterator_t it;
      return ytp_control_commit(ctrl, &it, peer, channel, time, data);
    });
}

APR_DECLARE(ytp_status_t) ytp_control_dir(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time,
                     apr_size_t size, const char *payload) {
  char *data = NULL;
  ytp_status_t rv = ytp_control_reserve(ctrl, &data, size);
  if(rv) {
    return rv;
  }
  std::memcpy(data, payload, size);
  ytp_channel_t channel = YTP_CHANNEL_DIR;
  ytp_iterator_t it;
  return ytp_control_commit(ctrl, &it, peer, channel, time, data);
}

APR_DECLARE(ytp_status_t) ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel, apr_size_t *size,
                         const char **name) {
  if (auto it = ctrl->channel_map.find(channel);
      it != ctrl->channel_map.end()) {
    *name = it->second.name.data();
    *size = it->second.name.size();
    return YTP_STATUS_OK;
  } else {
    return YTP_STATUS_ECHNOTFOUND;
  }
}

APR_DECLARE(ytp_status_t) ytp_control_ch_decl(ytp_control_t *ctrl, ytp_channel_t *channel, ytp_peer_t peer,
                                  uint64_t time, apr_size_t size, const char *name) {
  std::string_view namestr(name, size);

  bool found;
  ytp_status_t rv = lookup_or_insert_ctrl_msg(
    ctrl, &found,
    [ctrl, namestr]() {
      return ctrl->name_to_channel.find(namestr) !=
              ctrl->name_to_channel.end();
    },
    [ctrl, namestr, peer, time]() {
      char *dst = NULL;
      ytp_status_t rv = ytp_control_reserve(ctrl, &dst, namestr.size());
      if(rv) {
        return rv;
      }
      std::memcpy(dst, namestr.data(), namestr.size());
      ytp_channel_t channel = YTP_CHANNEL_ANN;
      ytp_iterator_t it;
      return ytp_control_commit(ctrl, &it, peer, channel, time, dst);
    });
  if(rv) {
    return rv;
  }

  *channel = found ? ctrl->name_to_channel.find(namestr)->second : 0;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer, apr_size_t *sz,
                           const char **name) {
  if (auto it = ctrl->peer_map.find(peer); it != ctrl->peer_map.end()) {
    *name = it->second.name.data();
    *sz = it->second.name.size();
    return YTP_STATUS_OK;
  } else {
    return YTP_STATUS_EPEERNOTFOUND;
  }
}

APR_DECLARE(ytp_status_t) ytp_control_peer_decl(ytp_control_t *ctrl, ytp_peer_t *peer, apr_size_t size,
                                 const char *name) {
  std::string_view namestr(name, size);

  bool found;
  ytp_status_t rv = lookup_or_insert_ctrl_msg(
      ctrl, &found,
      [ctrl, namestr]() {
        return ctrl->name_to_peer.find(namestr) != ctrl->name_to_peer.end();
      },
      [ctrl, namestr]() {
        ytp_iterator_t it;
        return ytp_peer_name(&ctrl->yamal, &it, namestr.size(), namestr.data());
      });
  if(rv) {
    return rv;
  }

  *peer = found ? ctrl->name_to_peer.find(namestr)->second : 0;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_next(ytp_control_t *ctrl, ytp_iterator_t *it_ptr, ytp_iterator_t iter) {
  ytp_status_t rv = ytp_yamal_next(&ctrl->yamal, it_ptr, iter);
  if(rv) {
    return rv;
  }
  if (iter == ctrl->ctrl) {
    ctrl->ctrl = *it_ptr;
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_read(ytp_control_t *ctrl, ytp_iterator_t it, ytp_peer_t *peer,
                      ytp_channel_t *channel, uint64_t *time, apr_size_t *size, const char **data) {
  ytp_status_t rv = ytp_time_read(&ctrl->yamal, it, peer, channel, time, size, data);
  if(rv) {
    return rv;
  }
  if (ytp_peer_ann(*peer)) {
    (void)process_peer(ctrl, std::string_view(*data, *size));
  } else {
    switch (*channel) {
      case YTP_CHANNEL_ANN: {
        (void)process_channel(ctrl, *peer, std::string_view(*data, *size));
      } break;
      case YTP_CHANNEL_SUB: {
        (void)process_sub(ctrl, *peer, std::string_view(*data, *size));
      } break;
      case YTP_CHANNEL_DIR: {
        process_dir(ctrl, *peer, std::string_view(*data, *size));
      } break;
    }
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_begin(ytp_control_t *ctrl, ytp_iterator_t *iterator) {
  return ytp_yamal_begin(&ctrl->yamal, iterator);
}

APR_DECLARE(ytp_status_t) ytp_control_end(ytp_control_t *ctrl, ytp_iterator_t *iterator) {
  ytp_status_t rv = ytp_yamal_end(&ctrl->yamal, iterator);
  if(rv) {
    return rv;
  }
  while (ctrl->ctrl != *iterator) {
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t time;
    apr_size_t sz;
    const char *data;
    rv = ytp_control_read(ctrl, ctrl->ctrl, &peer, &channel, &time, &sz, &data);
    if(rv) {
      return rv;
    }
    ytp_iterator_t new_it;
    rv = ytp_control_next(ctrl, &new_it, ctrl->ctrl);
    if(rv) {
      return rv;
    }
    ctrl->ctrl = new_it;
  }
  return rv;
}
 
APR_DECLARE(bool) ytp_control_term(ytp_iterator_t iterator) {
  return ytp_yamal_term(iterator);
}

APR_DECLARE(ytp_status_t) ytp_control_seek(ytp_control_t *ctrl, ytp_iterator_t *it_ptr, apr_size_t ptr) {
  ytp_status_t rv = ytp_yamal_seek(&ctrl->yamal, it_ptr, ptr);
  if(rv) {
    return rv;
  }
  while (ctrl->ctrl != *it_ptr && !ytp_yamal_term(ctrl->ctrl)) {
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t time;
    apr_size_t sz;
    const char *data;
    rv = ytp_control_read(ctrl, ctrl->ctrl, &peer, &channel, &time, &sz, &data);
    if(rv) {
      return rv;
    }
    ytp_iterator_t new_it;
    rv = ytp_control_next(ctrl, &new_it, ctrl->ctrl);
    if(rv) {
      return rv;
    }
    ctrl->ctrl = new_it;
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_control_tell(ytp_control_t *ctrl, apr_size_t *ptr, ytp_iterator_t iterator) {
  return ytp_yamal_tell(&ctrl->yamal, ptr, iterator);
}
