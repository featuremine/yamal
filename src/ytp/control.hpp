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

typedef std::string_view subs_key;

struct channel_data {
  std::string_view name;
};

struct peer_data {
  std::string_view name;
};

struct sub_data {};

struct ytp_control {
  ytp_yamal_t yamal;
  ytp_iterator_t ctrl;

  std::unordered_map<std::string_view, ytp_peer_t> name_to_peer;
  std::map<std::string_view, ytp_channel_t> name_to_channel;
  std::unordered_map<ytp_peer_t, peer_data> peer_map;
  std::unordered_map<ytp_channel_t, channel_data> channel_map;
  std::unordered_map<subs_key, sub_data> subs_announced;
};
