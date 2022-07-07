/******************************************************************************

        COPYRIGHT (c) 2018 by Featuremine Corporation.
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
 * @brief File contains tests for YTP yamal layer
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/yamal.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>
using namespace std;

struct test_msg {
  unsigned index = 0;
  char check[7] = {"hello\0"};
};

const unsigned test_batch = 10000;
const unsigned max_misses = 100;
const unsigned batch_count = 500;
const unsigned test_size = test_batch * batch_count;

static void sequential(bool enable_thread) {
  auto *error = (fmc_error_t *)1;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  ASSERT_TRUE(fmc_fvalid(fd));
  {
    error = (fmc_error_t *)1;
    auto *yamal = ytp_yamal_new_2(fd, enable_thread, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_NE(yamal, nullptr);
    for (unsigned i = 1; i < test_size; ++i) {
      error = (fmc_error_t *)1;
      auto *msg =
          (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
      ASSERT_EQ(error, nullptr);
      ASSERT_NE(msg, nullptr);
      msg->index = i;
      error = (fmc_error_t *)1;
      ASSERT_NE(ytp_yamal_commit(yamal, msg, &error), nullptr);
      ASSERT_EQ(error, nullptr);
    }
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    error = (fmc_error_t *)1;
    auto *yamal = ytp_yamal_new_2(fd, enable_thread, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_NE(yamal, nullptr);
    unsigned count = 1;
    unsigned last_idx = 0;
    error = (fmc_error_t *)1;
    auto it = ytp_yamal_begin(yamal, &error);
    ASSERT_EQ(error, nullptr);
    for (; !ytp_yamal_term(it); it = ytp_yamal_next(yamal, it, &error)) {
      size_t sz;
      test_msg *data;
      error = (fmc_error_t *)1;
      ytp_yamal_read(yamal, it, &sz, (const char **)&data, &error);
      ASSERT_EQ(error, nullptr);
      ASSERT_EQ(sz, sizeof(test_msg));
      last_idx = data->index;
      ASSERT_EQ(data->index, count++);
    }
    error = (fmc_error_t *)1;
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
  }

  error = (fmc_error_t *)1;
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(yamal, sequential) {
  sequential(true);
  sequential(false);
}

static void threaded(bool enable_thread) {
  fmc_error_t *error;
  error = (fmc_error_t *)1;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  ASSERT_TRUE(fmc_fvalid(fd));
  thread t1([fd, enable_thread]() {
    fmc_error_t *error = (fmc_error_t *)1;
    auto *yamal = ytp_yamal_new_2(fd, enable_thread, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_NE(yamal, nullptr);
    unsigned long index = 0;
    for (unsigned i = 0; i < batch_count; ++i) {
      for (unsigned j = 0; j < test_batch; ++j) {
        error = (fmc_error_t *)1;
        auto *msg =
            (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
        ASSERT_EQ(error, nullptr);
        ASSERT_NE(msg, nullptr);
        msg->index = index++;
        error = (fmc_error_t *)1;
        ASSERT_NE(ytp_yamal_commit(yamal, msg, &error), nullptr);
        ASSERT_EQ(error, nullptr);
      }
      this_thread::sleep_for(10us);
    }
    error = (fmc_error_t *)1;
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  });

  thread t2([fd, enable_thread]() {
    fmc_error_t *error = (fmc_error_t *)1;
    auto *yamal = ytp_yamal_new_2(fd, enable_thread, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_NE(yamal, nullptr);
    ytp_iterator_t iter;
    unsigned count = 0;
    unsigned misses = 0;
    unsigned last_idx = 0;
    auto it = ytp_yamal_begin(yamal, &error);
    while (count < test_size) {
      for (; !ytp_yamal_term(it); it = ytp_yamal_next(yamal, it, &error)) {
        size_t sz;
        test_msg *data;
        error = (fmc_error_t *)1;
        ytp_yamal_read(yamal, it, &sz, (const char **)&data, &error);
        ASSERT_EQ(error, nullptr);
        ASSERT_NE(data, nullptr);
        last_idx = data->index;
        ++count;
      }
      if (misses > max_misses)
        break;
      this_thread::sleep_for(100us);
    }
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count, test_size);
    error = (fmc_error_t *)1;
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  });

  t1.join();
  t2.join();

  fmc_fclose(fd, &error);
}

TEST(yamal, threaded) {
  threaded(true);
  threaded(false);
}

static void removal(bool enable_thread) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  ASSERT_TRUE(fmc_fvalid(fd));
  auto *yamal = ytp_yamal_new_2(fd, enable_thread, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);
  ytp_iterator_t iter;
  auto it = ytp_yamal_begin(yamal, &error);
  ASSERT_EQ(error, nullptr);
  auto end = ytp_yamal_end(yamal, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(it, end);
  ASSERT_EQ(ytp_yamal_remove(yamal, it, &error), nullptr);
  ASSERT_NE(error, nullptr);

  auto *zeromsg =
      (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(zeromsg, nullptr);
  zeromsg->index = 0;
  auto zeroit = ytp_yamal_commit(yamal, zeromsg, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(zeroit, nullptr);

  auto *oneptr = (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(oneptr, nullptr);
  oneptr->index = 1;
  auto oneit = ytp_yamal_commit(yamal, oneptr, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(oneit, nullptr);

  auto *twoptr = (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(twoptr, nullptr);
  twoptr->index = 2;
  auto *twoit = ytp_yamal_commit(yamal, twoptr, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(twoit, nullptr);

  auto *threeptr =
      (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(threeptr, nullptr);
  threeptr->index = 3;
  auto threeit = ytp_yamal_commit(yamal, threeptr, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(threeit, nullptr);

  ASSERT_NE(ytp_yamal_remove(yamal, zeroit, &error), nullptr);
  ASSERT_EQ(error, nullptr);

  iter = ytp_yamal_begin(yamal, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(iter, nullptr);

  test_msg *offmsg;
  size_t sz;
  ytp_yamal_read(yamal, iter, &sz, (const char **)&offmsg, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(offmsg, nullptr);

  ASSERT_EQ(offmsg->index, 1LL);

  ASSERT_NE(ytp_yamal_remove(yamal, twoit, &error), nullptr);
  ASSERT_EQ(error, nullptr);

  iter = ytp_yamal_next(yamal, iter, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(iter, nullptr);

  test_msg *nextoffmsg;
  ytp_yamal_read(yamal, iter, &sz, (const char **)&nextoffmsg, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(nextoffmsg, nullptr);

  ASSERT_EQ(nextoffmsg->index, 3UL);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
}

TEST(yamal, removal) {
  removal(true);
  removal(false);
}

static void seektell(bool enable_thread) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  ASSERT_TRUE(fmc_fvalid(fd));
  auto *yamal = ytp_yamal_new_2(fd, enable_thread, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);
  ytp_iterator_t iter;
  auto it = ytp_yamal_begin(yamal, &error);
  ASSERT_EQ(error, nullptr);
  auto end = ytp_yamal_end(yamal, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(it, end);
  ASSERT_EQ(ytp_yamal_remove(yamal, it, &error), nullptr);
  ASSERT_NE(error, nullptr);

  auto *zeromsg =
      (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(zeromsg, nullptr);
  zeromsg->index = 0;
  auto zeroit = ytp_yamal_commit(yamal, zeromsg, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(zeroit, nullptr);

  auto *oneptr = (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(oneptr, nullptr);
  oneptr->index = 1;
  auto oneit = ytp_yamal_commit(yamal, oneptr, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(oneit, nullptr);

  error = (fmc_error_t *)1;
  ASSERT_EQ(ytp_yamal_tell(yamal, ytp_yamal_begin(yamal, &error), &error), 0);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_tell(yamal, zeroit, &error), 0);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_tell(yamal, oneit, &error), 32);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_tell(yamal, ytp_yamal_end(yamal, &error), &error), 72);
  ASSERT_EQ(error, nullptr);

  error = (fmc_error_t *)1;
  ASSERT_EQ(ytp_yamal_seek(yamal, 0, &error), ytp_yamal_begin(yamal, &error));
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_seek(yamal, 32, &error), oneit);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_seek(yamal, 72, &error), ytp_yamal_end(yamal, &error));
  ASSERT_EQ(error, nullptr);

  error = (fmc_error_t *)1;
  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  error = (fmc_error_t *)1;
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(yamal, seektell) {
  seektell(true);
  seektell(false);
}

static void invalid_format_throws(bool enable_thread) {
  std::string path =
      fs::path(__FILE__).parent_path().append("../tests/data/synth_book.ore");
  FILE *ore_fp = fopen(path.c_str(), "rb");
  ASSERT_NE(ore_fp, nullptr);
  FILE *yamal_fp = tmpfile();
  ASSERT_NE(yamal_fp, nullptr);
  {
    char buffer[2048];
    size_t bytes;

    while (0 < (bytes = fread(buffer, 1, sizeof(buffer), ore_fp))) {
      fwrite(buffer, 1, bytes, yamal_fp);
    }
    fflush(yamal_fp);
  }
  fclose(ore_fp);
  off_t bytes = 0;
  struct stat fileinfo = {0};
  ASSERT_EQ(bytes, fileinfo.st_size);
  fmc_error_t *error;
  auto *yamal =
      ytp_yamal_new_2(fmc_fd_get(yamal_fp, &error), enable_thread, &error);
  ASSERT_EQ(yamal, nullptr);
  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 27),
            "invalid yamal file format (");
  fclose(yamal_fp);
}

TEST(yamal, invalid_format_throws) {
  invalid_format_throws(true);
  invalid_format_throws(false);
}

static void magic_number(bool enable_thread) {
  fmc_error_t *error;
  FILE *fp = tmpfile();
  auto fd = fmc_fd_get(fp, &error);
  ASSERT_TRUE(fmc_fvalid(fd));
  error = (fmc_error_t *)1;
  auto *yamal = ytp_yamal_new_2(fmc_fd_get(fp, &error), enable_thread, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);
  error = (fmc_error_t *)1;
  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  char magic_number[8];
  fseek(fp, 24, SEEK_SET);
  fread(magic_number, sizeof(magic_number), 1, fp);
  ASSERT_EQ(string_view(magic_number, 8), "YAMAL000");
  fclose(fp);
}

TEST(yamal, magic_number) {
  magic_number(true);
  magic_number(false);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
