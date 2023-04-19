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
 * @brief File contains tests for YTP time layer
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/channel.h>
#include <ytp/control.h>
#include <ytp/peer.h>
#include <ytp/time.h>
#include <ytp/yamal.h>

#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>

using namespace std;

struct test_msg {
  unsigned index = 0;
};

const unsigned test_batch = 10000;
const unsigned batch_count = 500;
const unsigned test_size = test_batch * batch_count;

TEST(time, sequential) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  {
    auto *ctrl = ytp_control_new(fd, &error);
    ASSERT_EQ(error, nullptr);
    for (unsigned i = 1; i < test_size; ++i) {
      ytp_peer_t peer = i + 1000;
      ytp_channel_t channel = i + 2000;
      uint64_t time = i + 3000;
      auto *msg =
          (test_msg *)ytp_control_reserve(ctrl, sizeof(test_msg), &error);
      ASSERT_EQ(error, nullptr);
      msg->index = i;
      ASSERT_NE(ytp_control_commit(ctrl, peer, channel, time, msg, &error),
                nullptr);
      ASSERT_EQ(error, nullptr);
    }
    ytp_control_del(ctrl, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    auto *ctrl = ytp_control_new(fd, &error);
    ASSERT_EQ(error, nullptr);
    ytp_iterator_t it = ytp_control_begin(ctrl, &error);
    ASSERT_EQ(error, nullptr);
    unsigned count = 1;
    unsigned last_idx = 0;
    while (!ytp_yamal_term(it)) {
      ytp_peer_t peer;
      ytp_channel_t channel;
      uint64_t time;
      size_t sz;
      test_msg *msg;
      ytp_control_read(ctrl, it, &peer, &channel, &time, &sz,
                       (const char **)&msg, &error);
      ASSERT_EQ(error, nullptr);

      it = ytp_control_next(ctrl, it, &error);
      ASSERT_EQ(error, nullptr);

      last_idx = msg->index;
      ASSERT_EQ(msg->index, count);
      ASSERT_EQ(peer, count + 1000);
      ASSERT_EQ(channel, count + 2000);
      ASSERT_EQ(time, count + 3000);
      ++count;
    }
    ytp_control_del(ctrl, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
  }
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(time, control_msg) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  {
    auto *ctrl = ytp_control_new(fd, &error);
    ASSERT_EQ(error, nullptr);

    ytp_control_dir(ctrl, 100, 300, 4, "ABCD", &error);
    ASSERT_EQ(error, nullptr);

    ytp_control_ch_decl(ctrl, 100, 300, 4, "ABCD", &error);
    ASSERT_EQ(error, nullptr);

    ytp_control_sub(ctrl, 101, 300, 4, "ABCD", &error);
    ASSERT_EQ(error, nullptr);

    ytp_control_del(ctrl, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t time;
    size_t sz;
    const char *data;

    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    auto iter = ytp_yamal_begin(yamal, &error);
    ASSERT_NE(iter, nullptr);

    ytp_time_read(yamal, iter, &peer, &channel, &time, &sz, &data, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(peer, 100);
    ASSERT_EQ(channel, 200);
    ASSERT_EQ(std::string_view(data, sz), "ABCD");

    iter = ytp_yamal_next(yamal, iter, &error);
    ASSERT_NE(iter, nullptr);
    ASSERT_EQ(error, nullptr);

    ytp_time_read(yamal, iter, &peer, &channel, &time, &sz, &data, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(channel, YTP_CHANNEL_ANN);
    ASSERT_EQ(std::string_view(data, sz), "ABCD");

    iter = ytp_yamal_next(yamal, iter, &error);
    ASSERT_NE(iter, nullptr);
    ASSERT_EQ(error, nullptr);

    ytp_time_read(yamal, iter, &peer, &channel, &time, &sz, &data, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(peer, 101);
    ASSERT_EQ(channel, YTP_CHANNEL_SUB);
    ASSERT_EQ(std::string_view(data, sz), "ABCD");

    iter = ytp_yamal_next(yamal, iter, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_NE(iter, nullptr);

    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  }
  fmc_fclose(fd, &error);
}

TEST(time, peer_name) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  {
    auto *ctrl = ytp_control_new(fd, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(ytp_control_peer_decl(ctrl, 4, "ABCD", &error));
    ytp_control_del(ctrl, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    ASSERT_EQ(error, nullptr);
    auto iter = ytp_yamal_begin(yamal, &error);
    ASSERT_NE(iter, nullptr);
    ASSERT_EQ(error, nullptr);
    ASSERT_NE(ytp_yamal_begin(yamal, &error), ytp_yamal_end(yamal, &error));
    ASSERT_EQ(error, nullptr);
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t time;
    size_t sz;
    const char *data;
    ytp_time_read(yamal, iter, &peer, &channel, &time, &sz, &data, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(peer, 0);
    ASSERT_EQ(sz, 4);
    ASSERT_EQ(std::string_view(data, sz), "ABCD");
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  }
  fmc_fclose(fd, &error);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
