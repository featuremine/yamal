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

#include <ytp/control.h>
#include <ytp/stream.h>
#include <ytp/yamal.h>

#include "control.hpp"
#include "stream.hpp"

static const std::string_view default_encoding = "";

ytp_control_t *ytp_control_new(fmc_fd fd, fmc_error_t **error) {
  return ytp_control_new_2(fd, true, error);
}

void ytp_control_init(ytp_control_t *ctrl, fmc_fd fd, fmc_error_t **error) {
  ytp_control_init_2(ctrl, fd, true, error);
}

ytp_control_t *ytp_control_new_2(fmc_fd fd, bool enable_thread,
                                 fmc_error_t **error) {
  auto *control = static_cast<ytp_control_t *>(aligned_alloc(alignof(ytp_control_t), sizeof(ytp_control_t)));
  if (!control) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return {};
  }

  ytp_control_init_2(control, fd, enable_thread, error);
  if (*error) {
    free(control);
    return {};
  }

  return control;
}

void ytp_control_init_2(ytp_control_t *ctrl, fmc_fd fd, bool enable_thread,
                        fmc_error_t **error) {
  new (ctrl) ytp_control_t(fd, enable_thread, error);
  if (*error) {
    ctrl->~ytp_control();
  }
}

void ytp_control_del(ytp_control_t *ctrl, fmc_error_t **error) {
  ytp_control_destroy(ctrl, error);
  if (!*error) {
    delete ctrl;
  }
}

void ytp_control_destroy(ytp_control_t *ctrl, fmc_error_t **error) {
  fmc_error_clear(error);
  ctrl->~ytp_control();
}

static void data_cb(void *closure, size_t seqno, uint64_t msgtime, ytp_stream_t stream, size_t sz, const char *data) {
  auto ctrl = (ytp_control_t *)closure;
  ctrl->last_data.seqno = seqno;
  ctrl->last_data.msgtime = msgtime;
  ctrl->last_data.stream = stream;
  ctrl->last_data.data = {data, sz};
  ctrl->last_type = last_info_type::DATA;
}

static void ann_cb(void *closure, ytp_stream_t stream,
                   size_t seqno,
                   size_t peer_sz, const char *peer_name,
                   size_t ch_sz, const char *ch_name,
                   size_t encoding_sz,
                   const char *encoding_data) {
  auto ctrl = (ytp_control_t *)closure;

  auto peername = std::string_view{peer_name, peer_sz};
  auto chname = std::string_view{ch_name, ch_sz};

  auto it_peer = ctrl->name_to_peerid.emplace(peername, ctrl->name_to_peerid.size() + YTP_PEER_OFF);
  if (it_peer.second) {
    ctrl->peers.emplace_back().name = peername;
  }
  auto peerid = it_peer.first->second;

  auto it_ch = ctrl->name_to_channelid.emplace(chname, ctrl->name_to_channelid.size() + YTP_CHANNEL_OFF);
  if (it_ch.second) {
    ctrl->channels.emplace_back().name = chname;
  }
  auto channelid = it_ch.first->second;

  auto it_stream = ctrl->key_to_streamid.emplace(stream_key(peerid, channelid), stream);
  if (it_stream.second) {
    auto &peer = ctrl->peers[peerid - YTP_PEER_OFF];
    auto &channel = ctrl->channels[channelid - YTP_CHANNEL_OFF];
    ctrl->name_to_streamid.emplace(stream_name(peer.name, channel.name), stream);

    auto &stream_data = ctrl->streams[stream];
    stream_data.peer = peerid;
    stream_data.channel = channelid;
  }

  ctrl->last_ann.stream = stream;
  ctrl->last_ann.peer = peerid;
  ctrl->last_ann.channel = channelid;
  ctrl->last_ann.seqno = seqno;
  ctrl->last_ann.peername = peername;
  ctrl->last_ann.chname = chname;
  ctrl->last_ann.encoding = {encoding_data, encoding_sz};
  ctrl->last_ann.is_peer_new = it_peer.second;
  ctrl->last_ann.is_ch_new = it_ch.second;
  ctrl->last_type = last_info_type::ANN;

  fmc_error_t *error;
  ytp_cursor_data_cb(&ctrl->cursor, stream, data_cb, ctrl, &error);
}

ytp_control::ytp_control(fmc_fd fd, bool enable_thread, fmc_error_t **error) :
    cursor(&yamal, error),
    anns(&yamal, error) {
  ytp_cursor_ann_cb(&cursor, ann_cb, this, error);
}

template <typename Handler>
static void process_control_msgs(ytp_control_t *ctrl, fmc_error_t **error,
                                 Handler &handler) {
  fmc_error_clear(error);
  while (!handler.found() && !*error && !ytp_yamal_term(ctrl->cursor.it_ann)) {
    auto polled = ytp_cursor_poll_one_ann(&ctrl->cursor, error);
    if (*error) {
      return;
    }
    handler.on_stream(ctrl->last_ann);
  }
}

template <typename Handler>
static void lookup_or_insert_ctrl_msg(ytp_control_t *ctrl, fmc_error_t **error,
                                      Handler &handler) {
  fmc_error_clear(error);
  process_control_msgs(ctrl, error, handler);
  if (*error) {
    return;
  }
  if (!handler.found()) {
    handler.insert();
    if (*error) {
      return;
    }
    process_control_msgs(ctrl, error, handler);
  }
}

char *ytp_control_reserve(ytp_control_t *ctrl, size_t size,
                          fmc_error_t **error) {
  return ytp_stream_reserve(&ctrl->yamal, size, error);
}

ytp_iterator_t ytp_control_commit(ytp_control_t *ctrl, ytp_peer_t peer,
                                  ytp_channel_t channel, uint64_t msgtime,
                                  void *data, fmc_error_t **error) {
  fmc_error_clear(error);
  struct handler_t : base_handler {
    void on_stream(const ann_info &data) {
      if (data.peer == peer && data.channel == channel) {
        found_streamid = data.stream;
      }
    }
    void insert() {
      if (peer - YTP_PEER_OFF >= ctrl->peers.size()) {
        fmc_error_set(error, "peer not found");
        return;
      }
      if (channel - YTP_CHANNEL_OFF >= ctrl->channels.size()) {
        fmc_error_set(error, "channel not found");
        return;
      }
      auto &peerdata = ctrl->peers[peer - YTP_PEER_OFF];
      auto &channeldata = ctrl->channels[channel - YTP_CHANNEL_OFF];

      ytp_anns_stream(&ctrl->anns, peerdata.name.size(), peerdata.name.data(), channeldata.name.size(), channeldata.name.data(), default_encoding.size(), default_encoding.data(), error);
    }
    bool found() const noexcept { return found_streamid != std::numeric_limits<ytp_stream_t>::max(); }

    ytp_control_t *ctrl;
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t msgtime;
    fmc_error_t **error;
    ytp_stream_t found_streamid;
  };

  auto it = ctrl->key_to_streamid.find({peer, channel});
  handler_t handler{
      .ctrl = ctrl,
      .peer = peer,
      .channel = channel,
      .msgtime = msgtime,
      .error = error,
      .found_streamid = it != ctrl->key_to_streamid.end() ? it->second : std::numeric_limits<ytp_stream_t>::max(),
  };

  lookup_or_insert_ctrl_msg(ctrl, error, handler);
  if (*error) {
    return {};
  }

  return ytp_stream_commit(&ctrl->yamal, msgtime, handler.found_streamid, data, error);
}

void ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel, size_t *sz,
                         const char **name, fmc_error_t **error) {
  if (channel - YTP_CHANNEL_OFF >= ctrl->channels.size()) {
    fmc_error_set(error, "channel not found");
    return;
  }

  fmc_error_clear(error);
  auto &n = ctrl->channels[channel - YTP_CHANNEL_OFF].name;
  *name = n.data();
  *sz = n.size();
}

ytp_channel_t ytp_control_ch_decl(ytp_control_t *ctrl, ytp_peer_t peer,
                                  uint64_t msgtime, size_t sz,
                                  const char *name, fmc_error_t **error) {
  fmc_error_clear(error);

  struct handler_t : base_handler {
    void on_stream(const ann_info &data) {
      if (data.chname == channel) {
        found_chid = data.channel;
      }
    }
    void insert() {
      if (peer - YTP_PEER_OFF >= ctrl->peers.size()) {
        fmc_error_set(error, "peer not found");
        return;
      }

      std::string_view peername = ctrl->peers[peer - YTP_PEER_OFF].name;

      ytp_anns_stream(&ctrl->anns, peername.size(), peername.data(), channel.size(), channel.data(), default_encoding.size(), default_encoding.data(), error);
    }
    bool found() const noexcept { return found_chid != std::numeric_limits<ytp_channel_t>::max(); }

    ytp_control_t *ctrl;
    ytp_peer_t peer;
    std::string_view channel;
    fmc_error_t **error;
    ytp_channel_t found_chid;
  };

  auto chname = std::string_view(name, sz);
  auto it = ctrl->name_to_channelid.find(chname);
  handler_t handler{
      .ctrl = ctrl,
      .peer = peer,
      .channel = chname,
      .error = error,
      .found_chid = it != ctrl->name_to_channelid.end() ? it->second : std::numeric_limits<ytp_channel_t>::max(),
  };

  lookup_or_insert_ctrl_msg(ctrl, error, handler);
  if (*error) {
    return {};
  }

  return handler.found_chid;
}

void ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer, size_t *sz,
                           const char **name, fmc_error_t **error) {
  if (peer - YTP_PEER_OFF >= ctrl->peers.size()) {
    fmc_error_set(error, "peer not found");
    return;
  }

  fmc_error_clear(error);
  auto &n = ctrl->peers[peer - YTP_PEER_OFF].name;
  *name = n.data();
  *sz = n.size();
}

ytp_peer_t ytp_control_peer_decl(ytp_control_t *ctrl, size_t sz,
                                 const char *name, fmc_error_t **error) {
  fmc_error_clear(error);

  struct handler_t : base_handler {
    void on_stream(const ann_info &data) {
      if (data.peername == peer) {
        found_peerid = data.peer;
      }
    }
    void insert() {
      ytp_anns_stream(&ctrl->anns, peer.size(), peer.data(), 0, nullptr, default_encoding.size(), default_encoding.data(), error);
    }
    bool found() const noexcept { return found_peerid != std::numeric_limits<ytp_peer_t>::max(); }

    ytp_control_t *ctrl;
    std::string_view peer;
    fmc_error_t **error;
    ytp_peer_t found_peerid;
  };

  auto peername = std::string_view(name, sz);
  auto it = ctrl->name_to_peerid.find(peername);
  handler_t handler{
      .ctrl = ctrl,
      .peer = peername,
      .error = error,
      .found_peerid = it != ctrl->name_to_peerid.end() ? it->second : std::numeric_limits<ytp_peer_t>::max(),
  };

  lookup_or_insert_ctrl_msg(ctrl, error, handler);
  if (*error) {
    return {};
  }

  return handler.found_peerid;
}

ytp_iterator_t ytp_control_next(ytp_control_t *ctrl, ytp_iterator_t iter,
                                fmc_error_t **error) {
  fmc_error_clear(error);
  return ctrl->cursor.it_data;
}

void ytp_control_read(ytp_control_t *ctrl, ytp_iterator_t it, ytp_peer_t *peer,
                      ytp_channel_t *channel, uint64_t *msgtime, size_t *sz,
                      const char **data, fmc_error_t **error) {
  fmc_error_clear(error);

  if (!ctrl->last_ann.is_ch_new) {
    ctrl->cursor.it_data = it;
    ytp_cursor_poll(&ctrl->cursor, error);
    if (*error) {
      return;
    }
  }

  if (ctrl->last_ann.is_peer_new) {
    ctrl->last_ann.is_peer_new = false;
    *peer = YTP_PEER_ANN;
    *channel = ytp_channel_t{};
    *msgtime = {};
    *data = ctrl->last_ann.peername.data();
    *sz = ctrl->last_ann.peername.size();
    return;
  }

  if (ctrl->last_ann.is_ch_new) {
    ctrl->last_ann.is_ch_new = false;
    *peer = ctrl->last_ann.peer;
    *channel = YTP_CHANNEL_ANN;
    *msgtime = {};
    *data = ctrl->last_ann.chname.data();
    *sz = ctrl->last_ann.chname.size();
    return;
  }

  auto it_stream = ctrl->streams.find(ctrl->last_data.stream);
  if (it_stream == ctrl->streams.end()) {
    fmc_error_set(error, "referenced stream not found");
    return;
  }

  auto &stream = it_stream->second;
  *peer = stream.peer;
  *channel = stream.channel;
  *msgtime = ctrl->last_data.msgtime;
  *sz = ctrl->last_data.data.size();
  *data = ctrl->last_data.data.data();
}

ytp_iterator_t ytp_control_begin(ytp_control_t *ctrl, fmc_error_t **error) {
  return ytp_yamal_begin(&ctrl->yamal, YTP_STREAM_LIST_DATA, error);
}

ytp_iterator_t ytp_control_end(ytp_control_t *ctrl, fmc_error_t **error) {
  return ytp_yamal_end(&ctrl->yamal, YTP_STREAM_LIST_DATA, error);
}

bool ytp_control_term(ytp_iterator_t iterator) {
  return ytp_yamal_term(iterator);
}

ytp_iterator_t ytp_control_seek(ytp_control_t *ctrl, size_t ptr,
                                fmc_error_t **error) {
  return ytp_yamal_seek(&ctrl->yamal, ptr, error);
}

size_t ytp_control_tell(ytp_control_t *ctrl, ytp_iterator_t iterator,
                        fmc_error_t **error) {
  return ytp_yamal_tell(&ctrl->yamal, iterator, error);
}
