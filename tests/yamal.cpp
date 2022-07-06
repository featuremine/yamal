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
 * @brief File contains tests for YTP yamal layer
 *
 * @see http://www.featuremine.com
 */

#include <ytp/yamal.h>
#include <ytp/errno.h> // ytp_status_t
#include <stdbool.h>
#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_pools.h> // apr_pool_t
#include <apr_file_io.h> // apr_file_t

#include <thread>
#include <chrono>
#include <string_view>

#include "utils.h" // test_init() make_temp_file()
#include "gtestwrap.hpp"

using namespace std::chrono_literals;

struct test_msg {
  unsigned index = 0;
  char check[7] = {"hello\0"};
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
      test_msg *msg = NULL;
      rv = ytp_yamal_reserve(yamal, (char **)&msg, sizeof(test_msg));
      ASSERT_EQ(rv, YTP_STATUS_OK);
      ASSERT_NE(msg, nullptr);
      msg->index = i;
      rv = ytp_yamal_commit(yamal, &it, msg);
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
      apr_size_t sz;
      test_msg *data;
      rv = ytp_yamal_read(yamal, it, &sz, (const char **)&data);
      ASSERT_EQ(rv, YTP_STATUS_OK);
      ASSERT_EQ(sz, sizeof(test_msg));
      ASSERT_NE(data, nullptr);
      last_idx = data->index;
      ASSERT_EQ(data->index, count++);
    }
    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
  }
  test_end(f, pool);
}

TEST(yamal, sequential) {
  sequential(true);
  sequential(false);
}

static void threaded(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  std::thread t1([f, enable_thread]() {
    ytp_yamal_t *yamal = NULL;
    ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(yamal, nullptr);
    unsigned long index = 0;
    ytp_iterator_t it;
    for (unsigned i = 0; i < batch_count; ++i) {
      for (unsigned j = 0; j < test_batch; ++j) {
        test_msg *msg = NULL;
        rv = ytp_yamal_reserve(yamal, (char **)&msg, sizeof(test_msg));
        ASSERT_EQ(rv, YTP_STATUS_OK);
        ASSERT_NE(msg, nullptr);
        msg->index = index++;
        rv = ytp_yamal_commit(yamal, &it, msg);
        ASSERT_EQ(rv, YTP_STATUS_OK);
      }
      std::this_thread::sleep_for(10us);
    }
    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
  });

  std::thread t2([f, enable_thread]() {
    ytp_yamal_t *yamal = NULL;
    ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(yamal, nullptr);
    unsigned count = 0;
    unsigned misses = 0;
    unsigned last_idx = 0;
    ytp_iterator_t it;
    rv = ytp_yamal_begin(yamal, &it);
    ASSERT_EQ(rv, YTP_STATUS_OK);
    ASSERT_NE(it, nullptr);
    while (count < test_size) {
      for (; !ytp_yamal_term(it); rv = ytp_yamal_next(yamal, &it, it)) {
        ASSERT_EQ(rv, YTP_STATUS_OK);
        apr_size_t sz;
        test_msg *data;
        rv = ytp_yamal_read(yamal, it, &sz, (const char **)&data);
        ASSERT_EQ(rv, YTP_STATUS_OK);
        ASSERT_EQ(sz, sizeof(test_msg));
        ASSERT_NE(data, nullptr);
        last_idx = data->index;
        ++count;
      }
      if (misses > max_misses)
        break;
      std::this_thread::sleep_for(100us);
    }
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
    rv = ytp_yamal_del(yamal);
    ASSERT_EQ(rv, YTP_STATUS_OK);
  });

  t1.join();
  t2.join();

  test_end(f, pool);
}

TEST(yamal, threaded) {
  threaded(true);
  threaded(false);
}

static void removal(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_yamal_t *yamal = NULL;
  ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(yamal, nullptr);
  ytp_iterator_t it;
  rv = ytp_yamal_begin(yamal, &it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_iterator_t end;
  rv = ytp_yamal_end(yamal, &end);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(it, end);
  rv = ytp_yamal_remove(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_EINVOFFSET);
  char error_str[128];
  ASSERT_STREQ(ytp_strerror(rv, error_str, sizeof(error_str)),
               "Invalid offset");

  test_msg *zeromsg = NULL;
  rv = ytp_yamal_reserve(yamal, (char **)&zeromsg, sizeof(test_msg));
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(zeromsg, nullptr);
  zeromsg->index = 0;
  ytp_iterator_t zeroit;
  rv = ytp_yamal_commit(yamal, &zeroit, zeromsg);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(zeroit, nullptr);

  test_msg *oneptr = NULL;
  rv = ytp_yamal_reserve(yamal, (char **)&oneptr, sizeof(test_msg));
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(oneptr, nullptr);
  oneptr->index = 1;
  ytp_iterator_t oneit;
  rv = ytp_yamal_commit(yamal, &oneit, oneptr);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(oneit, nullptr);

  test_msg *twoptr = NULL;
  rv = ytp_yamal_reserve(yamal, (char **)&twoptr, sizeof(test_msg));
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(twoptr, nullptr);
  twoptr->index = 2;
  ytp_iterator_t twoit;
  rv = ytp_yamal_commit(yamal, &twoit, twoptr);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(twoit, nullptr);

  test_msg *threeptr = NULL;
  rv = ytp_yamal_reserve(yamal, (char **)&threeptr, sizeof(test_msg));
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(threeptr, nullptr);
  threeptr->index = 3;
  ytp_iterator_t threeit;
  rv = ytp_yamal_commit(yamal, &threeit, threeptr);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(threeit, nullptr);

  rv = ytp_yamal_remove(yamal, &zeroit, zeroit);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(zeroit, nullptr);

  ytp_iterator_t iter;
  rv = ytp_yamal_begin(yamal, &iter);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(iter, nullptr);

  test_msg *offmsg;
  apr_size_t sz;
  rv = ytp_yamal_read(yamal, iter, &sz, (const char **)&offmsg);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(offmsg, nullptr);

  ASSERT_EQ(offmsg->index, 1LL);

  rv = ytp_yamal_remove(yamal, &twoit, twoit);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(twoit, nullptr);

  rv = ytp_yamal_next(yamal, &iter, iter);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(iter, nullptr);

  test_msg *nextoffmsg;
  rv = ytp_yamal_read(yamal, iter, &sz, (const char **)&nextoffmsg);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(nextoffmsg, nullptr);
  ASSERT_EQ(nextoffmsg->index, 3UL);

  rv = ytp_yamal_del(yamal);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(yamal, removal) {
  removal(true);
  removal(false);
}

static void seektell(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_yamal_t *yamal = NULL;
  ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(yamal, nullptr);

  ytp_iterator_t it;
  rv = ytp_yamal_begin(yamal, &it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ytp_iterator_t end;
  rv = ytp_yamal_end(yamal, &end);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(it, end);
  rv = ytp_yamal_remove(yamal, &it, it);
  ASSERT_EQ(rv, YTP_STATUS_EINVOFFSET);
  char error_str[128];
  ASSERT_STREQ(ytp_strerror(rv, error_str, sizeof(error_str)),
               "Invalid offset");

  test_msg *zeromsg = NULL;
  rv = ytp_yamal_reserve(yamal, (char **)&zeromsg, sizeof(test_msg));
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(zeromsg, nullptr);
  zeromsg->index = 0;
  ytp_iterator_t zeroit;
  rv = ytp_yamal_commit(yamal, &zeroit, zeromsg);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(zeroit, nullptr);

  test_msg *oneptr = NULL;
  rv = ytp_yamal_reserve(yamal, (char **)&oneptr, sizeof(test_msg));
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(oneptr, nullptr);
  oneptr->index = 1;
  ytp_iterator_t oneit;
  rv = ytp_yamal_commit(yamal, &oneit, oneptr);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(oneit, nullptr);

  apr_size_t sz;
  ytp_iterator_t begin_it;
  rv = ytp_yamal_begin(yamal, &begin_it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  rv = ytp_yamal_tell(yamal, &sz, begin_it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(sz, 0);
  rv = ytp_yamal_tell(yamal, &sz, zeroit);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(sz, 0);
  rv = ytp_yamal_tell(yamal, &sz, oneit);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(sz, 32);
  ytp_iterator_t end_it;
  rv = ytp_yamal_end(yamal, &end_it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  rv = ytp_yamal_tell(yamal, &sz, end_it);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(sz, 72);

  rv = ytp_yamal_seek(yamal, &it, 0);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(it, begin_it);
  rv = ytp_yamal_seek(yamal, &it, 32);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(it, oneit);
  rv = ytp_yamal_seek(yamal, &it, 72);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(it, end_it);

  rv = ytp_yamal_del(yamal);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(yamal, seektell) {
  seektell(true);
  seektell(false);
}

static void invalid_format_throws(bool enable_thread) {
  std::string path("data/synth_book.ore");
  apr_file_t *fb = NULL;
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);

  ytp_status_t rv = apr_file_open(&fb, path.c_str(), APR_FOPEN_READ | APR_FOPEN_BINARY, 
                     APR_FPROT_OS_DEFAULT, pool);
  ASSERT_EQ(rv, APR_SUCCESS);

  {
    char buffer[2048];
    apr_size_t bytes = sizeof(buffer);
    while(bytes) {
      rv = apr_file_read(fb, buffer, &bytes);
      if(rv == APR_EOF) break;
      ASSERT_EQ(rv, APR_SUCCESS);
      rv = apr_file_write(f, buffer, &bytes);
      ASSERT_EQ(rv, APR_SUCCESS);
    }
    apr_file_flush(f);
  }
  apr_file_close(fb);

  ytp_yamal_t *yamal = NULL;
  rv = ytp_yamal_new2(&yamal, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_EINVFORMAT);
  char error_str[128];
  ASSERT_STREQ(ytp_strerror(rv, error_str, sizeof(error_str)),
               "Invalid yamal file format");
  ASSERT_EQ(yamal, nullptr);

  rv = ytp_yamal_del(yamal); // it does nothing
  ASSERT_EQ(rv, YTP_STATUS_OK);
  test_end(f, pool);
}

TEST(yamal, invalid_format_throws) {
  invalid_format_throws(true);
  invalid_format_throws(false);
}

static void magic_number(bool enable_thread) {
  apr_pool_t *pool = NULL;
  apr_file_t *f = NULL;
  test_init(&pool);
  make_temp_file(&f, pool);
  ytp_yamal_t *yamal = NULL;
  ytp_status_t rv = ytp_yamal_new2(&yamal, f, enable_thread);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_NE(yamal, nullptr);

  rv = ytp_yamal_del(yamal);
  ASSERT_EQ(rv, YTP_STATUS_OK);

  apr_off_t offset = 24;
  char magic_number[8];
  apr_size_t bytes = sizeof(magic_number);
  rv = apr_file_seek(f, APR_SET, &offset);
  ASSERT_EQ(rv, YTP_STATUS_OK);
  ASSERT_EQ(offset, 24);
  rv = apr_file_read(f, magic_number, &bytes);
  ASSERT_EQ(rv, APR_SUCCESS);
  ASSERT_EQ(bytes, sizeof(magic_number));
  ASSERT_EQ(std::string_view(magic_number, 8), "YAMAL000");

  test_end(f, pool);
}

TEST(yamal, magic_number) {
  magic_number(true);
  magic_number(false);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
