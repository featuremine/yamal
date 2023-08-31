/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once

#include "yamal.hpp"

#include <fmc++/misc.hpp>

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

using stream_key = std::pair<ytp_peer_t, ytp_channel_t>;

struct ytp_control {
  ytp_control(fmc_fd fd, bool enable_thread);
  ytp_yamal_wrap yamal;
  ytp_iterator_t anns;
  uint64_t ann_processed;

  std::vector<peer_data> peers;
  std::vector<channel_data> channels;
  std::unordered_map<ytp_mmnode_offs, control_stream_data> streams;

  std::unordered_map<std::string_view, ytp_peer_t> name_to_peerid;
  std::map<std::string_view, ytp_channel_t> name_to_channelid;
  std::unordered_map<stream_key, ytp_mmnode_offs> key_to_streamid;
};
