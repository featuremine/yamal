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
 * @brief File contains tests for YTP yamal layer
 *
 * @see http://www.featuremine.com
 */

#include <random>
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
      ASSERT_NE(ytp_yamal_commit(yamal, msg, i % 2, &error), nullptr);
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
    auto it = ytp_yamal_begin(yamal, 0, &error);
    ASSERT_EQ(error, nullptr);
    for (; !ytp_yamal_term(it); it = ytp_yamal_next(yamal, it, &error)) {
      size_t sz;
      uint64_t seqno;
      test_msg *data;
      error = (fmc_error_t *)1;
      ytp_yamal_read(yamal, it, &seqno, &sz, (const char **)&data, &error);
      ASSERT_EQ(error, nullptr);
      ASSERT_EQ(sz, sizeof(test_msg));
      last_idx = data->index;
      ASSERT_EQ(data->index, count * 2);
      ASSERT_EQ(seqno, count);
      ++count;
    }
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(last_idx, test_size - 2);
    ASSERT_EQ(count * 2, test_size);

    count = 0;
    last_idx = 0;
    it = ytp_yamal_begin(yamal, 1, &error);
    ASSERT_EQ(error, nullptr);
    for (; !ytp_yamal_term(it); it = ytp_yamal_next(yamal, it, &error)) {
      size_t sz;
      uint64_t seqno;
      test_msg *data;
      error = (fmc_error_t *)1;
      ytp_yamal_read(yamal, it, &seqno, &sz, (const char **)&data, &error);
      ASSERT_EQ(error, nullptr);
      ASSERT_EQ(sz, sizeof(test_msg));
      last_idx = data->index;
      ASSERT_EQ(data->index, count * 2 + 1);
      ASSERT_EQ(seqno, count + 1);
      ++count;
    }
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(last_idx, test_size - 1);
    ASSERT_EQ(count * 2, test_size);

    error = (fmc_error_t *)1;
    ytp_yamal_del(yamal, &error);
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
        ASSERT_NE(ytp_yamal_commit(yamal, msg, 0, &error), nullptr);
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
    unsigned count = 0;
    unsigned misses = 0;
    unsigned last_idx = 0;
    auto it = ytp_yamal_begin(yamal, 0, &error);
    while (count < test_size) {
      for (; !ytp_yamal_term(it); it = ytp_yamal_next(yamal, it, &error)) {
        size_t sz;
        uint64_t seqno;
        test_msg *data;
        error = (fmc_error_t *)1;
        ytp_yamal_read(yamal, it, &seqno, &sz, (const char **)&data, &error);
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

static void seektell(bool enable_thread) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  ASSERT_TRUE(fmc_fvalid(fd));
  auto *yamal = ytp_yamal_new_2(fd, enable_thread, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);
  auto it = ytp_yamal_begin(yamal, 0, &error);
  ASSERT_EQ(error, nullptr);
  auto end = ytp_yamal_end(yamal, 0, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(it, end);

  auto *zeromsg =
      (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(zeromsg, nullptr);
  zeromsg->index = 0;
  auto zeroit = ytp_yamal_commit(yamal, zeromsg, 0, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(zeroit, nullptr);

  it = ytp_yamal_next(yamal, zeroit, &error);
  ASSERT_EQ(error, nullptr);
  end = ytp_yamal_end(yamal, 0, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(it, end);

  auto *oneptr = (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(oneptr, nullptr);
  oneptr->index = 1;
  auto oneit = ytp_yamal_commit(yamal, oneptr, 0, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(oneit, nullptr);

  error = (fmc_error_t *)1;
  ASSERT_EQ(ytp_yamal_tell(yamal, ytp_yamal_begin(yamal, 0, &error), &error),
            16);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_tell(yamal, zeroit, &error), 16);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_tell(yamal, oneit, &error), 536);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_tell(yamal, ytp_yamal_end(yamal, 0, &error), &error),
            584);
  ASSERT_EQ(error, nullptr);

  error = (fmc_error_t *)1;
  ASSERT_EQ(ytp_yamal_seek(yamal, 16, &error),
            ytp_yamal_begin(yamal, 0, &error));
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_seek(yamal, 536, &error), oneit);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_seek(yamal, 584, &error),
            ytp_yamal_end(yamal, 0, &error));
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
  std::string path(__FILE__);
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
  fseek(fp, 0, SEEK_SET);
  ASSERT_EQ(fread(magic_number, sizeof(magic_number), 1, fp), 1);
  ASSERT_EQ(string_view(magic_number, 8), "YAMAL001");
  fclose(fp);
}

TEST(yamal, magic_number) {
  magic_number(true);
  magic_number(false);
}

static void allocate_out_of_range_page(bool enable_thread) {
  fmc_error_t *error;
  FILE *fp = tmpfile();
  auto fd = fmc_fd_get(fp, &error);
  ASSERT_TRUE(fmc_fvalid(fd));
  error = (fmc_error_t *)1;
  auto *yamal = ytp_yamal_new_2(fmc_fd_get(fp, &error), enable_thread, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);
  ytp_yamal_allocate_page(yamal, 99999999999ull, &error);
  ASSERT_NE(error, nullptr);
  ASSERT_EQ(std::string_view(fmc_error_msg(error), 25),
            "page index out of range (");
  error = (fmc_error_t *)1;
  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);
  fclose(fp);
}

TEST(yamal, allocate_out_of_range_page) {
  allocate_out_of_range_page(true);
  allocate_out_of_range_page(false);
}

TEST(yamal, commit_sublist) {
  fmc_error_t *error;

  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  std::atomic<int> pending_producers;
  std::atomic<size_t> total_messages = 0;
  auto producer = [&]() {
    std::random_device rd;
    std::mt19937 mt_rand(rd());
    std::uniform_int_distribution<unsigned> batch_rng(1, 1000);
    std::uniform_int_distribution<int> case_rng(0, 1);
    std::uniform_int_distribution<size_t> sleep_rng(0, 1000);

    auto *yamal = ytp_yamal_new_2(fd, false, &error);
    ASSERT_EQ(error, nullptr);

    for (unsigned i = 0; i < batch_count; ++i) {
      switch (case_rng(mt_rand)) {
      case 0: {
        auto batch_sz = batch_rng(mt_rand);
        for (unsigned j = 0; j < batch_sz; ++j) {
          auto *msg =
              (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
          ASSERT_EQ(error, nullptr);
          msg->index = 0;
          strncpy(msg->check, "end", sizeof(msg->check));
          ytp_yamal_commit(yamal, msg, 0, &error);
          ASSERT_EQ(error, nullptr);
          ++total_messages;
        }
      } break;
      case 1: {
        test_msg *first_node = nullptr;
        test_msg *last_node = nullptr;
        auto batch_sz = batch_rng(mt_rand);
        for (unsigned j = 0; j < batch_sz; ++j) {
          if (sleep_rng(mt_rand) == 0) {
            this_thread::sleep_for(std::chrono::milliseconds(10));
          }
          auto *msg =
              (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
          ASSERT_EQ(error, nullptr);
          msg->index = j;
          ytp_yamal_sublist_commit(yamal, (void **)&first_node,
                                   (void **)&last_node, msg, &error);
          ASSERT_EQ(error, nullptr);
          ++total_messages;
        }
        strncpy(last_node->check, "end", sizeof(last_node->check));
        ytp_yamal_commit(yamal, first_node, 0, &error);
        ASSERT_EQ(error, nullptr);
      } break;
      }
    }
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
    --pending_producers;
  };

  size_t total_processed = 0;
  auto consumer = [&]() {
    uint64_t expected_seqno = 0;
    auto *yamal = ytp_yamal_new_2(fd, false, &error);
    ASSERT_EQ(error, nullptr);

    auto it = ytp_yamal_begin(yamal, 0, &error);
    ASSERT_EQ(error, nullptr);

    while (true) {
      if (ytp_yamal_term(it)) {
        if (pending_producers.load() == 0) {
          break;
        }
        continue;
      }

      for (unsigned expected_idx = 0;; ++expected_idx) {
        ++total_processed;

        size_t sz;
        test_msg *data;
        uint64_t seqno;

        ytp_yamal_read(yamal, it, &seqno, &sz, (const char **)&data, &error);
        ASSERT_EQ(error, nullptr);

        auto expected_prev = it;
        it = ytp_yamal_next(yamal, it, &error);
        ASSERT_EQ(error, nullptr);

        auto prev = ytp_yamal_prev(yamal, it, &error);
        ASSERT_EQ(error, nullptr);
        ASSERT_EQ(prev, expected_prev);

        ASSERT_EQ(data->index, expected_idx);
        ++expected_seqno;
        ASSERT_EQ(seqno, expected_seqno);
        if (strcmp(data->check, "end") == 0) {
          break;
        }

        ASSERT_FALSE(ytp_yamal_term(it));
      }
    }
    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  };

  pending_producers = 2;
  thread t1(producer);
  thread t2(producer);
  consumer();
  t2.join();
  t1.join();

  fmc_fclose(fd, &error);

  ASSERT_EQ(total_processed, total_messages);
}

TEST(yamal, allocate_closable) {
  fmc_error_t *error;
  FILE *fp = tmpfile();
  auto fd = fmc_fd_get(fp, &error);
  ASSERT_TRUE(fmc_fvalid(fd));
  error = (fmc_error_t *)1;
  auto *yamal =
      ytp_yamal_new_3(fmc_fd_get(fp, &error), false, YTP_CLOSABLE, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);

  // Validate that we cannot open a closable sequence as unclosable
  ASSERT_EQ(
      ytp_yamal_new_3(fmc_fd_get(fp, &error), false, YTP_UNCLOSABLE, &error),
      nullptr);
  ASSERT_NE(error, nullptr);
  fmc_error_clear(&error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(ytp_yamal_new_2(fmc_fd_get(fp, &error), false, &error), nullptr);
  ASSERT_NE(error, nullptr);

  ytp_yamal_del(yamal, &error);
  yamal = nullptr;
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(yamal, nullptr);
  fclose(fp);

  fp = tmpfile();
  fd = fmc_fd_get(fp, &error);
  ASSERT_TRUE(fmc_fvalid(fd));
  error = (fmc_error_t *)1;
  yamal =
      ytp_yamal_new_3(fmc_fd_get(fp, &error), false, YTP_UNCLOSABLE, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);

  // Validate that we cannot close an unclosable sequence
  ytp_yamal_close(yamal, 0, &error);
  ASSERT_NE(error, nullptr);

  // Validate that we cannot open a unclosable sequence as closable
  ASSERT_EQ(
      ytp_yamal_new_3(fmc_fd_get(fp, &error), false, YTP_CLOSABLE, &error),
      nullptr);
  ASSERT_NE(error, nullptr);
  fmc_error_clear(&error);
  ASSERT_EQ(error, nullptr);

  // Validate that we can open a unclosable sequence with the ytp_yamal_new_2
  // API
  auto *yamal2 = ytp_yamal_new_2(fmc_fd_get(fp, &error), false, &error);
  ASSERT_NE(yamal2, nullptr);
  ASSERT_EQ(error, nullptr);
  ytp_yamal_del(yamal2, &error);
  ASSERT_EQ(error, nullptr);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);
  fclose(fp);
}

TEST(yamal, closable_empty_list) {
  fmc_error_t *error;
  FILE *fp = tmpfile();
  auto fd = fmc_fd_get(fp, &error);
  ASSERT_TRUE(fmc_fvalid(fd));
  error = (fmc_error_t *)1;
  auto *yamal =
      ytp_yamal_new_3(fmc_fd_get(fp, &error), false, YTP_CLOSABLE, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);

  ASSERT_FALSE(ytp_yamal_closed(yamal, 0, &error));
  ASSERT_EQ(error, nullptr);

  ytp_yamal_close(yamal, 0, &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_TRUE(ytp_yamal_closed(yamal, 0, &error));
  ASSERT_EQ(error, nullptr);

  // try to close more than once
  ytp_yamal_close(yamal, 0, &error);
  ASSERT_EQ(error, nullptr);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);
  fclose(fp);
}

TEST(yamal, closable_write) {
  fmc_error_t *error;
  FILE *fp = tmpfile();
  auto fd = fmc_fd_get(fp, &error);
  ASSERT_TRUE(fmc_fvalid(fd));
  error = (fmc_error_t *)1;
  auto *yamal =
      ytp_yamal_new_3(fmc_fd_get(fp, &error), false, YTP_CLOSABLE, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(yamal, nullptr);

  ASSERT_FALSE(ytp_yamal_closed(yamal, 0, &error));
  ASSERT_EQ(error, nullptr);

  auto *msg = (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(msg, nullptr);
  msg->index = 1;
  error = (fmc_error_t *)1;
  ASSERT_NE(ytp_yamal_commit(yamal, msg, 0, &error), nullptr);
  ASSERT_EQ(error, nullptr);

  // Reserve before close

  auto *msg2 = (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(msg, nullptr);
  msg2->index = 2;

  ytp_yamal_close(yamal, 0, &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_TRUE(ytp_yamal_closed(yamal, 0, &error));
  ASSERT_EQ(error, nullptr);

  error = (fmc_error_t *)1;
  ASSERT_EQ(ytp_yamal_commit(yamal, msg2, 0, &error), nullptr);
  ASSERT_NE(error, nullptr);
  ASSERT_EQ(error->code, FMC_ERROR_CLOSED);

  // Reserve after close
  auto *msg3 = (test_msg *)ytp_yamal_reserve(yamal, sizeof(test_msg), &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(msg, nullptr);
  msg3->index = 3;

  error = (fmc_error_t *)1;
  ASSERT_EQ(ytp_yamal_commit(yamal, msg3, 0, &error), nullptr);
  ASSERT_NE(error, nullptr);
  ASSERT_EQ(error->code, FMC_ERROR_CLOSED);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);
  fclose(fp);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
