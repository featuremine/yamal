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
