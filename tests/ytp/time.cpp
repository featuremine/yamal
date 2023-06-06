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
  EXPECT_TRUE(fmc_fvalid(fd));
  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    for (unsigned i = 1; i < test_size; ++i) {
      ytp_peer_t peer = i + 1000;
      ytp_channel_t channel = i + 2000;
      uint64_t time = i + 3000;
      auto *msg = (test_msg *)ytp_time_reserve(yamal, sizeof(test_msg), &error);
      ASSERT_NE(msg, nullptr);
      msg->index = i;
      ASSERT_NE(ytp_time_commit(yamal, peer, channel, time, msg, &error),
                nullptr);
    }
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    unsigned count = 1;
    unsigned last_idx = 0;
    auto iter = ytp_yamal_begin(yamal, 0, &error);
    auto end = ytp_yamal_end(yamal, 0, &error);
    ASSERT_NE(iter, nullptr);
    ASSERT_NE(end, nullptr);
    for (; !ytp_yamal_term(iter); iter = ytp_yamal_next(yamal, iter, &error)) {
      ytp_peer_t peer;
      ytp_channel_t channel;
      uint64_t time;
      size_t sz;
      test_msg *msg;
      ytp_time_read(yamal, iter, &peer, &channel, &time, &sz,
                    (const char **)&msg, &error);
      ASSERT_EQ(error, nullptr);
      last_idx = msg->index;
      ASSERT_EQ(msg->index, count);
      ASSERT_EQ(peer, count + 1000);
      ASSERT_EQ(channel, count + 2000);
      ASSERT_EQ(time, count + 3000);
      ++count;
    }
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
  }
  fmc_fclose(fd, &error);
}

TEST(time, peer_name) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  EXPECT_TRUE(fmc_fvalid(fd));
  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    ASSERT_TRUE(ytp_peer_name(yamal, 4, "ABCD", &error));
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    auto iter = ytp_yamal_begin(yamal, 0, &error);
    ASSERT_NE(iter, nullptr);
    ASSERT_NE(ytp_yamal_begin(yamal, 0, &error),
              ytp_yamal_end(yamal, 0, &error));
    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t time;
    size_t sz;
    const char *name;
    ytp_time_read(yamal, iter, &peer, &channel, &time, &sz, &name, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(peer, 0);
    ASSERT_EQ(sz, 4);
    ASSERT_EQ(std::string_view(name, sz), "ABCD");
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  }
  fmc_fclose(fd, &error);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
