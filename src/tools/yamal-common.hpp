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

#include <fmc++/time.hpp>
#include <ytp/cursor.h>
#include <ytp/data.h>
#include <ytp/streams.h>
#include <ytp/subscription.h>
#include <ytp/version.h>
#include <ytp/yamal.h>

#include <cstring>
#include <deque>
#include <iostream>

#define CHECK(E)                                                               \
  if (E) {                                                                     \
    handler.on_error(E);                                                       \
    return;                                                                    \
  }

struct data_cl_t {
  void *closure;
  ytp_mmnode_offs dst_stream;
};

template <typename Handler> struct ann_cl_t {
  ann_cl_t(Handler &handler) : handler(handler) {}

  void init(const char *src_name, const char *dest_name) {
    src_fd = fmc_fopen(src_name, fmc_fmode::READ, &error);
    if (error) {
      fmc_error_set(&error, "Unable to open file %s: %s", src_name,
                    fmc_error_msg(error));
      return;
    }

    dest_fd = fmc_fopen(dest_name, fmc_fmode::READWRITE, &error);
    if (error) {
      fmc_error_set(&error, "Unable to open file %s: %s", dest_name,
                    fmc_error_msg(error));
      return;
    }

    src_yml = ytp_yamal_new(src_fd, &error);
    CHECK(error);

    dest_yml = ytp_yamal_new(dest_fd, &error);
    CHECK(error);

    cursor = ytp_cursor_new(src_yml, &error);
    CHECK(error);

    streams = ytp_streams_new(dest_yml, &error);
    CHECK(error);

    ytp_cursor_ann_cb(
        cursor,
        [](void *closure, uint64_t seqno, ytp_mmnode_offs stream,
           size_t peer_sz, const char *peer_name, size_t ch_sz,
           const char *ch_name, size_t encoding_sz, const char *encoding_data,
           bool subscribed) {
          static_cast<ann_cl_t *>(closure)->on_ann(
              seqno, stream, peer_sz, peer_name, ch_sz, ch_name, encoding_sz,
              encoding_data, subscribed);
        },
        this, &error);
    CHECK(error);
  }

  void on_ann(uint64_t seqno, ytp_mmnode_offs stream, size_t peer_sz,
              const char *peer_name, size_t ch_sz, const char *ch_name,
              size_t encoding_sz, const char *encoding_data, bool subscribed) {
    auto &cl = data_cl.emplace_back();
    cl.closure = this;
    cl.dst_stream =
        ytp_streams_announce(streams, peer_sz, peer_name, ch_sz, ch_name,
                             encoding_sz, encoding_data, &error);
    CHECK(error);

    if (subscribed) {
      ytp_subscription_commit(dest_yml, stream, &error);
      CHECK(error);
    }

    ytp_cursor_data_cb(
        cursor, stream,
        [](void *closure, uint64_t seqno, int64_t ts,
           ytp_mmnode_offs src_stream, size_t sz, const char *data) {
          auto &cl = *static_cast<data_cl_t *>(closure);
          static_cast<ann_cl_t *>(cl.closure)
              ->on_data(seqno, ts, cl.dst_stream, sz, data);
        },
        &cl, &error);
    CHECK(error);
  }

  void on_data(uint64_t seqno, int64_t ts, ytp_mmnode_offs dst_stream,
               size_t sz, const char *data) {
    auto commit = handler.on_message(dest_yml, ts, sz, &error);
    CHECK(error);
    if (!commit) {
      return;
    }

    auto *d = ytp_data_reserve(dest_yml, sz, &error);
    CHECK(error);

    memcpy(d, data, sz);

    ytp_data_commit(dest_yml, ts, dst_stream, d, &error);
    CHECK(error);
  }

  ~ann_cl_t() {
    if (error)
      std::cerr << fmc_error_msg(error) << std::endl;
    if (streams)
      ytp_streams_del(streams, &error);
    if (cursor)
      ytp_cursor_del(cursor, &error);
    if (src_yml)
      ytp_yamal_del(src_yml, &error);
    if (dest_yml)
      ytp_yamal_del(dest_yml, &error);
    if (src_fd != -1)
      fmc_fclose(src_fd, &error);
    if (dest_fd != -1)
      fmc_fclose(dest_fd, &error);
  }

  Handler &handler;
  std::deque<data_cl_t> data_cl;
  fmc_error_t *error = nullptr;
  fmc_fd src_fd = -1;
  fmc_fd dest_fd = -1;
  ytp_yamal_t *src_yml = nullptr;
  ytp_yamal_t *dest_yml = nullptr;
  ytp_cursor_t *cursor = nullptr;
  ytp_streams_t *streams = nullptr;
};
