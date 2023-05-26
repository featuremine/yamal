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
  uint64_t offset;
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
  explicit ytp_cursor(ytp_yamal_t *yamal);
  ytp_yamal_t *yamal;
  ytp_iterator_t it_data;
  ytp_iterator_t it_ann;
  uint64_t ann_processed;
  uint64_t data_processed;

  fmc::lazy_rem_vector<ytp_cursor_ann_cb_cl_t> cb_ann;
  std::unordered_map<ytp_stream_t,
                     fmc::lazy_rem_vector<ytp_cursor_data_cb_cl_t>>
      cb_data;
};

struct ytp_anns {
  ytp_anns(ytp_yamal_t *yamal);
  ytp_yamal_t *yamal;
  ytp_iterator_t it_ann;
  uint64_t ann_processed;
  std::unordered_map<stream_name, ytp_stream_t> reverse_map;
};

extern bool ytp_cursor_poll_ann(ytp_cursor_t *cursor, fmc_error_t **error);
extern bool ytp_cursor_poll_data(ytp_cursor_t *cursor, fmc_error_t **error);
extern bool ytp_cursor_term_ann(ytp_cursor_t *cursor);
extern std::tuple<std::string_view, std::string_view, std::string_view,
                  const std::atomic<uint64_t> *>
parse_ann_payload(const char *data, std::size_t sz, fmc_error_t **error);

template <typename F>
void ytp_anns_lookup_one(ytp_anns_t *anns, fmc_error_t **error,
                         const F &should_stop) {
  while (!ytp_yamal_term(anns->it_ann)) {
    uint64_t seqno;
    size_t sz;
    const char *dataptr;

    ytp_yamal_read(anns->yamal, anns->it_ann, &seqno, &sz, &dataptr, error);
    if (*error) {
      return;
    }

    ytp_stream_t stream = ytp_yamal_tell(anns->yamal, anns->it_ann, error);
    if (*error) {
      return;
    }

    auto next_it = ytp_yamal_next(anns->yamal, anns->it_ann, error);
    if (*error) {
      return;
    }

    auto [peername, chname, encoding, const_sub] =
        parse_ann_payload(dataptr, sz, error);
    if (*error) {
      return;
    }

    using key_t = typename decltype(anns->reverse_map)::key_type;
    auto it = anns->reverse_map.emplace(key_t{peername, chname}, stream);

    SUBSCRIPTION_STATE state;
    if (anns->yamal->readonly_) {
      state = static_cast<SUBSCRIPTION_STATE>(const_sub->load());
      if (state == SUBSCRIPTION_STATE::YTP_SUB_UNKNOWN) {
        break;
      }
    } else {
      uint64_t unset = SUBSCRIPTION_STATE::YTP_SUB_UNKNOWN;
      auto &sub = *const_cast<std::atomic<uint64_t> *>(const_sub);
      sub.compare_exchange_weak(
          unset, it.second ? SUBSCRIPTION_STATE::YTP_SUB_NO_SUBSCRIPTION
                           : SUBSCRIPTION_STATE::YTP_SUB_DUPLICATED);
      state = static_cast<SUBSCRIPTION_STATE>(sub.load());
    }

    anns->it_ann = next_it;
    anns->ann_processed = seqno;

    if (state != SUBSCRIPTION_STATE::YTP_SUB_DUPLICATED) {
      if (should_stop(it.first->second, peername, chname, encoding)) {
        return;
      }
    }
  }
}
