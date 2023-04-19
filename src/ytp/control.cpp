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

#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <ytp/channel.h>
#include <ytp/control.h>
#include <ytp/time.h>
#include <ytp/yamal.h>

#include <cstring>

#include "control.hpp"
#include "yamal.hpp"

static ytp_peer_t process_peer(ytp_control_t *ctrl, std::string_view name) {
  auto it = ctrl->name_to_peerid.emplace(name, ctrl->name_to_peerid.size() + YTP_PEER_OFF);
  if (it.second) {
    ctrl->peers.emplace_back().name = name;
  }
  return it.first->second;
}

static std::size_t process_stream(ytp_control *ctrl, ytp_peer_t peer, std::string_view data, fmc_error_t **error) {
  if (data.size() < sizeof(stream_announcement_msg_t)) {
    fmc_error_set(error, "invalid stream announcement message");
    return 0;
  }

  auto &hdr = *reinterpret_cast<const stream_announcement_msg_t *>(data.data());
  auto channel_name_sz = ye64toh(hdr.channel_name_sz);

  if (data.size() < channel_name_sz + sizeof(stream_announcement_msg_t)) {
    fmc_error_set(error, "invalid stream announcement message");
    return 0;
  }

  const char *encoding_ptr = data.data() + channel_name_sz + sizeof(stream_announcement_msg_t);
  size_t encoding_sz = data.size() - channel_name_sz - sizeof(stream_announcement_msg_t);
  auto name = std::string_view(hdr.channel_name, channel_name_sz);
  auto encoding = std::string_view(encoding_ptr, encoding_sz);

  auto it_ch = ctrl->name_to_channelid.emplace(name, ctrl->name_to_channelid.size() + YTP_CHANNEL_OFF);
  if (it_ch.second) {
    ctrl->channels.emplace_back().name = name;
  }

  auto it_stream = ctrl->key_to_streamid.emplace(stream_key(peer, it_ch.first->second), ctrl->key_to_streamid.size());
  if (it_stream.second) {
    ctrl->name_to_streamid.emplace(stream_name(peer, ctrl->channels[it_ch.first->second - YTP_CHANNEL_OFF].name), it_stream.first->second);
    auto &stream = ctrl->streams.emplace_back();
    stream.peer = peer;
    stream.channel = it_ch.first->second;
    stream.encoding = encoding;
  }

  return it_stream.first->second;
}

static subs_key process_sub(ytp_control *ctrl, ytp_peer_t peer, std::string_view data, fmc_error_t **error) {
  if (data.size() < sizeof(ytp_channel_t)) {
    fmc_error_set(error, "invalid subscribe message");
    return {};
  }
  auto ch = ye64toh(*reinterpret_cast<const ytp_channel_t *>(data.data()));
  auto key = subs_key(peer, ch);
  ctrl->key_to_subs.emplace(key);
  return key;
}

struct default_read_msg_handler {
  void on_peer(ytp_peer_t) {}
  void on_stream(std::size_t) {}
  void on_sub(const subs_key &) {}
};

template<typename Handler = fmc::as_ref<default_read_msg_handler>>
static void read_msg(ytp_control_t *ctrl, ytp_iterator_t it, ytp_peer_t *peer,
                     ytp_channel_t *channel, uint64_t *time, size_t *sz,
                     const char **data, fmc_error_t **error,
                     Handler handler = Handler()) {
  ytp_time_read(&ctrl->yamal, it, peer, channel, time, sz, data, error);
  if (*error) {
    return;
  }

  if (ytp_peer_ann(*peer)) {
    handler->on_peer(process_peer(ctrl, std::string_view(*data, *sz)));
  } else {
    switch (*channel) {
    case YTP_CHANNEL_ANN: {
      auto i = process_stream(ctrl, *peer, std::string_view(*data, *sz), error);
      if (*error) {
        return;
      }
      handler->on_stream(i);
    } break;
    case YTP_CHANNEL_SUB: {
      auto key = process_sub(ctrl, *peer, std::string_view(*data, *sz), error);
      if (*error) {
        return;
      }
      handler->on_sub(key);
    } break;
    }
  }
}

template<typename Handler>
static void process_control_msgs(ytp_control_t *ctrl, fmc_error_t **error,
                                 Handler handler) {
  fmc_error_clear(error);

  ytp_peer_t peer;
  ytp_channel_t channel;
  uint64_t time;
  size_t sz;
  const char *data;

  while (!handler->found() && !*error && !ytp_yamal_term(ctrl->ctrl)) {
    read_msg(ctrl, ctrl->ctrl, &peer, &channel, &time, &sz, &data, error, handler);
    if (*error) {
      return;
    }
    auto new_it = ytp_control_next(ctrl, ctrl->ctrl, error);
    if (*error) {
      return;
    }
    ctrl->ctrl = new_it;
  }
}

template <typename Handler>
static void lookup_or_insert_ctrl_msg(ytp_control_t *ctrl, fmc_error_t **error,
                                      Handler handler) {
  fmc_error_clear(error);
  process_control_msgs(ctrl, error, handler);
  if (*error) {
    return;
  }
  if (!handler->found()) {
    handler->insert();
    if (*error) {
      return;
    }
    process_control_msgs(ctrl, error, handler);
  }
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

void ytp_control_sub(ytp_control_t *ctrl, uint64_t time, ytp_peer_t peer, ytp_channel_t channel, fmc_error_t **error) {
  struct handler_t : default_read_msg_handler {
    void on_sub(const subs_key &key) {
      found_ = found_ || key_ == key;
    }
    void insert() {
#ifdef DIRECT_BYTE_ORDER
      auto payload = std::string_view((char *)&key_.second, sizeof(ytp_channel_t));
#else
      auto payload_storage = ytp_channel_t(htoye64(channel));
      auto payload = std::string_view((char *)&payload_storage, sizeof(ytp_channel_t));
#endif
      if (auto ptr = ytp_control_reserve(ctrl_, payload.size(), error_); !*error_) {
        std::memcpy(ptr, payload.data(), payload.size());
        ytp_control_commit(ctrl_, key_.first, YTP_CHANNEL_SUB, time_, ptr, error_);
      }
    }
    bool found() const noexcept { return found_; }

    ytp_control_t *ctrl_;
    subs_key key_;
    uint64_t time_;
    fmc_error_t **error_;
    bool found_;
  };

  auto key = subs_key(peer, channel);
  handler_t handler {
    .ctrl_ = ctrl,
    .key_ = key,
    .time_ = time,
    .error_ = error,
    .found_ = ctrl->key_to_subs.find(key) != ctrl->key_to_subs.end(),
  };

  lookup_or_insert_ctrl_msg(ctrl, error, &handler);
}

void ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel, size_t *sz,
                         const char **name, fmc_error_t **error) {
  if (channel < YTP_PEER_OFF) {
    fmc_error_set(error, "channel not found");
    return;
  }
  auto idx = channel - YTP_CHANNEL_OFF;
  if (idx >= ctrl->peers.size()) {
    fmc_error_set(error, "channel not found");
    return;
  }

  fmc_error_clear(error);
  auto &n = ctrl->channels[idx].name;
  *name = n.data();
  *sz = n.size();
}

ytp_channel_t ytp_control_stream_decl(ytp_control_t *ctrl,
                                      uint64_t time, ytp_peer_t peer,
                                      size_t chname_sz, const char *chname_ptr,
                                      size_t encoding_sz, const char *encoding_ptr,
                                      fmc_error_t **error) {
  if (chname_sz > YTP_CHANNEL_NAME_MAXLEN) {
    fmc_error_set(error, "channel name is too long");
    return {};
  }

  struct handler_t : default_read_msg_handler {
    void on_stream(std::size_t streamid) {
      auto &stream = ctrl_->streams[streamid];
      if (stream.peer == peer_ && ctrl_->channels[stream.channel - YTP_CHANNEL_OFF].name == chname_) {
        found_ = streamid;
      }
    }
    void insert() {
      size_t sz = sizeof(stream_announcement_msg_t) + chname_.size() + encoding_.size();
      if (auto ptr = ytp_control_reserve(ctrl_, sz, error_); !*error_) {
        auto &hdr = *reinterpret_cast<stream_announcement_msg_t *>(ptr);
        hdr.channel_name_sz = htoye64(chname_.size());
        std::memcpy(hdr.channel_name, chname_.data(), chname_.size());
        std::memcpy(hdr.channel_name + chname_.size(), encoding_.data(), encoding_.size());
        ytp_control_commit(ctrl_, peer_, YTP_CHANNEL_ANN, time_, ptr, error_);
      }
    }
    bool found() const noexcept { return found_ != std::numeric_limits<std::size_t>::max(); }

    ytp_control_t *ctrl_;
    std::string_view chname_;
    std::string_view encoding_;
    ytp_peer_t peer_;
    uint64_t time_;
    fmc_error_t **error_;
    std::size_t found_;
  };

  auto chname = std::string_view(chname_ptr, chname_sz);
  auto it = ctrl->name_to_streamid.find({peer, chname});
  handler_t handler {
    .ctrl_ = ctrl,
    .chname_ = chname,
    .encoding_ = std::string_view(encoding_ptr, encoding_sz),
    .peer_ = peer,
    .time_ = time,
    .error_ = error,
    .found_ = it != ctrl->name_to_streamid.end() ? it->second : std::numeric_limits<std::size_t>::max(),
  };

  lookup_or_insert_ctrl_msg(ctrl, error, &handler);

  auto &stream = ctrl->streams[handler.found_];
  if (stream.encoding != handler.encoding_) {
    fmc_error_set(error, "encoding cannot be redefined");
    return {};
  }

  return stream.channel;
}

void ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer, size_t *sz,
                           const char **name, fmc_error_t **error) {
  if (peer < YTP_PEER_OFF) {
    fmc_error_set(error, "peer not found");
    return;
  }
  auto idx = peer - YTP_PEER_OFF;
  if (idx >= ctrl->peers.size()) {
    fmc_error_set(error, "peer not found");
    return;
  }

  fmc_error_clear(error);
  auto &n = ctrl->peers[idx].name;
  *name = n.data();
  *sz = n.size();
}

ytp_peer_t ytp_control_peer_decl(ytp_control_t *ctrl, size_t sz,
                                 const char *name, fmc_error_t **error) {
  struct handler_t : default_read_msg_handler {
    void on_peer(ytp_peer_t peerid) {
      auto &peer = ctrl_->peers[peerid - YTP_PEER_OFF];
      if (peer.name == name_) {
        found_ = peerid;
      }
    }
    void insert() {
      ytp_peer_name(&ctrl_->yamal, name_.size(), name_.data(), error_);
    }
    bool found() const noexcept { return found_ != ytp_peer_t{}; }

    ytp_control_t *ctrl_;
    std::string_view name_;
    fmc_error_t **error_;
    ytp_peer_t found_;
  };

  auto namestr = std::string_view(name, sz);
  auto it = ctrl->name_to_peerid.find(namestr);
  handler_t handler {
    .ctrl_ = ctrl,
    .name_ = namestr,
    .error_ = error,
    .found_ = it != ctrl->name_to_peerid.end() ? it->second : ytp_peer_t{},
  };

  lookup_or_insert_ctrl_msg(ctrl, error, &handler);

  return handler.found_;
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
  if (*error) {
    return nullptr;
  }

  while (ctrl->ctrl != it && !ytp_yamal_term(ctrl->ctrl)) {
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t time;
    size_t sz;
    const char *data;
    read_msg(ctrl, ctrl->ctrl, &peer, &channel, &time, &sz, &data, error);
    if (*error) {
      return nullptr;
    }
    auto new_it = ytp_control_next(ctrl, ctrl->ctrl, error);
    if (*error) {
      return nullptr;
    }
    ctrl->ctrl = new_it;
  }

  return it;
}

size_t ytp_control_tell(ytp_control_t *ctrl, ytp_iterator_t iterator,
                        fmc_error_t **error) {
  return ytp_yamal_tell(&ctrl->yamal, iterator, error);
}
