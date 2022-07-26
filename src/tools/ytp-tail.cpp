/******************************************************************************
    COPYRIGHT (c) 2020 by Featuremine Corporation.
    This software has been provided pursuant to a License Agreement
    containing restrictions on its use.  This software contains
    valuable trade secrets and proprietary information of
    FeatureMine Corporation and is protected by law.  It may not be
    copied or distributed in any form or medium, disclosed to third
    parties, reverse engineered or used in any manner not provided
    for in said License Agreement except with the prior written
    authorization Featuremine Corporation.
*****************************************************************************/

#include <tclap/CmdLine.h>

#include <fmc++/time.hpp>
#include <fmc/signals.h>
#include <ytp/control.h>
#include <ytp/sequence.h>
#include <ytp/version.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

static std::atomic<bool> run = true;
static void sig_handler(int s) { run = false; }

int main(int argc, char **argv) {
  fmc_set_signal_handler(sig_handler);

  TCLAP::CmdLine cmd("ytp tail tool", ' ', YTP_VERSION);

  TCLAP::UnlabeledValueArg<std::string> pathArg("ytp", "ytp path", true, "path",
                                                "string");
  cmd.add(pathArg);

  TCLAP::SwitchArg fArg("f", "follow",
                        "output appended data as the file grows;");

  cmd.add(fArg);

  cmd.parse(argc, argv);

  auto out_yamal_name = pathArg.getValue();

  fmc_error_t *error;
  fmc_fd out_yamal_fd =
      fmc_fopen(out_yamal_name.c_str(), fmc_fmode::READWRITE, &error);
  if (!fmc_fvalid(out_yamal_fd)) {
    std::cerr << "Unable to open file " << out_yamal_name << std::endl;
    return -1;
  }

  auto *seq = ytp_sequence_new(out_yamal_fd, &error);
  if (error) {
    std::cerr << fmc_error_msg(error) << std::endl;
    fmc_fclose(out_yamal_fd, &error);
    return -1;
  }

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t data_sz, const char *data_ptr) {
    auto seq = (ytp_sequence_t *)closure;
    fmc_error_t *error;

    size_t peer_name_sz;
    const char *peer_name_ptr;
    ytp_sequence_peer_name(seq, peer, &peer_name_sz,
                            &peer_name_ptr, &error);
    std::string_view peer_name(peer_name_ptr, peer_name_sz);

    size_t channel_name_sz;
    const char *channel_name_ptr;
    ytp_sequence_ch_name(seq, channel, &channel_name_sz,
                          &channel_name_ptr, &error);
    std::string_view channel_name(channel_name_ptr, channel_name_sz);

    fprintf(stdout, "%s %s %s %s\n",
            std::to_string(fmc::time(time)).c_str(),
            std::string(peer_name).c_str(),
            std::string(channel_name).c_str(),
            std::string_view(data_ptr, data_sz).data());
    fflush(stdout);

  };

  ytp_sequence_prfx_cb(seq, 1, "/", cb, seq, &error);
  if (error) {
    std::cerr << fmc_error_msg(error) << std::endl;
    ytp_sequence_del(seq, &error);
    fmc_fclose(out_yamal_fd, &error);
    return -1;
  }

  while (run) {
    if (!ytp_sequence_poll(seq, &error) && !fArg.getValue()) {
      run = false;
    }
    if (error) {
      std::cerr << fmc_error_msg(error) << std::endl;
      ytp_sequence_del(seq, &error);
      fmc_fclose(out_yamal_fd, &error);
      return -1;
    }
  }

  ytp_sequence_del(seq, &error);
  if (error) {
    std::cerr << fmc_error_msg(error) << std::endl;
    fmc_fclose(out_yamal_fd, &error);
    return -1;
  }

  fmc_fclose(out_yamal_fd, &error);
  if (error) {
    std::cerr << fmc_error_msg(error) << std::endl;
    return -1;
  }
  return 0;
}
