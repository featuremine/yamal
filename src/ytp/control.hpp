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

#pragma once

#include "stream.hpp"
#include "yamal.hpp"

#include <ytp/control.h>
#include <ytp/yamal.h>

#include <map>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct peer_data {
  std::string_view name;
};

struct channel_data {
  std::string_view name;
};

struct control_stream_data {
  ytp_peer_t peer;
  ytp_channel_t channel;
};

struct ann_info {
  ytp_stream_t stream;
  ytp_peer_t peer;
  ytp_channel_t channel;
  size_t seqno;
  std::string_view peername;
  std::string_view chname;
  std::string_view encoding;
};

struct data_info {
  size_t seqno;
  uint64_t msgtime;
  ytp_stream_t stream;
  std::string_view data;
};

using stream_key = std::pair<ytp_peer_t, ytp_channel_t>;
using stream_name = std::pair<std::string_view, std::string_view>;

struct ytp_control_cursor {
  enum class state_t {
    NONE,
    DATA,
    ANN_PEERCH,
    ANN_CH,
  };

  ytp_control_cursor(ytp_yamal_t *yamal) : cursor(yamal) {}

  state_t state;
  ytp_cursor_t cursor;
  union last_t {
    last_t() {}
    data_info data;
    ann_info ann;
  } last;
};

struct ytp_control {
  ytp_control(fmc_fd fd, bool enable_thread);
  ytp_yamal_t yamal;

  ytp_control_cursor data;
  ytp_control_cursor ann;

  std::vector<peer_data> peers;
  std::vector<channel_data> channels;
  std::unordered_map<ytp_stream_t, control_stream_data> streams;

  std::unordered_map<std::string_view, ytp_peer_t> name_to_peerid;
  std::map<std::string_view, ytp_channel_t> name_to_channelid;
  std::unordered_map<stream_name, ytp_stream_t> name_to_streamid;
  std::unordered_map<stream_key, ytp_stream_t> key_to_streamid;
};

struct base_handler {
  void on_stream(const ann_info &data) {}
};
