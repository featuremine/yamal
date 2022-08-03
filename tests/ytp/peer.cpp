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
 * @brief File contains tests for YTP peer layer
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/peer.h>
#include <ytp/yamal.h>

#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>

using namespace std;

struct test_msg {
  unsigned index = 0;
};

const unsigned test_batch = 10000;
const unsigned max_misses = 100;
const unsigned batch_count = 500;
const unsigned test_size = test_batch * batch_count;

TEST(peer, sequential) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  EXPECT_TRUE(fmc_fvalid(fd));
  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    for (unsigned i = 1; i < test_size; ++i) {
      ytp_peer_t peer = i + 1000;
      auto *msg = (test_msg *)ytp_peer_reserve(yamal, sizeof(test_msg), &error);
      ASSERT_NE(msg, nullptr);
      msg->index = i;
      ASSERT_NE(ytp_peer_commit(yamal, peer, msg, &error), nullptr);
    }
    ytp_yamal_del(yamal, &error);
    EXPECT_EQ(error, nullptr);
  }

  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    unsigned count = 1;
    unsigned last_idx = 0;
    auto iter = ytp_yamal_begin(yamal, &error);
    ASSERT_NE(iter, nullptr);
    for (; !ytp_yamal_term(iter); iter = ytp_yamal_next(yamal, iter, &error)) {
      ytp_peer_t peer;
      size_t sz;
      test_msg *msg;
      ytp_peer_read(yamal, iter, &peer, &sz, (const char **)&msg, &error);
      ASSERT_EQ(error, nullptr);
      last_idx = msg->index;
      ASSERT_EQ(msg->index, count);
      ASSERT_EQ(peer, count + 1000);
      ++count;
    }
    ytp_yamal_del(yamal, &error);
    EXPECT_EQ(error, nullptr);
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
  }
  fmc_fclose(fd, &error);
}

TEST(peer, name) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  EXPECT_TRUE(fmc_fvalid(fd));
  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    ASSERT_TRUE(ytp_peer_name(yamal, 4, "ABCD", &error));
    ytp_yamal_del(yamal, &error);
    EXPECT_EQ(error, nullptr);
  }

  {
    auto *yamal = ytp_yamal_new(fd, &error);
    ASSERT_NE(yamal, nullptr);
    auto iter = ytp_yamal_begin(yamal, &error);
    ASSERT_NE(iter, nullptr);
    ASSERT_NE(ytp_yamal_begin(yamal, &error), ytp_yamal_end(yamal, &error));
    ytp_peer_t peer;
    size_t sz;
    const char *name;
    ytp_peer_read(yamal, iter, &peer, &sz, &name, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(peer, 0);
    ASSERT_EQ(sz, 4);
    ASSERT_EQ(std::string_view(name, sz), "ABCD");
    ytp_yamal_del(yamal, &error);
    EXPECT_EQ(error, nullptr);
  }
  fmc_fclose(fd, &error);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
