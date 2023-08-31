/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <fmc++/counters.hpp>
#include <fmc++/time.hpp>
#include <fmc/process.h>
#include <tclap/CmdLine.h>
#include <ytp/control.h>
#include <ytp/data.h>
#include <ytp/sequence.h>
#include <ytp/version.h>

#include <iostream>

#define CHECK(E)                                                               \
  if (E) {                                                                     \
    goto error;                                                                \
  }

int main(int argc, char **argv) {
  TCLAP::CmdLine cmd("yamal copy tool", ' ', YTP_VERSION);

  TCLAP::UnlabeledValueArg<std::string> srcArg("src", "source yamal path", true,
                                               "src", "string");
  cmd.add(srcArg);

  TCLAP::ValueArg<size_t> periodArg(
      "p", "period", "period of diagram publishing", false, 1, "seconds");
  cmd.add(periodArg);

  TCLAP::ValueArg<int> affinityArg("a", "affinity",
                                   "set the CPU affinity of the main process",
                                   false, 0, "cpuid");
  cmd.add(affinityArg);

  TCLAP::ValueArg<int> auxArg("x", "auxiliary",
                              "set the CPU affinity of the auxiliary process",
                              false, 0, "cpuid");
  cmd.add(auxArg);

  cmd.parse(argc, argv);

  if (auxArg.isSet()) {
    ytp_yamal_set_aux_thread_affinity(auxArg.getValue());
  }

  if (affinityArg.isSet()) {
    fmc_error_t *error;
    auto cur_thread = fmc_tid_cur(&error);
    if (error) {
      std::string message;
      message += "Error getting current thread: ";
      message += fmc_error_msg(error);
      std::cerr << message << std::endl;
      return -1;
    } else {
      auto cpuid = affinityArg.getValue();
      fmc_set_affinity(cur_thread, cpuid, &error);
      if (error) {
        std::string message;
        message += "Error set affinity: ";
        message += fmc_error_msg(error);
        std::cerr << message << std::endl;
        return -1;
      }
    }
  }

  auto src_name = srcArg.getValue();
  auto period = 1000000000LL * periodArg.getValue();

  fmc_error_t *error;
  fmc_fd src_fd = -1;
  ytp_yamal_t *src_yml = nullptr;
  ytp_iterator_t it = nullptr;
  int64_t last = fmc_cur_time_ns();

  fmc::counter::log_bucket buckets_;
  uint64_t count = 0ULL;
  std::vector<double> percentiles{25.0, 50.0, 75.0, 90.0, 95.0, 99.0, 100.0};

  src_fd = fmc_fopen(src_name.c_str(), fmc_fmode::READ, &error);
  if (!fmc_fvalid(src_fd)) {
    std::cerr << "Unable to open file " << src_name << std::endl;
    goto error;
  }

  src_yml = ytp_yamal_new(src_fd, &error);
  CHECK(error);

  it = ytp_data_end(src_yml, &error);
  CHECK(error);
  while (true) {
    if (ytp_yamal_term(it))
      continue;
    CHECK(error);
    size_t sz;
    char *src;
    uint64_t seqno;
    int64_t ts;
    ytp_time_read(src_yml, it, &seqno, &ts, &sz, (const char **)&src, &error);
    CHECK(error);
    auto now = fmc_cur_time_ns();
    buckets_.sample(now - ts);
    ++count;
    it = ytp_yamal_next(src_yml, it, &error);

    if (last + period < now) {
      std::cout << "count: " << count << std::endl;
      for (double &percentile : percentiles) {
        std::cout << percentile
                  << "% percentile: " << buckets_.percentile(percentile)
                  << " nanoseconds" << std::endl;
      }
      last = now;
    }
  }

  return 0;

error:
  std::cerr << fmc_error_msg(error) << std::endl;
  if (src_yml)
    ytp_yamal_del(src_yml, &error);
  if (src_fd != -1)
    fmc_fclose(src_fd, &error);

  return -1;
}
