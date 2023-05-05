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

#include <fmc++/as_ref.hpp>
#include <fmc++/lazy_rem_vector.hpp>
#include <fmc++/misc.hpp>

#include <ytp/stream.h>
#include <ytp/yamal.h>

#include <list>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#pragma pack(push, 1)
struct ann_msg_t {
  std::atomic<uint64_t> subscription;
  uint16_t peer_name_sz;
  uint16_t channel_name_sz;
  char payload[];
};
struct sub_msg_t {
  ytp_stream_t stream;
};
struct idx_msg_t {
  ytp_stream_t stream;
  size_t offset;
  char payload[];
};
struct data_msg_t {
  uint64_t stream;
  char payload[];
};
static_assert(sizeof(ann_msg_t) == 12);
static_assert(sizeof(sub_msg_t) == 8);
static_assert(sizeof(idx_msg_t) == 16);
static_assert(sizeof(data_msg_t) == 8);
#pragma pack(pop)

using ytp_cursor_ann_cb_cl_t = std::pair<ytp_cursor_ann_cb_t, void *>;
using ytp_cursor_data_cb_cl_t = std::pair<ytp_cursor_data_cb_t, void *>;
using stream_name = std::pair<std::string_view, std::string_view>;

struct ytp_cursor {
  ytp_cursor(ytp_yamal_t *yamal, fmc_error_t **error);
  ytp_yamal_t *yamal;
  ytp_iterator_t it_data;
  ytp_iterator_t it_ann;
  size_t ann_processed;
  size_t last_read_seqno;

  std::list<ytp_cursor_ann_cb_cl_t> cb_ann;
  std::unordered_map<ytp_stream_t, std::list<ytp_cursor_data_cb_cl_t>> cb_data;
};

struct ytp_anns {
  ytp_anns(ytp_yamal_t *yamal, fmc_error_t **error);
  ytp_yamal_t *yamal;
  ytp_iterator_t it_ann;
  size_t ann_processed;
  std::unordered_map<stream_name, ytp_stream_t> reverse_map;
};

extern bool ytp_cursor_poll_one_ann(ytp_cursor_t *cursor, fmc_error_t **error);
extern bool ytp_cursor_poll_one_data(ytp_cursor_t *cursor, fmc_error_t **error);
