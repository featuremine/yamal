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
 * @author Featuremine Corporation
 * @date 28 Apr 2021
 * @brief File contains tests for YTP channel layer
 *
 * @see http://www.featuremine.com
 */

#include <stdbool.h> 
#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_pools.h> // apr_pool_t
#include <apr_file_io.h> // apr_file_t
#include <ytp/yamal.h>
#include <ytp/peer.h>
#include <ytp/channel.h>
#include <ytp/errno.h> // ytp_status_t
#include "utils.h" // test_init() make_temp_file()
#include "gtestwrap.hpp"

struct test_msg {
  unsigned index = 0;
};

const unsigned test_batch = 10000;
const unsigned max_misses = 100;
const unsigned batch_count = 500;
const unsigned test_size = test_batch * batch_count;

static void sequential(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  {
    ytp_yamal_t *yamal = NULL;
    ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(yamal, nullptr);
    ytp_iterator_t it;
    for (unsigned i = 1; i < test_size; ++i) {
      ytp_peer_t peer = i + 1000;
      ytp_channel_t channel = i + 2000;
      test_msg *msg = NULL;
      rv = ytp_channel_reserve(yamal, (char **)&msg, sizeof(test_msg));
      ASSERT_EQ(rv, YTP_STATUS_OK);
      ASSERT_NE(msg, nullptr);
      msg->index = i;
      rv = ytp_channel_commit(yamal, &it, peer, channel, msg);
      ASSERT_EQ(rv, YTP_STATUS_OK);
    }
    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
  }

  {
    ytp_yamal_t *yamal = NULL;
    ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(yamal, nullptr);
    unsigned count = 1;
    unsigned last_idx = 0;
    ytp_iterator_t it;
    rv = ytp_yamal_begin(yamal, &it);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(it, nullptr);
    for (; !ytp_yamal_term(it); rv = ytp_yamal_next(yamal, &it, it)) {
      ASSERT_EQ(rv, YTP_STATUS_OK);
      ytp_peer_t peer;
      ytp_channel_t channel;
      apr_size_t sz;
      test_msg *data;
      rv = ytp_channel_read(yamal, it, &peer, &channel, &sz, (const char **)&data);
      ASSERT_EQ(rv, YTP_STATUS_OK);
      ASSERT_EQ(sz, sizeof(test_msg));
      ASSERT_NE(data, nullptr);
      last_idx = data->index;
      ASSERT_EQ(data->index, count);
      ASSERT_EQ(peer, count + 1000);
      ASSERT_EQ(channel, count + 2000);
      ++count;
    }
    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
  }
  test_end(f, pool);
}

TEST(channel, sequential) {
  sequential(true);
  sequential(false);
}

static void peer_name(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  {
    ytp_yamal_t *yamal = NULL;
    ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(yamal, nullptr);
    ytp_iterator_t it;
    rv = ytp_peer_name(yamal, &it, 4, "ABCD");
    ASSERT_EQ(rv, YTP_STATUS_OK);
    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
  }
  {
    ytp_yamal_t *yamal = NULL;
    ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(yamal, nullptr);
    ytp_iterator_t it;
    rv = ytp_yamal_begin(yamal, &it);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(it, nullptr);
    ytp_iterator_t end;
    rv = ytp_yamal_end(yamal, &end);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(it, end);
    ytp_peer_t peer;
    ytp_channel_t channel;
    apr_size_t sz;
    const char *name;
    rv = ytp_channel_read(yamal, it, &peer, &channel, &sz, (const char **)&name);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(peer, 0);
    ASSERT_EQ(sz, 4);
    ASSERT_EQ(std::string_view(name, sz), "ABCD");
    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
  }
  test_end(f, pool);
}

TEST(channel, peer_name) {
  peer_name(true);
  peer_name(false);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
