#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <ytp/channel.h>
#include <ytp/control.h>
#include <ytp/time.h>
#include <ytp/yamal.h>

#include <cstring>
#include <fmc/alignment.h>

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

static void read_msg(ytp_control_t *ctrl, ytp_iterator_t it, ytp_peer_t *peer,
                     ytp_channel_t *channel, uint64_t *time, size_t *sz,
                     const char **data, fmc_error_t **error) {
  ytp_time_read(&ctrl->yamal, it, peer, channel, time, sz, data, error);
  if (!*error) {
    if (ytp_peer_ann(*peer)) {
      auto &peer_d = process_peer(ctrl, std::string_view(*data, *sz));
    } else {
      switch (*channel) {
      case YTP_CHANNEL_ANN: {
        auto &channel_d =
            process_channel(ctrl, *peer, std::string_view(*data, *sz));
      } break;
      case YTP_CHANNEL_SUB: {
        auto &sub_d = process_sub(ctrl, *peer, std::string_view(*data, *sz));
      } break;
      case YTP_CHANNEL_DIR: {
        process_dir(ctrl, *peer, std::string_view(*data, *sz));
      } break;
      }
    }
  }
}

template <typename F>
static bool process_control_msgs(ytp_control_t *ctrl, fmc_error_t **error,
                                 const F &found) {
  fmc_error_clear(error);

  ytp_peer_t peer;
  ytp_channel_t channel;
  uint64_t time;
  size_t sz;
  const char *data;

  bool f;
  while (!(f = found()) && !ytp_yamal_term(ctrl->ctrl)) {
    read_msg(ctrl, ctrl->ctrl, &peer, &channel, &time, &sz, &data, error);
    if (!*error) {
      auto new_it = ytp_control_next(ctrl, ctrl->ctrl, error);
      if (!*error) {
        ctrl->ctrl = new_it;
      }
    }
  }

  return f;
}

template <typename F, typename I>
static bool lookup_or_insert_ctrl_msg(ytp_control_t *ctrl, fmc_error_t **error,
                                      const F &found, const I &insert) {
  fmc_error_clear(error);
  if (!process_control_msgs(ctrl, error, found)) {
    insert();
    if (*error) {
      return false;
    }
    return process_control_msgs(ctrl, error, found);
  }
  return true;
}

ytp_control_t *ytp_control_new(fmc_fd fd, fmc_error_t **error) {
  return ytp_control_new_2(fd, true, error);
}

void ytp_control_init(ytp_control_t *ctrl, fmc_fd fd, fmc_error_t **error) {
  ytp_control_init_2(ctrl, fd, true, error);
}

ytp_control_t *ytp_control_new_2(fmc_fd fd, bool enable_thread,
                                 fmc_error_t **error) {
  auto *ctrl = new ytp_control_t;
  ytp_control_init_2(ctrl, fd, enable_thread, error);
  if (!*error) {
    return ctrl;
  }
  delete ctrl;
  return nullptr;
}

void ytp_control_init_2(ytp_control_t *ctrl, fmc_fd fd, bool enable_thread,
                        fmc_error_t **error) {
  ytp_yamal_init_2(&ctrl->yamal, fd, enable_thread, error);
  if (!*error) {
    ctrl->ctrl = ytp_yamal_begin(&ctrl->yamal, error);
    if (*error) {
      std::string err1_msg{fmc_error_msg(*error)};
      ytp_control_destroy(ctrl, error);
      if (*error) {
        fmc_error_set(error, "%s. %s", err1_msg.c_str(), fmc_error_msg(*error));
      } else {
        fmc_error_set(error, "%s", err1_msg.c_str());
      }
    }
  }
}

void ytp_control_del(ytp_control_t *ctrl, fmc_error_t **error) {
  ytp_yamal_destroy(&ctrl->yamal, error);
  if (!*error) {
    delete ctrl;
  }
}

void ytp_control_destroy(ytp_control_t *ctrl, fmc_error_t **error) {
  ytp_yamal_destroy(&ctrl->yamal, error);
}

char *ytp_control_reserve(ytp_control_t *ctrl, size_t size,
                          fmc_error_t **error) {
  if (auto *msg = ytp_time_reserve(&ctrl->yamal, size, error); msg) {
    return msg;
  }

  return nullptr;
}

ytp_iterator_t ytp_control_commit(ytp_control_t *ctrl, ytp_peer_t peer,
                                  ytp_channel_t channel, uint64_t time,
                                  void *data, fmc_error_t **error) {
  return ytp_time_commit(&ctrl->yamal, peer, channel, time, data, error);
}

void ytp_control_sub(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time,
                     size_t sz, const char *payload_ptr, fmc_error_t **error) {
  auto payload = std::string_view(payload_ptr, sz);
  auto key = subs_key(payload);
  lookup_or_insert_ctrl_msg(
      ctrl, error,
      [ctrl, key]() {
        return ctrl->subs_announced.find(key) != ctrl->subs_announced.end();
      },
      [ctrl, &key, peer, time, error]() {
        if (auto ptr = ytp_control_reserve(ctrl, key.size(), error); !*error) {
          std::memcpy(ptr, key.data(), key.size());
          ytp_channel_t channel = YTP_CHANNEL_SUB;
          ytp_control_commit(ctrl, peer, channel, time, ptr, error);
          return true;
        }
        return false;
      });
}

void ytp_control_dir(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time,
                     size_t sz, const char *payload, fmc_error_t **error) {
  if (auto ptr = ytp_control_reserve(ctrl, sz, error); !*error) {
    std::memcpy(ptr, payload, sz);
    ytp_channel_t channel = YTP_CHANNEL_DIR;
    ytp_control_commit(ctrl, peer, channel, time, ptr, error);
  }
}

void ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel, size_t *sz,
                         const char **name, fmc_error_t **error) {
  fmc_error_clear(error);
  if (auto it = ctrl->channel_map.find(channel);
      it != ctrl->channel_map.end()) {
    *name = it->second.name.data();
    *sz = it->second.name.size();
  } else {
    fmc_error_set(error, "channel not found");
  }
}

ytp_channel_t ytp_control_ch_decl(ytp_control_t *ctrl, ytp_peer_t peer,
                                  uint64_t time, size_t sz, const char *name,
                                  fmc_error_t **error) {
  std::string_view namestr(name, sz);

  bool found = lookup_or_insert_ctrl_msg(
      ctrl, error,
      [ctrl, namestr]() {
        return ctrl->name_to_channel.find(namestr) !=
               ctrl->name_to_channel.end();
      },
      [ctrl, namestr, peer, time, error]() {
        if (char *dst = ytp_control_reserve(ctrl, namestr.size(), error); dst) {
          ytp_channel_t channel = YTP_CHANNEL_ANN;
          std::memcpy(dst, namestr.data(), namestr.size());
          ytp_control_commit(ctrl, peer, channel, time, dst, error);
          return true;
        }
        return false;
      });

  if (!found) {
    return 0;
  }

  return ctrl->name_to_channel.find(namestr)->second;
}

void ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer, size_t *sz,
                           const char **name, fmc_error_t **error) {
  fmc_error_clear(error);
  if (auto it = ctrl->peer_map.find(peer); it != ctrl->peer_map.end()) {
    *name = it->second.name.data();
    *sz = it->second.name.size();
  } else {
    fmc_error_set(error, "peer not found");
  }
}

ytp_peer_t ytp_control_peer_decl(ytp_control_t *ctrl, size_t sz,
                                 const char *name, fmc_error_t **error) {
  std::string_view namestr(name, sz);

  bool found = lookup_or_insert_ctrl_msg(
      ctrl, error,
      [ctrl, namestr]() {
        return ctrl->name_to_peer.find(namestr) != ctrl->name_to_peer.end();
      },
      [ctrl, namestr, &error]() {
        return ytp_peer_name(&ctrl->yamal, namestr.size(), namestr.data(),
                             error) != nullptr;
      });

  if (!found) {
    return 0;
  }

  return ctrl->name_to_peer.find(namestr)->second;
}

ytp_iterator_t ytp_control_next(ytp_control_t *ctrl, ytp_iterator_t iter,
                                fmc_error_t **error) {
  auto new_iter = ytp_yamal_next(&ctrl->yamal, iter, error);
  if (!*error && iter == ctrl->ctrl) {
    ctrl->ctrl = new_iter;
  }
  return new_iter;
}

void ytp_control_read(ytp_control_t *ctrl, ytp_iterator_t it, ytp_peer_t *peer,
                      ytp_channel_t *channel, uint64_t *time, size_t *sz,
                      const char **data, fmc_error_t **error) {
  read_msg(ctrl, it, peer, channel, time, sz, data, error);
}

ytp_iterator_t ytp_control_begin(ytp_control_t *ctrl, fmc_error_t **error) {
  return ytp_yamal_begin(&ctrl->yamal, error);
}

ytp_iterator_t ytp_control_end(ytp_control_t *ctrl, fmc_error_t **error) {
  auto end = ytp_yamal_end(&ctrl->yamal, error);
  while (ctrl->ctrl != end) {
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t time;
    size_t sz;
    const char *data;
    read_msg(ctrl, ctrl->ctrl, &peer, &channel, &time, &sz, &data, error);
    if (!*error) {
      auto new_it = ytp_control_next(ctrl, ctrl->ctrl, error);
      if (!*error) {
        ctrl->ctrl = new_it;
      }
    }
  }
  return end;
}

bool ytp_control_term(ytp_iterator_t iterator) {
  return ytp_yamal_term(iterator);
}

ytp_iterator_t ytp_control_seek(ytp_control_t *ctrl, size_t ptr,
                                fmc_error_t **error) {
  auto it = ytp_yamal_seek(&ctrl->yamal, ptr, error);

  while (ctrl->ctrl != it && !ytp_yamal_term(ctrl->ctrl)) {
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t time;
    size_t sz;
    const char *data;
    read_msg(ctrl, ctrl->ctrl, &peer, &channel, &time, &sz, &data, error);
    if (!*error) {
      auto new_it = ytp_control_next(ctrl, ctrl->ctrl, error);
      if (!*error) {
        ctrl->ctrl = new_it;
      }
    }
  }

  return it;
}

size_t ytp_control_tell(ytp_control_t *ctrl, ytp_iterator_t iterator,
                        fmc_error_t **error) {
  return ytp_yamal_tell(&ctrl->yamal, iterator, error);
}
