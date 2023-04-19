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

#include "yamal.hpp"

#include <fmc++/misc.hpp>
#include <fmc++/stable_map.hpp>

#include <ytp/channel.h>
#include <ytp/control.h>
#include <ytp/peer.h>
#include <ytp/timeline.h>
#include <ytp/yamal.h>

#include <map>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#pragma pack(push, 1)
struct stream_announcement_msg_t {
  uint16_t channel_name_sz;
  char channel_name[];
};
#pragma pack(pop)

static_assert(sizeof(stream_announcement_msg_t) == 2);

using subs_key = std::pair<ytp_peer_t, ytp_channel_t>;
using stream_name = std::pair<ytp_peer_t, std::string_view>;
using stream_key = std::pair<ytp_peer_t, ytp_channel_t>;

struct channel_data {
  std::string_view name;
};

struct stream_data {
  ytp_channel_t channel;
  ytp_peer_t peer;
  std::string_view encoding;
};

struct peer_data {
  std::string_view name;
};

struct ytp_control {
  ytp_yamal_t yamal;
  ytp_iterator_t ctrl;

  std::vector<peer_data> peers;
  std::vector<channel_data> channels;
  std::vector<stream_data> streams;

  std::unordered_map<std::string_view, ytp_peer_t> name_to_peerid;
  std::map<std::string_view, ytp_channel_t> name_to_channelid;
  std::unordered_map<stream_name, std::size_t> name_to_streamid;
  std::unordered_map<stream_key, std::size_t> key_to_streamid;
  std::unordered_set<subs_key> key_to_subs;
};
