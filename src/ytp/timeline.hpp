/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once

#include "cursor.h"

#include "control.hpp"
#include "yamal.hpp"

#include <fmc++/lazy_rem_vector.hpp>
#include <fmc++/misc.hpp>
#include <fmc++/stable_map.hpp>

#include <ytp/control.h>
#include <ytp/cursor.h>
#include <ytp/timeline.h>
#include <ytp/yamal.h>

#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using ytp_timeline_peer_cb_cl_t = std::pair<ytp_timeline_peer_cb_t, void *>;
using ytp_timeline_ch_cb_cl_t = std::pair<ytp_timeline_ch_cb_t, void *>;
using ytp_timeline_data_cb_cl_t = std::pair<ytp_timeline_data_cb_t, void *>;
using ytp_timeline_idle_cb_cl_t = std::pair<ytp_timeline_idle_cb_t, void *>;

using prfx_cb_key = std::string;
using ch_key = ytp_channel_t;

struct ytp_timeline {
  ytp_timeline(ytp_control_t *ctrl);
  ytp_control_t *ctrl;
  ytp_iterator_t it_data;
  ytp_iterator_t it_ann;
  uint64_t ann_processed;

  fmc::lazy_rem_vector<ytp_timeline_peer_cb_cl_t> cb_peer;
  fmc::lazy_rem_vector<ytp_timeline_ch_cb_cl_t> cb_ch;
  std::unordered_map<prfx_cb_key,
                     fmc::lazy_rem_vector<ytp_timeline_data_cb_cl_t>>
      prfx_cb;
  fmc::stable_map<ch_key, fmc::lazy_rem_vector<ytp_timeline_data_cb_cl_t>>
      idx_cb;
  fmc::lazy_rem_vector<ytp_timeline_idle_cb_cl_t> cb_idle;
  std::vector<uint8_t> ch_announced;
  std::vector<uint8_t> peer_announced;
  std::unordered_set<std::string_view> sub_announced;
};
