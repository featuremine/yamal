/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <fmc++/counters.hpp>
#include <fmc/cmdline.h>
#include <fmc/files.h>
#include <ytp/data.h>
#include <ytp/yamal.h>

static int interrupted;

static void sigint_handler(int sig) { interrupted = 1; }

int main(int argc, const char **argv) {
  using namespace std;

  fmc_fd fd;
  fmc_error_t *error = nullptr;

  signal(SIGINT, sigint_handler);

  const char *ytpfile = nullptr;
  const char *help = nullptr;
  fmc_cmdline_opt_t options[] = {
      {"--ytp-file", true, &ytpfile}, {"--help", false, &help}, {NULL}};

  fmc_cmdline_opt_proc(argc, argv, options, &error);
  if (error && !help) {
    fprintf(stderr, "could not process args: %s\n", fmc_error_msg(error));
    return 1;
  }

  if (help) {
    printf("yamal-local-perf --ytp-file FILE\n\n"
           "This tools reads new messages and periodically writes out a "
           "histogram of\n"
           "difference between the time on the message and the time it was "
           "received.\n");
    return 0;
  }

  fd = fmc_fopen(ytpfile, fmc_fmode::READWRITE, &error);
  if (error) {
    fprintf(stderr, "could not open file %s with error %s\n", ytpfile,
            fmc_error_msg(error));
    return 1;
  }
  auto *yamal = ytp_yamal_new(fd, &error);
  if (error) {
    fprintf(stderr, "could not create yamal with error %s\n",
            fmc_error_msg(error));
    return 1;
  }

  fmc::counter::precision_sampler sampler;
  int64_t last = 0;
  int64_t period = 10LL * 1000000000LL; // 10s
  auto it = ytp_data_end(yamal, &error);
  while (!interrupted) {
    if (ytp_yamal_term(it)) {
      int64_t now = fmc_cur_time_ns();
      if (last + period <= now) {
        printf("processed %ld samples\n", sampler.total());
        for (int p = 10; p <= 100; p += 10) {
          printf("percentile %d: %.0f\n", p, sampler.percentile(p));
        }
        last = now;
      }
      continue;
    }
    uint64_t seqno;
    int64_t ts;
    ytp_mmnode_offs stream;
    size_t sz;
    const char *data;
    ytp_data_read(yamal, it, &seqno, &ts, &stream, &sz, &data, &error);
    if (error) {
      fprintf(stderr, "could not read yamal: %s\n", fmc_error_msg(error));
      return 1;
    }
    int64_t now = fmc_cur_time_ns();
    sampler.sample(now - ts);
    it = ytp_yamal_next(yamal, it, &error);
  }

  ytp_yamal_del(yamal, &error);

  return 0;
}
