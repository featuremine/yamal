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
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <fmc++/error.hpp>

#include <ytp/control.h>
#include <ytp/data.h>
#include <ytp/stream.h>
#include <ytp/streams.h>
#include <ytp/yamal.h>

#include "control.hpp"

static const std::string_view default_encoding = "";

ytp_control_t *ytp_control_new(fmc_fd fd, fmc_error_t **error) {
  return ytp_control_new_2(fd, true, error);
}

void ytp_control_init(ytp_control_t *ctrl, fmc_fd fd, fmc_error_t **error) {
  ytp_control_init_2(ctrl, fd, true, error);
}

ytp_control_t *ytp_control_new_2(fmc_fd fd, bool enable_thread,
                                 fmc_error_t **error) {
  auto *control = static_cast<ytp_control_t *>(
      aligned_alloc(alignof(ytp_control_t), sizeof(ytp_control_t)));
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
  try {
    new (ctrl) ytp_control_t(fd, enable_thread);
    fmc_error_clear(error);
  } catch (fmc::error &e) {
    *error = fmc_error_inst();
    fmc_error_mov(*error, &e);
  }
}

void ytp_control_del(ytp_control_t *ctrl, fmc_error_t **error) {
  ytp_control_destroy(ctrl, error);
  if (error) {
    return;
  }

  free(ctrl);
}

void ytp_control_destroy(ytp_control_t *ctrl, fmc_error_t **error) {
  fmc_error_clear(error);
  ctrl->~ytp_control();
}

ytp_control::ytp_control(fmc_fd fd, bool enable_thread)
    : yamal(fd, enable_thread, YTP_UNCLOSABLE) {
  fmc_error_t *error;
  anns = ytp_yamal_begin(&yamal, YTP_STREAM_LIST_ANNS, &error);
  if (error) {
    throw fmc::error(*error);
  }
}

template<typename Handler>
static ytp_mmnode_offs process_ann(ytp_control_t *ctrl, ytp_mmnode_offs stream, std::string_view peername, std::string_view chname, Handler &handler) {
  auto it_peer = ctrl->name_to_peerid.emplace(
      peername, ctrl->name_to_peerid.size() + YTP_PEER_OFF);
  if (it_peer.second) {
    ctrl->peers.emplace_back().name = peername;
  }
  auto peerid = it_peer.first->second;

  auto it_ch = ctrl->name_to_channelid.emplace(
      chname, ctrl->name_to_channelid.size() + YTP_CHANNEL_OFF);
  if (it_ch.second) {
    ctrl->channels.emplace_back().name = chname;
  }
  auto channelid = it_ch.first->second;

  auto it_stream =
      ctrl->key_to_streamid.emplace(stream_key(peerid, channelid), stream);
  if (it_stream.second) {
    auto &stream_data = ctrl->streams[stream];
    stream_data.peer = peerid;
    stream_data.channel = channelid;
    handler.on_stream(stream, peerid, channelid, peername, chname);
  }

  return it_stream.first->second;
}

template <typename Handler>
static void process_control_msgs(ytp_control_t *ctrl, fmc_error_t **error,
                                 Handler &handler) {
  fmc_error_clear(error);

  while (!handler.found() && !ytp_yamal_term(ctrl->anns)) {
    uint64_t seqno;
    size_t read_psz;
    const char *read_peer;
    size_t read_csz;
    const char *read_channel;
    size_t read_esz;
    const char *read_encoding;
    ytp_mmnode_offs *original;
    ytp_mmnode_offs *subscribed;

    ytp_announcement_read(&ctrl->yamal, ctrl->anns, &seqno, &read_psz,
                          &read_peer, &read_csz, &read_channel, &read_esz,
                          &read_encoding, &original, &subscribed, error);
    if (*error) {
      return;
    }

    ytp_mmnode_offs stream =
        ytp_yamal_tell(&ctrl->yamal, ctrl->anns, error);
    if (*error) {
      return;
    }

    ytp_mmnode_offs o = process_ann(ctrl, stream, {read_peer, read_psz}, {read_channel, read_csz}, handler);
    if (*original != o) {
      *original = o;
    }

    ytp_iterator_t next =
        ytp_yamal_next(&ctrl->yamal, ctrl->anns, error);
    if (*error) {
      return;
    }
    ctrl->anns = next;
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
  return ytp_data_reserve(&ctrl->yamal, size, error);
}

ytp_iterator_t ytp_control_commit(ytp_control_t *ctrl, ytp_peer_t peer,
                                  ytp_channel_t channel, uint64_t msgtime,
                                  void *data, fmc_error_t **error) {
  fmc_error_clear(error);
  struct handler_t {
    void on_stream(ytp_mmnode_offs stream, ytp_peer_t peerid, ytp_channel_t channelid, std::string_view peername, std::string_view chname) {
      if (peerid == peer && channelid == channel) {
        found_streamid = stream;
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

      ytp_announcement_write(&ctrl->yamal, peerdata.name.size(),
                           peerdata.name.data(), channeldata.name.size(),
                           channeldata.name.data(), default_encoding.size(),
                           default_encoding.data(), error);
    }
    bool found() const {
      return found_streamid != std::numeric_limits<ytp_mmnode_offs>::max();
    }

    ytp_control_t *ctrl;
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t msgtime;
    fmc_error_t **error;
    ytp_mmnode_offs found_streamid;
  };

  auto it = ctrl->key_to_streamid.find(stream_key{peer, channel});
  handler_t handler{
      .ctrl = ctrl,
      .peer = peer,
      .channel = channel,
      .msgtime = msgtime,
      .error = error,
      .found_streamid = it != ctrl->key_to_streamid.end()
                            ? it->second
                            : std::numeric_limits<ytp_mmnode_offs>::max(),
  };

  lookup_or_insert_ctrl_msg(ctrl, error, handler);
  if (*error) {
    return {};
  }

  return ytp_data_commit(&ctrl->yamal, msgtime, handler.found_streamid, data,
                           error);
}

void ytp_control_sub(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time,
                     size_t sz, const char *payload_ptr, fmc_error_t **error) {
  fmc_error_set(error, "not supported");
}

void ytp_control_dir(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time,
                     size_t sz, const char *payload, fmc_error_t **error) {
  fmc_error_set(error, "not supported");
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
                                  uint64_t msgtime, size_t sz, const char *name,
                                  fmc_error_t **error) {
  fmc_error_clear(error);

  struct handler_t {
    void on_stream(ytp_mmnode_offs stream, ytp_peer_t peerid, ytp_channel_t channelid, std::string_view peername, std::string_view chname) {
      if (chname == channel) {
        found_chid = channelid;
      }
    }
    void insert() {
      if (peer - YTP_PEER_OFF >= ctrl->peers.size()) {
        fmc_error_set(error, "peer not found");
        return;
      }

      std::string_view peername = ctrl->peers[peer - YTP_PEER_OFF].name;

      ytp_announcement_write(&ctrl->yamal, peername.size(), peername.data(),
                           channel.size(), channel.data(),
                           default_encoding.size(), default_encoding.data(),
                           error);
    }
    bool found() const {
      return found_chid != std::numeric_limits<ytp_channel_t>::max();
    }

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
      .found_chid = it != ctrl->name_to_channelid.end()
                        ? it->second
                        : std::numeric_limits<ytp_channel_t>::max(),
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

  struct handler_t {
    void on_stream(ytp_mmnode_offs stream, ytp_peer_t peerid, ytp_channel_t channelid, std::string_view peername, std::string_view chname) {
      if (peername == peer) {
        found_peerid = peerid;
      }
    }
    void insert() {
      ytp_announcement_write(&ctrl->yamal, peer.size(), peer.data(), 0, nullptr,
                           default_encoding.size(), default_encoding.data(),
                           error);
    }
    bool found() const {
      return found_peerid != std::numeric_limits<ytp_peer_t>::max();
    }

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
      .found_peerid = it != ctrl->name_to_peerid.end()
                          ? it->second
                          : std::numeric_limits<ytp_peer_t>::max(),
  };

  lookup_or_insert_ctrl_msg(ctrl, error, handler);
  if (*error) {
    return {};
  }

  return handler.found_peerid;
}

ytp_iterator_t ytp_control_next(ytp_control_t *ctrl, ytp_iterator_t iter,
                                fmc_error_t **error) {
  auto new_iter = ytp_yamal_next(&ctrl->yamal, iter, error);
  if (!*error && iter == ctrl->anns) {
    ctrl->anns = new_iter;
  }
  return new_iter;
}

void ytp_control_read(ytp_control_t *ctrl, ytp_iterator_t it, ytp_peer_t *peer,
                      ytp_channel_t *channel, uint64_t *time, size_t *sz,
                      const char **data, fmc_error_t **error) {
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

ytp_iterator_t ytp_control_seek(ytp_control_t *ctrl, uint64_t ptr,
                                fmc_error_t **error) {
  return ytp_yamal_seek(&ctrl->yamal, ptr, error);
}

uint64_t ytp_control_tell(ytp_control_t *ctrl, ytp_iterator_t iterator,
                          fmc_error_t **error) {
  return ytp_yamal_tell(&ctrl->yamal, iterator, error);
}
