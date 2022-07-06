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
 * @brief File contains tests for YTP time layer
 *
 * @see http://www.featuremine.com
 */

#include <ytp/yamal.h>
#include <ytp/peer.h>
#include <ytp/channel.h>
#include <ytp/time.h>
#include <ytp/control.h>
#include <ytp/errno.h> // ytp_status_t
#include <stdbool.h> 
#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_pools.h> // apr_pool_t
#include <apr_file_io.h> // apr_file_t
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
    ytp_control_t *ctrl = NULL;
    ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(ctrl, nullptr);
    ytp_iterator_t it;
    for (unsigned i = 1; i < test_size; ++i) {
      ytp_peer_t peer = i + 1000;
      ytp_channel_t channel = i + 2000;
      uint64_t time = i + 3000;
      test_msg *msg = NULL;
      rv = ytp_control_reserve(ctrl, (char **)&msg, sizeof(test_msg));
      ASSERT_EQ(rv, YTP_STATUS_OK);
      ASSERT_NE(msg, nullptr);
      msg->index = i;
      rv = ytp_control_commit(ctrl, &it, peer, channel, time, msg);
      ASSERT_EQ(rv, YTP_STATUS_OK);
    }
    rv = ytp_control_del(ctrl);
    ASSERT_EQ(rv, YTP_STATUS_OK);
  }

  {
    ytp_control_t *ctrl = NULL;
    ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(ctrl, nullptr);
    unsigned count = 1;
    unsigned last_idx = 0;
    ytp_iterator_t it;
    rv = ytp_control_begin(ctrl, &it);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(it, nullptr);
    while (!ytp_control_term(it)) {
      ytp_peer_t peer;
      ytp_channel_t channel;
      uint64_t time;
      apr_size_t sz;
      test_msg *msg;
      rv = ytp_control_read(ctrl, it, &peer, &channel, &time, &sz, (const char **)&msg);
      ASSERT_EQ(rv, YTP_STATUS_OK);
      ASSERT_NE(msg, nullptr);
      rv = ytp_control_next(ctrl, &it, it);
      ASSERT_EQ(rv, YTP_STATUS_OK);
      last_idx = msg->index;
      ASSERT_EQ(msg->index, count);
      ASSERT_EQ(peer, count + 1000);
      ASSERT_EQ(channel, count + 2000);
      ASSERT_EQ(time, count + 3000);
      ++count;

    }
    rv = ytp_control_del(ctrl);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
  }
  test_end(f, pool);
}

TEST(control, sequential) {
  sequential(true);
  sequential(false);
}

static void control_msg(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  {
    ytp_control_t *ctrl = NULL;
    ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(ctrl, nullptr);
    rv = ytp_control_dir(ctrl, 100, 300, 4, "ABCD");
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ytp_channel_t channel;
    rv = ytp_control_ch_decl(ctrl, &channel, 100, 300, 4, "ABCD");
    ASSERT_EQ(rv, YTP_STATUS_OK);
    rv = ytp_control_sub(ctrl, 101, 300, 4, "ABCD");
    ASSERT_EQ(rv, YTP_STATUS_OK);
    rv = ytp_control_del(ctrl);
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
    uint64_t time;
    apr_size_t sz;
    const char *data;
    rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, (const char **)&data);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(peer, 100);
    ASSERT_EQ(channel, YTP_CHANNEL_DIR);
    ASSERT_EQ(std::string_view(data, sz), "ABCD");

    rv = ytp_yamal_next(yamal, &it, it);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(it, nullptr);
    rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, (const char **)&data);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(channel, YTP_CHANNEL_ANN);
    ASSERT_EQ(std::string_view(data, sz), "ABCD");

    rv = ytp_yamal_next(yamal, &it, it);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(it, nullptr);
    rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, (const char **)&data);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(peer, 101);
    ASSERT_EQ(channel, YTP_CHANNEL_SUB);
    ASSERT_EQ(std::string_view(data, sz), "ABCD");

    rv = ytp_yamal_next(yamal, &it, it);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(it, nullptr);

    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
  }
  test_end(f, pool);
}

TEST(control, control_msg) {
  control_msg(true);
  control_msg(false);
}

static void peer_name(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  {
    ytp_control_t *ctrl = NULL;
    ytp_status_t rv = ytp_control_new2(&ctrl, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(ctrl, nullptr);
    ytp_peer_t peer;
    rv = ytp_control_peer_decl(ctrl, &peer, 4, "ABCD");
    ASSERT_EQ(rv, YTP_STATUS_OK);
    rv = ytp_control_del(ctrl);
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
    uint64_t time;
    apr_size_t sz;
    const char *name;
    rv = ytp_time_read(yamal, it, &peer, &channel, &time, &sz, (const char **)&name);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(peer, 0);
    ASSERT_EQ(sz, 4);
    ASSERT_EQ(std::string_view(name, sz), "ABCD");
    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
  }
  test_end(f, pool);
}

TEST(control, peer_name) {
  peer_name(true);
  peer_name(false);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
