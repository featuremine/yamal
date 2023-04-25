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

#include <ytp/cursor.h>
#include <ytp/stream.h>
#include <ytp/yamal.h>

#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#pragma pack(push, 1)
struct stream_announcement_msg_t {
  uint16_t peer_name_sz;
  uint16_t channel_name_sz;
  char payload[];
};
#pragma pack(pop)

static_assert(sizeof(stream_announcement_msg_t) == 4);

using stream_name = std::pair<std::string_view, std::string_view>;

struct stream_data {
  std::string_view encoding;
  bool subscribed = 0;
};

struct ytp_stream {
  ytp_yamal_t yamal;
  ytp_iterator_t ctrl;

  std::vector<stream_data> streams;

  std::unordered_map<stream_name, ytp_stream_t> name_to_streamid;
};
