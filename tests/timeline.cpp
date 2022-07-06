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

/**
 * @file timeline.cpp
 * @author Featuremine Corporation
 * @date 10 Jan 2022
 * @brief File contains tests for YTP timeline API
 *
 * @see http://www.featuremine.com
 */

#include <ytp/yamal.h>
#include <ytp/peer.h>
#include <ytp/channel.h>
#include <ytp/time.h>
#include <ytp/control.h>
#include <ytp/timeline.h>
#include <ytp/errno.h> // ytp_status_t
#include <stdbool.h>
#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_pools.h> // apr_pool_t
#include <apr_file_io.h> // apr_file_t

#include <string_view>
#include <vector>
#include <tuple>
#include <set> // std::multiset
#include <functional> // std::function
#include <string.h> // strcpy()
#include <cstring> // std::memcpy
#include "utils.h" // test_init() make_temp_file()
#include "gtestwrap.hpp"

static void data_simple_subscription_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_simple_subscription_1) {
 data_simple_subscription_1(true);
 data_simple_subscription_1(false);
}

static void data_simple_subscription_rm_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  // ytp_timeline_indx_cb(timeline, channel1, cb, &output);
  ytp_timeline_prfx_cb(timeline, 5, "main/", cb, &output);
  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  ytp_timeline_prfx_cb_rm(timeline, 5, "main/", cb, &output);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "EFGH");
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_simple_subscription_rm_1) {
 data_simple_subscription_rm_1(true);
 data_simple_subscription_rm_1(false);
}

static void data_simple_subscription_rm_2(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output);
  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  ytp_timeline_indx_cb_rm(timeline, channel1, cb, &output);
  
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "EFGH");
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_simple_subscription_rm_2) {
  data_simple_subscription_rm_2(true);
  data_simple_subscription_rm_2(false);
}

static void data_multiple_channel_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2, consumer1, 0, 18, "secondary/channel2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "EFGH");
  rv = ytp_control_commit(ctrl, &it, producer1, channel2, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_multiple_channel_1) {
  data_multiple_channel_1(true);
  data_multiple_channel_1(false);
}

static void data_multiple_channel_2(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2, consumer1, 0, 18, "secondary/channel2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "EFGH");
  rv = ytp_control_commit(ctrl, &it, producer1, channel2, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel2, cb, &output);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "EFGH");

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_multiple_channel_2) {
  data_multiple_channel_2(true);
  data_multiple_channel_2(false);
}

static void data_multiple_producers_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer2 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer2, 9, "producer2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2, consumer1, 0, 18, "secondary/channel2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "EFGH");
  rv = ytp_control_commit(ctrl, &it, producer1, channel2, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "IJKL");
  rv = ytp_control_commit(ctrl, &it, producer2, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_multiple_producers_1) {
  data_multiple_producers_1(true);
  data_multiple_producers_1(false);
}

static void data_subscription_first_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  std::vector<std::string_view> output;

  auto cb1 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_prfx_cb(timeline, 5, "main/", cb1, &output);

  auto cb2 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, apr_size_t sz, const char *data) {
  };

  ytp_timeline_prfx_cb(timeline, 5, "main/", cb2, &output);

  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2, consumer1, 0, 18, "secondary/channel2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel3 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel3, consumer1, 0, 13, "main/channel3");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);
  ASSERT_NE(channel3, 0);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "EFGH");
  rv = ytp_control_commit(ctrl, &it, producer1, channel2, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "IJKL");
  rv = ytp_control_commit(ctrl, &it, producer1, channel3, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_subscription_first_1) {
  data_subscription_first_1(true);
  data_subscription_first_1(false);
}

static void data_subscription_first_2(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_prfx_cb(timeline, 5, "main/channel1", cb, &output);

  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2, consumer1, 0, 18, "secondary/channel2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "EFGH");
  rv = ytp_control_commit(ctrl, &it, producer1, channel2, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_subscription_first_2) {
  data_subscription_first_2(true);
  data_subscription_first_2(false);
}

static void peer_simple(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);
  
  std::vector<std::tuple<std::string_view, ytp_peer_t>> output;

  auto peer_cb = [](void *closure, ytp_peer_t peer, apr_size_t sz,
                    const char *name) {
    auto *output =
        (std::vector<std::tuple<std::string_view, ytp_peer_t>> *)closure;
    output->emplace_back(std::tuple<std::string_view, ytp_peer_t>(
        std::string_view(name, sz), peer));
  };
  ytp_timeline_peer_cb(timeline, peer_cb, &output);

  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer2 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer2, 9, "producer2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);

  const char *name;
  apr_size_t sz;
  rv = ytp_control_peer_name(ctrl, producer1, &sz, &name);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(std::string_view(name, sz), "producer1");
  rv = ytp_control_peer_name(ctrl, 555, &sz, &name);
  ASSERT_EQ(rv, YTP_STATUS_EPEERNOTFOUND);
  char error_str[128];
  ASSERT_STREQ(ytp_strerror(rv, error_str, sizeof(error_str)),
               "Peer not found");

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ytp_timeline_peer_cb_rm(timeline, peer_cb, &output);

  ytp_peer_t producer3 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer3, 9, "producer3");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(std::get<0>(output[0]), "producer1");
  ASSERT_EQ(std::get<1>(output[0]), producer1);

  ASSERT_EQ(std::get<0>(output[1]), "producer2");
  ASSERT_EQ(std::get<1>(output[1]), producer2);

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, peer_simple) {
  peer_simple(true);
  peer_simple(false);
}

static void idempotence_simple_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t consumer1_2 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1_2, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer1_2 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1_2, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel1_2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1_2, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(consumer1, consumer1_2);
  ASSERT_EQ(producer1, producer1_2);
  ASSERT_EQ(channel1, channel1_2);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "ABCD");
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  auto cb2 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output);
  ytp_timeline_indx_cb(timeline, channel1_2, cb, &output);
  ytp_timeline_indx_cb(timeline, channel1, cb2, &output);
  ytp_timeline_indx_cb(timeline, channel1_2, cb2, &output);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  ytp_peer_t consumer2 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer2, 9, "consumer2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t consumer2_2 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer2_2, 9, "consumer2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer2 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer2, 9, "producer2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer2_2 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer2_2, 9, "producer2");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_channel_t channel2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2, consumer1, 1000, 13, "main/channel2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel2_2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2_2, consumer1, 1000, 13, "main/channel2");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_timeline_indx_cb(timeline, channel2, cb, &output);
  ytp_timeline_indx_cb(timeline, channel2_2, cb, &output);
  ytp_timeline_indx_cb(timeline, channel2, cb2, &output);
  ytp_timeline_indx_cb(timeline, channel2_2, cb2, &output);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  strcpy(dst, "EFGH");
  rv = ytp_control_commit(ctrl, &it, producer2, channel2, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 4);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "ABCD");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);

  ASSERT_EQ(std::get<0>(output[2]), "EFGH");
  ASSERT_EQ(std::get<1>(output[2]), producer2);
  ASSERT_EQ(std::get<2>(output[2]), channel2);
  ASSERT_EQ(std::get<3>(output[2]), 1000);

  ASSERT_EQ(std::get<0>(output[3]), "EFGH");
  ASSERT_EQ(std::get<1>(output[3]), producer2);
  ASSERT_EQ(std::get<2>(output[3]), channel2);
  ASSERT_EQ(std::get<3>(output[3]), 1000);

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);


  ytp_yamal_t *yamal = NULL;
  rv = ytp_yamal_new2(&yamal, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(yamal, nullptr);
  rv = ytp_yamal_begin(yamal, &it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_FALSE(ytp_yamal_term(it));

  ytp_peer_t peer;
  ytp_channel_t channel;
  uint64_t time;
  apr_size_t sz;
  const char *data;
  rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(peer, 0);
  rv = ytp_yamal_next(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(it, nullptr);
  ASSERT_FALSE(ytp_yamal_term(it));

  rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(peer, 0);
  rv = ytp_yamal_next(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(it, nullptr);
  ASSERT_FALSE(ytp_yamal_term(it));

  rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(peer, consumer1);
  ASSERT_EQ(channel, YTP_CHANNEL_ANN);
  rv = ytp_yamal_next(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(it, nullptr);
  ASSERT_FALSE(ytp_yamal_term(it));

  rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(peer, producer1);
  ASSERT_EQ(channel, channel1);
  ASSERT_EQ(std::string_view(data, sz), "ABCD");
  rv = ytp_yamal_next(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(it, nullptr);
  ASSERT_FALSE(ytp_yamal_term(it));

  rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(peer, 0);
  rv = ytp_yamal_next(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(it, nullptr);
  ASSERT_FALSE(ytp_yamal_term(it));

  rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(peer, 0);
  rv = ytp_yamal_next(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(it, nullptr);
  ASSERT_FALSE(ytp_yamal_term(it));

  rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(peer, consumer1);
  ASSERT_EQ(channel, YTP_CHANNEL_ANN);
  rv = ytp_yamal_next(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(it, nullptr);
  ASSERT_FALSE(ytp_yamal_term(it));

  rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(peer, producer2);
  ASSERT_EQ(channel, channel2);
  ASSERT_EQ(std::string_view(data, sz), "EFGH");
  rv = ytp_yamal_next(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(it, nullptr);
  ASSERT_TRUE(ytp_yamal_term(it));

  rv = ytp_yamal_del(yamal);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, idempotence_simple_1) {
  idempotence_simple_1(true);
  idempotence_simple_1(false);
}

static void idempotence_simple_2(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);

  ytp_yamal_t *yamal = NULL;
  ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(yamal, nullptr);

  ytp_iterator_t it;
  rv = ytp_peer_name(yamal, &it, 5, "peer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  rv = ytp_peer_name(yamal, &it, 5, "peer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_yamal_del(yamal);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  
  ytp_control_t *ctrl = NULL;
  rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t>> output;

  auto peer_cb = [](void *closure, ytp_peer_t peer, apr_size_t sz,
                    const char *name) {
    auto *output =
        (std::vector<std::tuple<std::string_view, ytp_peer_t>> *)closure;
    output->emplace_back(std::tuple<std::string_view, ytp_peer_t>(
        std::string_view(name, sz), peer));
  };
  
  ytp_timeline_peer_cb(timeline, peer_cb, &output);
  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "peer1");

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, idempotence_simple_2) {
  idempotence_simple_2(true);
  idempotence_simple_2(false);
}

static void idempotence_simple_3(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);

  ytp_yamal_t *yamal = NULL;
  ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(yamal, nullptr);

  ytp_iterator_t it;
  rv = ytp_peer_name(yamal, &it, 5, "peer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  char *channel_name = NULL;
  rv = ytp_time_reserve(yamal, (char **)&channel_name, 8);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(channel_name, nullptr);
  std::memcpy(channel_name, "channel1", 8);
  rv = ytp_time_commit(yamal, &it, YTP_PEER_OFF, YTP_CHANNEL_ANN, 1000, channel_name);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  rv = ytp_time_reserve(yamal, (char **)&channel_name, 8);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(channel_name, nullptr);
  std::memcpy(channel_name, "channel1", 8);
  rv = ytp_time_commit(yamal, &it, YTP_PEER_OFF, YTP_CHANNEL_ANN, 1000, channel_name);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_yamal_del(yamal);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_control_t *ctrl = NULL;
  rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  std::vector<std::string_view> output;

  auto ch_cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                  uint64_t time, apr_size_t sz, const char *name) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(name, sz));
  };

  ytp_timeline_ch_cb(timeline, ch_cb, &output);
  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "channel1");
  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, idempotence_simple_3) {
  idempotence_simple_3(true);
  idempotence_simple_3(false);
}

static void idempotence_simple_4(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);

  ytp_yamal_t *yamal = NULL;
  ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(yamal, nullptr);

  ytp_iterator_t it;
  rv = ytp_peer_name(yamal, &it, 5, "peer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);

  char *channel_name = NULL;
  rv = ytp_time_reserve(yamal, (char **)&channel_name, 8);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(channel_name, nullptr);
  std::memcpy(channel_name, "channel1", 8);
  rv = ytp_time_commit(yamal, &it, YTP_PEER_OFF, YTP_CHANNEL_SUB, 1000, channel_name);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  rv = ytp_time_reserve(yamal, (char **)&channel_name, 8);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(channel_name, nullptr);
  std::memcpy(channel_name, "channel1", 8);
  rv = ytp_time_commit(yamal, &it, YTP_PEER_OFF, YTP_CHANNEL_SUB, 1000, channel_name);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_yamal_del(yamal);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_control_t *ctrl = NULL;
  rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  std::vector<std::string_view> output;

  auto sub_cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                  uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, YTP_CHANNEL_SUB, sub_cb, &output);
  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "channel1");
  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, idempotence_simple_4) {
  idempotence_simple_4(true);
  idempotence_simple_4(false);
}

static void leading_slash_test(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 9, "/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  auto cb1 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
  };
  ytp_timeline_indx_cb(timeline, channel1, cb1, nullptr);

  auto cb2 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
  };
  ytp_timeline_prfx_cb(timeline, 1, "a/", cb2, nullptr);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  ytp_channel_t channel2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2, consumer1, 0, 9, "/channel2");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, leading_slash_test) {
  leading_slash_test(true);
  leading_slash_test(false);
}

static void read_only_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);

  const char *tempdir = NULL;
  char *filetemplate;
  ytp_status_t rv = apr_temp_dir_get(&tempdir, pool);
  ASSERT_EQ(rv, APR_SUCCESS);
  filetemplate = apr_pstrcat(pool, tempdir, "/tempfileXXXXXX", NULL);
  // No APR_FOPEN_DELONCLOSE this time
  rv = apr_file_mktemp(&f, filetemplate,
                       APR_FOPEN_CREATE | APR_FOPEN_READ |
                       APR_FOPEN_WRITE | APR_FOPEN_EXCL, pool);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  const char *file_path;
  rv = apr_file_name_get(&file_path, f);

  ytp_control_t *ctrl = NULL;
  rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);
  
  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  memcpy(dst, "ABCD", 4);
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, consumer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  rv = apr_file_close(f);
  ASSERT_EQ(rv, APR_SUCCESS);

  rv = apr_file_perms_set(file_path, APR_FPROT_OS_DEFAULT);
  ASSERT_EQ(rv, APR_SUCCESS);
  rv = apr_file_open(&f, file_path,
                     APR_FOPEN_READ | APR_FOPEN_DELONCLOSE,
                     APR_FPROT_OS_DEFAULT,
                     pool);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  
  rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);
  
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_EREADONLY);
  ASSERT_NE(dst, nullptr);

  char error_str[128];
  ASSERT_STREQ(ytp_strerror(rv, error_str, sizeof(error_str)),
               "Yamal file is readonly");

  ytp_peer_t consumer2 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer2, 9, "consumer2");
  ASSERT_EQ(rv, YTP_STATUS_EREADONLY);
  ASSERT_STREQ(ytp_strerror(rv, error_str, sizeof(error_str)),
               "Yamal file is readonly");

  ytp_channel_t channel2 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel2, consumer1, 0, 13, "main/channel2");
  ASSERT_EQ(rv, YTP_STATUS_EREADONLY);
  ASSERT_STREQ(ytp_strerror(rv, error_str, sizeof(error_str)),
               "Yamal file is readonly");

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, read_only_1) {
  read_only_1(true);
  read_only_1(false);
}

static void data_iter_set_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t consumer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &consumer1, 9, "consumer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_peer_t producer1 = 0;
  rv = ytp_control_peer_decl(ctrl, &producer1, 9, "producer1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t channel1 = 0;
  rv = ytp_control_ch_decl(ctrl, &channel1, consumer1, 0, 13, "main/channel1");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  memcpy(dst, "ABCD", 4);
  ytp_iterator_t first_iter;
  rv = ytp_control_commit(ctrl, &first_iter, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  memcpy(dst, "EFGH", 4);
  ytp_iterator_t second_iter;
  rv = ytp_control_commit(ctrl, &second_iter, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 4);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  memcpy(dst, "IJKL", 4);
  ytp_iterator_t third_iter;
  rv = ytp_control_commit(ctrl, &third_iter, producer1, channel1, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, apr_size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output);
  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 3);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "EFGH");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);

  ASSERT_EQ(std::get<0>(output[2]), "IJKL");
  ASSERT_EQ(std::get<1>(output[2]), producer1);
  ASSERT_EQ(std::get<2>(output[2]), channel1);
  ASSERT_EQ(std::get<3>(output[2]), 1000);
  output.clear();

  ytp_timeline_iter_set(timeline, first_iter);
  apr_size_t first_off;
  rv = ytp_timeline_tell(timeline, &first_off, ytp_timeline_iter_get(timeline));
  ASSERT_EQ(rv, YTP_STATUS_OK);
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 3);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "EFGH");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);

  ASSERT_EQ(std::get<0>(output[2]), "IJKL");
  ASSERT_EQ(std::get<1>(output[2]), producer1);
  ASSERT_EQ(std::get<2>(output[2]), channel1);
  ASSERT_EQ(std::get<3>(output[2]), 1000);
  output.clear();

  ytp_timeline_iter_set(timeline, second_iter);
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(std::get<0>(output[0]), "EFGH");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "IJKL");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);
  output.clear();

  apr_size_t last_off;
  rv = ytp_timeline_tell(timeline, &last_off, ytp_timeline_iter_get(timeline));
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_timeline_iter_set(timeline, second_iter);
  ytp_iterator_t it;
  rv = ytp_timeline_seek(timeline, &it, last_off);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(output.size(), 0);
  rv = ytp_timeline_seek(timeline, &it, first_off);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);
  ASSERT_EQ(output.size(), 3);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "EFGH");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);

  ASSERT_EQ(std::get<0>(output[2]), "IJKL");
  ASSERT_EQ(std::get<1>(output[2]), producer1);
  ASSERT_EQ(std::get<2>(output[2]), channel1);
  ASSERT_EQ(std::get<3>(output[2]), 1000);
  output.clear();
  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_iter_set_1) {
  data_iter_set_1(true);
  data_iter_set_1(false);
}

static void data_callback_removal_1(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t peer = 0;
  rv = ytp_control_peer_decl(ctrl, &peer, 4, "peer");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t ch = 0;
  rv = ytp_control_ch_decl(ctrl, &ch, peer, 0, 2, "ch");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(peer, 0);
  ASSERT_NE(ch, 0);

  struct callback_t {
    static void c_callback(void *closure, ytp_peer_t peer,
                                   ytp_channel_t channel, uint64_t time, apr_size_t sz,
                                   const char *data) {
      auto &self = *reinterpret_cast<callback_t *>(closure);
      self.callback();
    }
    std::function<void()> callback;
  };
  std::multiset<int> execution;
  std::multiset<int> expected;

  callback_t c1{[&]() { execution.insert(1); }};

  callback_t c2{[&]() { execution.insert(2); }};

  callback_t c4{[&]() { execution.insert(4); }};

  callback_t c3{[&]() {
    execution.insert(3);
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c4);
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c1);
  }};

  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c1);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c2);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c3);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c4);

  char *dst = NULL;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 1);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  ytp_iterator_t it;
  rv = ytp_control_commit(ctrl, &it, peer, ch, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  expected = {1, 2, 3};
  EXPECT_EQ(execution, expected);
  execution.clear();

  rv = ytp_control_reserve(ctrl, (char **)&dst, 1);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  rv = ytp_control_commit(ctrl, &it, peer, ch, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  expected = {2, 3};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c2);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 1);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  rv = ytp_control_commit(ctrl, &it, peer, ch, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  expected = {3};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_callback_removal_1) {
  data_callback_removal_1(true);
  data_callback_removal_1(false);
}

static void data_callback_removal_2(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_control_t *ctrl = NULL;
  ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(ctrl, nullptr);
  ytp_timeline_t *timeline;
  rv = ytp_timeline_new(&timeline, ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(timeline, nullptr);

  ytp_peer_t peer = 0;
  rv = ytp_control_peer_decl(ctrl, &peer, 4, "peer");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_channel_t ch = 0;
  rv = ytp_control_ch_decl(ctrl, &ch, peer, 0, 2, "ch");
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(peer, 0);
  ASSERT_NE(ch, 0);

  struct callback_t {
    static void c_callback(void *closure, ytp_peer_t peer,
                                   ytp_channel_t channel, uint64_t time, apr_size_t sz,
                                   const char *data) {
      auto &self = *reinterpret_cast<callback_t *>(closure);
      self.callback();
    }
    std::function<void()> callback;
  };

  std::vector<int> execution;
  std::vector<int> expected;

  callback_t c1{[&]() {
    execution.emplace_back(1);
    bool new_data;
    while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
      ;
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(new_data, false);
    }};

  callback_t c3{[&]() { execution.emplace_back(3); }};

  callback_t c2{[&]() {
    execution.emplace_back(2);
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c3);
  }};

  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c1);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c2);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c3);

  char *dst = NULL;
  ytp_iterator_t it;
  rv = ytp_control_reserve(ctrl, (char **)&dst, 1);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  rv = ytp_control_commit(ctrl, &it, peer, ch, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  rv = ytp_control_reserve(ctrl, (char **)&dst, 1);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  rv = ytp_control_commit(ctrl, &it, peer, ch, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  bool new_data;
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  expected = {1, 1, 2, 2};
  EXPECT_EQ(execution, expected);
  execution.clear();

  rv = ytp_control_reserve(ctrl, (char **)&dst, 1);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(dst, nullptr);
  rv = ytp_control_commit(ctrl, &it, peer, ch, 1000, dst);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  while ( !(rv = ytp_timeline_poll(timeline, &new_data)) && new_data)
    ;
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(new_data, false);

  expected = {1, 2};
  EXPECT_EQ(execution, expected);
  execution.clear();
  ytp_timeline_del(timeline);
  rv = ytp_control_del(ctrl);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(timeline, data_callback_removal_2) {
  data_callback_removal_2(true);
  data_callback_removal_2(false);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
