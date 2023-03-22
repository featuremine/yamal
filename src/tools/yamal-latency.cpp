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

#include <tclap/CmdLine.h>
#include <fmc++/counters.hpp>
#include <fmc++/time.hpp>
#include <fmc/process.h>
#include <fmc/signals.h>
#include <ytp/control.h>
#include <ytp/sequence.h>
#include <ytp/version.h>

#include <cstring>
#include <iostream>

#define CHECK(E) if (E) {goto error;}

int main(int argc, char **argv) {
  TCLAP::CmdLine cmd("yamal copy tool", ' ', YTP_VERSION);

  TCLAP::UnlabeledValueArg<std::string> srcArg("src", "source yamal path", true, "src", "string");
  cmd.add(srcArg);

  TCLAP::ValueArg<size_t> periodArg(
      "p", "period", "period of diagram publishing", false, 1, "seconds");
  cmd.add(periodArg);

  TCLAP::ValueArg<int> affinityArg("a", "affinity", "set the CPU affinity of the main process",
                                   false, 0, "cpuid");
  cmd.add(affinityArg);

  cmd.parse(argc, argv);

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
  auto period = 1000000000ULL * periodArg.getValue();

  fmc_error_t *error;
  fmc_fd src_fd = -1;
  ytp_yamal_t *src_yml = NULL;
  ytp_iterator_t it = NULL;
  int64_t last = fmc_cur_time_ns();

  fmc::counter::precision_sampler buckets_;
  std::vector<double> percentiles{25.0, 50.0, 75.0, 90.0, 95.0, 99.0, 100.0};

  src_fd = fmc_fopen(src_name.c_str(), fmc_fmode::READ, &error);
  if (!fmc_fvalid(src_fd)) {
    std::cerr << "Unable to open file " << src_name << std::endl;
    goto error;
  }

  src_yml = ytp_yamal_new(src_fd, &error);
  CHECK(error);

  it = ytp_yamal_end(src_yml, &error);
  CHECK(error);
  while (true) {
    if (ytp_yamal_term(it)) continue;
    CHECK(error);
    size_t sz;
    char *src;
    ytp_peer_t peer;
    ytp_channel_t ch;
    uint64_t time;
    ytp_time_read(src_yml, it, &peer, &ch, &time, &sz, (const char **)&src, &error);
    CHECK(error);
    auto now = fmc_cur_time_ns();
    buckets_.sample(now - time);
    it = ytp_yamal_next(src_yml, it, &error);

    if (last + period < now) {
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
  if (src_yml) ytp_yamal_del(src_yml, &error);
  if (src_fd != -1) fmc_fclose(src_fd, &error);

  return -1;
}
