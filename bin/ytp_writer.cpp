#include <fmc/signals.h>
#include <ytp/control.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <random>
#include <thread>

static std::atomic<bool> run = true;
static void sig_handler(int s) { run = false; }

int main(int argc, char **argv) {
  fmc_set_signal_handler(sig_handler);

  if (argc != 2) {
    std::cerr << "Incorrect number of arguments" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "ytp_writer <yamal_file>" << std::endl;
    return -1;
  }

  const char *out_yamal_name = argv[1];

  fmc_error_t *error;
  fmc_fd out_yamal_fd = fmc_fopen(out_yamal_name, fmc_fmode::READWRITE, &error);
  if (!fmc_fvalid(out_yamal_fd)) {
    std::cerr << "Unable to open file " << out_yamal_name << std::endl;
    return -1;
  }

  auto *ctrl = ytp_control_new(out_yamal_fd, &error);
  if (error) {
    std::cerr << fmc_error_msg(error) << std::endl;
    return -1;
  }

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<size_t> message_number(
      0ull, std::numeric_limits<size_t>::max());
  std::uniform_int_distribution<size_t> idx(0ull, 10ull);

  while (run) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto peername = "peer" + std::to_string(idx(gen));
    auto peer =
        ytp_control_peer_decl(ctrl, peername.size(), peername.data(), &error);

    auto chname = "channel" + std::to_string(idx(gen));
    auto channel = ytp_control_ch_decl(ctrl, peer, 0, chname.size(),
                                       chname.data(), &error);

    switch (idx(gen)) {
    case 0: {
      auto numstr = std::to_string(message_number(gen));
      std::string_view dir = "dir1";
      ytp_control_dir(ctrl, peer, 1000, dir.size(), dir.data(), &error);
    } break;
    case 1: {
      auto numstr = std::to_string(message_number(gen));
      ytp_control_sub(ctrl, peer, 1000, chname.size(), chname.data(), &error);
    } break;
    default: {
      auto numstr = std::to_string(message_number(gen));
      char *dst = ytp_control_reserve(ctrl, numstr.size(), &error);
      strcpy(dst, numstr.c_str());
      ytp_control_commit(ctrl, peer, channel, 1000, dst, &error);
    } break;
    }
  }

  ytp_control_del(ctrl, &error);
  if (error) {
    std::cerr << fmc_error_msg(error) << std::endl;
    return -1;
  }

  fmc_fclose(out_yamal_fd, &error);
  if (error) {
    std::cerr << fmc_error_msg(error) << std::endl;
    return -1;
  }
  return 0;
}
