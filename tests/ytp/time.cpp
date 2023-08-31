/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
      uint64_t time = i + 3000;
      auto *msg = (test_msg *)ytp_time_reserve(yamal, sizeof(test_msg), &error);
      ASSERT_NE(msg, nullptr);
      msg->index = i;
      ASSERT_NE(ytp_time_commit(yamal, time, msg, 0, &error), nullptr);
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
      int64_t time;
      size_t sz;
      test_msg *msg;
      uint64_t seqno;
      ytp_time_read(yamal, iter, &seqno, &time, &sz, (const char **)&msg,
                    &error);
      ASSERT_EQ(error, nullptr);
      last_idx = msg->index;
      ASSERT_EQ(msg->index, count);
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

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
