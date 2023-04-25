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
#include <ytp/channel.h>
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
  ytp_channel_t channel;
  ytp_peer_t peer;
};

struct ytp_control : ytp_stream {
  std::vector<peer_data> peers;
  std::vector<channel_data> channels;
  std::vector<control_stream_data> control_streams;

  std::unordered_map<std::string_view, ytp_peer_t> name_to_peerid;
  std::map<std::string_view, ytp_channel_t> name_to_channelid;
};
