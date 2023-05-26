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
 * @file yamal.cpp
 * @author Federico Ravchina
 * @date 28 Apr 2021
 * @brief File contains tests for YTP sequence API
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/control.h>
#include <ytp/sequence.h>
#include <ytp/yamal.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>

using namespace std;

TEST(sequence, data_simple_subscription_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
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

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_simple_subscription_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
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

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_simple_subscription_rm_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
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

  ytp_sequence_prfx_cb(seq, 5, "main/", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ytp_sequence_prfx_cb_rm(seq, 5, "main/", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_simple_subscription_rm_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
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

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ytp_sequence_indx_cb_rm(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_multiple_channel_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_multiple_channel_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_sequence_indx_cb(seq, channel2, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "EFGH");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_multiple_channel_3) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_multiple_channel_4) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_sequence_indx_cb(seq, channel2, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "EFGH");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_multiple_producers_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer2 = ytp_sequence_peer_decl(seq, 9, "producer2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_sequence_commit(seq, producer2, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_multiple_producers_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer2 = ytp_sequence_peer_decl(seq, 9, "producer2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_sequence_commit(seq, producer2, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_subscription_first_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb1 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_sequence_prfx_cb(seq, 5, "main/", cb1, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto cb2 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, size_t sz, const char *data) {};

  ytp_sequence_prfx_cb(seq, 5, "main/", cb2, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel3 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel3", &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);
  ASSERT_NE(channel3, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_sequence_commit(seq, producer1, channel3, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_subscription_first_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_sequence_prfx_cb(seq, 13, "main/channel1", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_sequence_commit(seq, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, peer_simple) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t>> output;

  auto peer_cb = [](void *closure, ytp_peer_t peer, size_t sz,
                    const char *name) {
    auto *output =
        (std::vector<std::tuple<std::string_view, ytp_peer_t>> *)closure;
    output->emplace_back(std::tuple<std::string_view, ytp_peer_t>(
        std::string_view(name, sz), peer));
  };

  ytp_sequence_peer_cb(seq, peer_cb, &output, &error);

  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  auto producer2 = ytp_sequence_peer_decl(seq, 9, "producer2", &error);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);

  const char *name;
  size_t sz;
  ytp_sequence_peer_name(seq, producer1, &sz, &name, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(std::string_view(name, sz), "producer1");
  ytp_sequence_peer_name(seq, 555, &sz, &name, &error);
  ASSERT_NE(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

  ytp_sequence_peer_cb_rm(seq, peer_cb, &output, &error);

  (void)ytp_sequence_peer_decl(seq, 9, "producer3", &error);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(std::get<0>(output[0]), "producer1");
  ASSERT_EQ(std::get<1>(output[0]), producer1);

  ASSERT_EQ(std::get<0>(output[1]), "producer2");
  ASSERT_EQ(std::get<1>(output[1]), producer2);

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
}

TEST(sequence, leading_slash_test) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 9, "/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  ytp_sequence_indx_cb(
      seq, channel1,
      [](void *closure, ytp_peer_t peer, ytp_channel_t channel, uint64_t time,
         size_t sz, const char *data) {},
      nullptr, &error);
  ASSERT_EQ(error, nullptr);

  ytp_sequence_prfx_cb(
      seq, 1, "a/",
      [](void *closure, ytp_peer_t peer, ytp_channel_t channel, uint64_t time,
         size_t sz, const char *data) {},
      nullptr, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  (void)ytp_sequence_ch_decl(seq, consumer1, 0, 9, "/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, read_only_1) {
  fmc_error_t *error;

  fs::path filename = "readonly.ytp";

  fs::remove(filename);
  auto fd = fmc_fopen(filename.c_str(), fmc_fmode::READWRITE, &error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "ABCD", 4);
  ytp_sequence_commit(seq, consumer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
  fd = fmc_fopen(filename.c_str(), fmc_fmode::READ, &error);
  seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  (void)ytp_sequence_peer_decl(seq, 9, "consumer2", &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  (void)ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel2", &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
  fs::remove(filename);
}

TEST(sequence, data_iter_set_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "ABCD", 4);
  auto first_iter =
      ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "EFGH", 4);
  auto second_iter =
      ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "IJKL", 4);
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
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

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

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

  ytp_sequence_set_it(seq, first_iter);

  auto first_off = ytp_sequence_tell(seq, ytp_sequence_get_it(seq), &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

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

  ytp_sequence_set_it(seq, second_iter);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

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

  auto last_off = ytp_sequence_tell(seq, ytp_sequence_get_it(seq), &error);
  ASSERT_EQ(error, nullptr);

  ytp_sequence_set_it(seq, second_iter);

  ytp_sequence_seek(seq, last_off, &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 0);

  ytp_sequence_seek(seq, first_off, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

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

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_iter_set_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto first_off = ytp_sequence_tell(seq, ytp_sequence_get_it(seq), &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_sequence_peer_decl(seq, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_sequence_peer_decl(seq, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_sequence_reserve(seq, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "ABCD", 4);
  ytp_sequence_commit(seq, producer1, channel1, 1000, dst, &error);
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

  ytp_sequence_indx_cb(seq, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);
  output.clear();

  ytp_sequence_ch_decl(seq, consumer1, 0, 13, "main/channel2", &error);

  ytp_sequence_seek(seq, first_off, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);
  output.clear();

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_iter_set_3) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto first_off = ytp_sequence_tell(seq, ytp_sequence_get_it(seq), &error);
  ASSERT_EQ(error, nullptr);

  ytp_sequence_seek(seq, first_off, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(sequence, data_iter_set_4) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *seq = ytp_sequence_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto first_off = ytp_sequence_tell(seq, ytp_sequence_get_it(seq), &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

  ytp_sequence_seek(seq, first_off, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_sequence_poll(seq, &error)) {
    ASSERT_EQ(error, nullptr);
  }

  ytp_sequence_del(seq, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
