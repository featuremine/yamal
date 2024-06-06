/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file daemon.cpp
 * @date 10 Jan 2022
 * @brief File contains tests for YTP daemon tool
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/yamal.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>
#include <fmc/process.h>

#include <signal.h>
#include <unistd.h>

using namespace std;

struct state {
  const char *name;
  const char *link;
  bool empty;
} states[] = {
    /* 0 */ {NULL, NULL, false},
    /* 1 */ {"daemon.test.ytp", NULL, false},
    /* 2 */ {"daemon.test.ytp", "daemon.link.ytp", true},
    /* 3 */ {"daemon.test.ytp", "daemon.link.ytp", false},
    /* 4 */ {"daemon.test.ytp", "daemon.link2.ytp", false},
};

unsigned tran_mtrx[4][5] = {
    {0, 1, 8, 10, 0},
    {7, 0, 2, 12, 0},
    {9, 6, 0, 3, 0},
    {13, 11, 5, 0, 4},
};

bool streq(const char *a, const char *b) {
  return a == b || (a && b && strcmp(a, b) == 0);
}

struct state *transitions(size_t *size) {
  unsigned sz = 0;
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 5; ++c) {
      sz = max(tran_mtrx[r][c], sz);
    }
  }
  struct state *trans = (struct state *)calloc(sz, sizeof(struct state));
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 5; ++c) {
      if (!tran_mtrx[r][c])
        continue;
      trans[tran_mtrx[r][c] - 1] = states[c];
    }
  }
  *size = sz;
  return trans;
}

void create_yamal_file(const char *name) {
  fmc_error_t *error = nullptr;
  int fd = fmc_fopen(name, fmc_fmode::READWRITE, &error);
  ASSERT_EQ(error, nullptr);
  ytp_yamal_t *ytp = ytp_yamal_new(fd, &error);
  ASSERT_EQ(error, nullptr);
  ytp_yamal_del(ytp, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

static pid_t pid = 0;

static void sig_handler(int signo) {
  if (pid != 0)
    killpg(pid, signo);
  exit(0);
}

TEST(daemon, state_transition) {
  size_t sz = 0;
  struct state *cur = &states[0];
  struct state *trans = transitions(&sz);

  fmc_error_t *error = nullptr;

  unlink("daemon-state-transition.test.log");
  unlink("daemon.test.ytp");
  unlink("daemon.link.ytp");
  unlink("daemon.link2.ytp");
  unlink("state_transition.cfg");

  fmc_fd cfgfd =
      fmc_fopen("state_transition.cfg", fmc_fmode::READWRITE, &error);
  ASSERT_EQ(error, nullptr);
  const char cfgstr[] = "[main]\n"
                        "ytps=test_ytp,\n"
                        "[test_ytp]\n"
                        "path=\"daemon.test.ytp\"\n"
                        "rate=0\n"
                        "initial_size=\"32\"";
  ASSERT_EQ(fmc_write(cfgfd, cfgstr, sizeof(cfgstr)), sizeof(cfgstr));
  fmc_fclose(cfgfd, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fd basefd =
      fmc_fopen("daemon-base.log", fmc_fmode::READWRITE, &error);
  ASSERT_EQ(error, nullptr);
  const char basestr[] = "opened file at daemon.test.ytp\n"
                         "closed yamal file daemon.test.ytp";
  for (auto i = 0; i < 7; ++i) {
    if (i != 0)
      ASSERT_EQ(fmc_write(basefd, "\n", 1), 1);
    ASSERT_EQ(fmc_write(basefd, basestr, sizeof(basestr)), sizeof(basestr));
  }
  fmc_fclose(basefd, &error);
  ASSERT_EQ(error, nullptr);

  signal(SIGTERM, sig_handler);
  signal(SIGINT, sig_handler);
  signal(SIGABRT, sig_handler);

  pid = fmc_exec("../../package/bin/yamal-daemon -c "
                 "../../../tests/tools/state_transition.cfg -s main > "
                 "daemon-state-transition.test.log",
                 &error);
  ASSERT_NE(pid, -1);
  ASSERT_EQ(error, nullptr);

  usleep(500000);

  for (size_t i = 0; i < sz; ++i) {
    struct state *next = &trans[i];
    printf("==============================================\n");
    printf("cur name %s link %s %s\n", cur->name ? cur->name : "NULL",
           cur->link ? cur->link : "NULL", cur->empty ? "empty" : "");
    printf("next name %s link %s %s\n", next->name ? next->name : "NULL",
           next->link ? next->link : "NULL", next->empty ? "empty" : "");

    if (!streq(cur->link, next->link) || (!cur->empty ^ !next->empty)) {
      if (cur->link && !cur->empty) {
        printf("unlink %s %d\n", cur->link, __LINE__);
        ASSERT_EQ(unlink(cur->link), 0);
      }
      if (next->link && !next->empty) {
        printf("create yamal file %s\n", next->link);
        create_yamal_file(next->link);
      }
    }
    if (!streq(cur->name, next->name) || (!cur->link ^ !next->link)) {
      if (cur->name) {
        printf("unlink %s %d\n", cur->name, __LINE__);
        ASSERT_EQ(unlink(cur->name), 0);
      }
      if (next->name) {
        if (next->link) {
          printf("create link %s -> %s\n", next->name, next->link);
          ASSERT_EQ(symlink(next->link, next->name), 0);
        } else {
          printf("create yamal file %s\n", next->name);
          create_yamal_file(next->name);
        }
      }
    } else {
      if (!streq(cur->link, next->link) && cur->name) {
        printf("unlink %s %d\n", cur->name, __LINE__);
        ASSERT_EQ(unlink(cur->name), 0);
        printf("create link %s -> %s\n", next->name, next->link);
        ASSERT_EQ(symlink(next->link, next->name), 0);
      }
    }
    cur = next;
    sleep(4);
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    ASSERT_EQ(result, 0);
  }
  free(trans);

  killpg(pid, SIGTERM);

  int status = fmc_waitpid(pid, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(status, -1);

  ASSERT_TRUE(fmc_run_base_vs_test_diff("daemon-base.log",
                                        "daemon-state-transition.test.log"));
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
