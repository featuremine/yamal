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
 * @author Federico Ravchina
 * @date 10 Jan 2022
 * @brief File contains tests for YTP timeline API
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/control.h>
#include <ytp/peer.h>
#include <ytp/timeline.h>
#include <ytp/yamal.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>

using namespace std;

TEST(timeline, data_simple_subscription_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_simple_subscription_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_simple_subscription_rm_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_prfx_cb(timeline, 5, "main/", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ytp_timeline_prfx_cb_rm(timeline, 5, "main/", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_simple_subscription_rm_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ytp_timeline_indx_cb_rm(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_channel_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_channel_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel2, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "EFGH");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_channel_3) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_channel_4) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel2, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "EFGH");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_producers_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_control_commit(ctrl, producer2, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_producers_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_control_commit(ctrl, producer2, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_subscription_first_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb1 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_prfx_cb(timeline, 5, "main/", cb1, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto cb2 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, size_t sz, const char *data) {};

  ytp_timeline_prfx_cb(timeline, 5, "main/", cb2, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel3 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel3", &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);
  ASSERT_NE(channel3, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_control_commit(ctrl, producer1, channel3, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_subscription_first_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_prfx_cb(timeline, 13, "main/channel1", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, peer_simple) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t>> output;

  auto peer_cb = [](void *closure, ytp_peer_t peer, size_t sz,
                    const char *name) {
    auto *output =
        (std::vector<std::tuple<std::string_view, ytp_peer_t>> *)closure;
    output->emplace_back(std::tuple<std::string_view, ytp_peer_t>(
        std::string_view(name, sz), peer));
  };

  ytp_timeline_peer_cb(timeline, peer_cb, &output, &error);

  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  auto producer2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);

  const char *name;
  size_t sz;
  ytp_control_peer_name(ctrl, producer1, &sz, &name, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(std::string_view(name, sz), "producer1");
  ytp_control_peer_name(ctrl, 555, &sz, &name, &error);
  ASSERT_NE(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ytp_timeline_peer_cb_rm(timeline, peer_cb, &output, &error);

  auto producer3 = ytp_control_peer_decl(ctrl, 9, "producer3", &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(std::get<0>(output[0]), "producer1");
  ASSERT_EQ(std::get<1>(output[0]), producer1);

  ASSERT_EQ(std::get<0>(output[1]), "producer2");
  ASSERT_EQ(std::get<1>(output[1]), producer2);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
}

TEST(timeline, idempotence_simple_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  auto consumer1_2 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  auto producer1_2 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  auto channel1_2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(consumer1, consumer1_2);
  ASSERT_EQ(producer1, producer1_2);
  ASSERT_EQ(channel1, channel1_2);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  auto cb2 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ytp_timeline_indx_cb(timeline, channel1_2, cb, &output, &error);
  ytp_timeline_indx_cb(timeline, channel1, cb2, &output, &error);
  ytp_timeline_indx_cb(timeline, channel1_2, cb2, &output, &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  auto consumer2 = ytp_control_peer_decl(ctrl, 9, "consumer2", &error);
  auto consumer2_2 = ytp_control_peer_decl(ctrl, 9, "consumer2", &error);
  auto producer2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  auto producer2_2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 1000, 13, "main/channel2", &error);
  auto channel2_2 =
      ytp_control_ch_decl(ctrl, consumer1, 1000, 13, "main/channel2", &error);

  ytp_timeline_indx_cb(timeline, channel2, cb, &output, &error);
  ytp_timeline_indx_cb(timeline, channel2_2, cb, &output, &error);
  ytp_timeline_indx_cb(timeline, channel2, cb2, &output, &error);
  ytp_timeline_indx_cb(timeline, channel2_2, cb2, &output, &error);

  dst = ytp_control_reserve(ctrl, 4, &error);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer2, channel2, 1000, dst, &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

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

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  ytp_peer_t peer;
  ytp_channel_t channel;
  uint64_t time;
  char control;
  size_t sz;
  const char *data;
  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_NE(yamal, nullptr);

  auto *it = ytp_yamal_begin(yamal, &error);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, 0);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, 0);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, consumer1);
  ASSERT_EQ(channel, YTP_CHANNEL_ANN);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, producer1);
  ASSERT_EQ(channel, channel1);
  ASSERT_EQ(std::string_view(data, sz), "ABCD");
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, 0);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, 0);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, consumer1);
  ASSERT_EQ(channel, YTP_CHANNEL_ANN);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, producer2);
  ASSERT_EQ(channel, channel2);
  ASSERT_EQ(std::string_view(data, sz), "EFGH");
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_TRUE(ytp_yamal_term(it));

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
}

TEST(timeline, idempotence_simple_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  ytp_peer_name(yamal, 5, "peer1", &error);
  ytp_peer_name(yamal, 5, "peer1", &error);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t>> output;

  auto peer_cb = [](void *closure, ytp_peer_t peer, size_t sz,
                    const char *name) {
    auto *output =
        (std::vector<std::tuple<std::string_view, ytp_peer_t>> *)closure;
    output->emplace_back(std::tuple<std::string_view, ytp_peer_t>(
        std::string_view(name, sz), peer));
  };

  ytp_timeline_peer_cb(timeline, peer_cb, &output, &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "peer1");

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, idempotence_simple_3) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  ytp_peer_name(yamal, 5, "peer1", &error);

  auto *channel_name = ytp_time_reserve(yamal, 8, &error);
  ASSERT_NE(channel_name, nullptr);
  std::memcpy(channel_name, "channel1", 8);
  ASSERT_NE(ytp_time_commit(yamal, YTP_PEER_OFF, YTP_CHANNEL_ANN, 1000,
                            channel_name, &error),
            nullptr);

  channel_name = ytp_time_reserve(yamal, 8, &error);
  ASSERT_NE(channel_name, nullptr);
  std::memcpy(channel_name, "channel1", 8);
  ASSERT_NE(ytp_time_commit(yamal, YTP_PEER_OFF, YTP_CHANNEL_ANN, 1000,
                            channel_name, &error),
            nullptr);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto ch_cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                  uint64_t time, size_t sz, const char *name) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(name, sz));
  };

  ytp_timeline_ch_cb(timeline, ch_cb, &output, &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "channel1");

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, idempotence_simple_4) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  ytp_peer_name(yamal, 5, "peer1", &error);

  auto *channel_name = ytp_time_reserve(yamal, 8, &error);
  ASSERT_NE(channel_name, nullptr);
  std::memcpy(channel_name, "channel1", 8);
  ASSERT_NE(ytp_time_commit(yamal, YTP_PEER_OFF, YTP_CHANNEL_SUB, 1000,
                            channel_name, &error),
            nullptr);

  channel_name = ytp_time_reserve(yamal, 8, &error);
  ASSERT_NE(channel_name, nullptr);
  std::memcpy(channel_name, "channel1", 8);
  ASSERT_NE(ytp_time_commit(yamal, YTP_PEER_OFF, YTP_CHANNEL_SUB, 1000,
                            channel_name, &error),
            nullptr);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto sub_cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                   uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, YTP_CHANNEL_SUB, sub_cb, &output, &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "channel1");

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, leading_slash_test) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 9, "/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  ytp_timeline_indx_cb(timeline, channel1,
                       [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                          uint64_t time, size_t sz, const char *data) {},
                       nullptr, &error);
  ASSERT_EQ(error, nullptr);

  ytp_timeline_prfx_cb(timeline, 1, "a/",
                       [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                          uint64_t time, size_t sz, const char *data) {},
                       nullptr, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 9, "/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, read_only_1) {
  fmc_error_t *error;

  fs::path filename = "readonly.ytp";

  fs::remove(filename);
  auto fd = fmc_fopen(filename.c_str(), fmc_fmode::READWRITE, &error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "ABCD", 4);
  ytp_control_commit(ctrl, consumer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
  fd = fmc_fopen(filename.c_str(), fmc_fmode::READ, &error);

  ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  auto consumer2 = ytp_control_peer_decl(ctrl, 9, "consumer2", &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel2", &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
  fs::remove(filename);
}

TEST(timeline, data_iter_set_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "ABCD", 4);
  auto first_iter =
      ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "EFGH", 4);
  auto second_iter =
      ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "IJKL", 4);
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;

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

  auto first_off =
      ytp_timeline_tell(timeline, ytp_timeline_iter_get(timeline), &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;

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

  while (ytp_timeline_poll(timeline, &error))
    ;

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

  auto last_off =
      ytp_timeline_tell(timeline, ytp_timeline_iter_get(timeline), &error);
  ASSERT_EQ(error, nullptr);

  ytp_timeline_iter_set(timeline, second_iter);

  ytp_timeline_seek(timeline, last_off, &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 0);

  ytp_timeline_seek(timeline, first_off, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;

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

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_callback_removal_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto peer = ytp_control_peer_decl(ctrl, 4, "peer", &error);
  ASSERT_EQ(error, nullptr);
  auto ch = ytp_control_ch_decl(ctrl, peer, 0, 2, "ch", &error);
  ASSERT_EQ(error, nullptr);

  struct callback_t {
    static void c_callback(void *closure, ytp_peer_t peer,
                           ytp_channel_t channel, uint64_t time, size_t sz,
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
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c4, &error);
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c1, &error);
  }};

  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c1, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c2, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c3, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c4, &error);

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {1, 2, 3};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {2, 3};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c2, &error);

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {3};
  EXPECT_EQ(execution, expected);
  execution.clear();
}

TEST(timeline, data_callback_removal_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto peer = ytp_control_peer_decl(ctrl, 4, "peer", &error);
  ASSERT_EQ(error, nullptr);
  auto ch = ytp_control_ch_decl(ctrl, peer, 0, 2, "ch", &error);
  ASSERT_EQ(error, nullptr);

  struct callback_t {
    static void c_callback(void *closure, ytp_peer_t peer,
                           ytp_channel_t channel, uint64_t time, size_t sz,
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
    while (ytp_timeline_poll(timeline, &error))
      ;
  }};

  callback_t c3{[&]() { execution.emplace_back(3); }};

  callback_t c2{[&]() {
    execution.emplace_back(2);
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c3, &error);
  }};

  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c1, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c2, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c3, &error);

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {1, 1, 2, 2};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {1, 2};
  EXPECT_EQ(execution, expected);
  execution.clear();
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
