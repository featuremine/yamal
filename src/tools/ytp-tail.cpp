#include <fmc/signals.h>
#include <ytp/control.h>
#include <ytp/sequence.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

static std::atomic<bool> run = true;
static void sig_handler(int s) { run = false; }

int main(int argc, char **argv) {
  fmc_set_signal_handler(sig_handler);

  if (argc != 2) {
    std::cerr << "Incorrect number of arguments" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "ytp-tail <yamal_file>" << std::endl;
    return -1;
  } else if (std::string("--help").compare(argv[1]) == 0) {
    std::cout << "Usage:" << std::endl;
    std::cout << "ytp-tail <yamal_file>" << std::endl;
    return 0;
  }

  const char *out_yamal_name = argv[1];

  fmc_error_t *error;
  fmc_fd out_yamal_fd = fmc_fopen(out_yamal_name, fmc_fmode::READWRITE, &error);
  if (!fmc_fvalid(out_yamal_fd)) {
    std::cerr << "Unable to open file " << out_yamal_name << std::endl;
    return -1;
  }

  auto *seq = ytp_sequence_new(out_yamal_fd, &error);
  if (error) {
    std::cerr << fmc_error_msg(error) << std::endl;
    return -1;
  }

  auto ch_cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                  uint64_t time, size_t sz, const char *name) {
    fmc_error_t *error;

    auto seq = (ytp_sequence_t *)closure;
    std::string_view channel_name(name, sz);

    std::cout << "Channel: " << channel_name << " (" << channel << "), " << time
              << std::endl;

    auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                 uint64_t time, size_t data_sz, const char *data_ptr) {
      auto seq = (ytp_sequence_t *)closure;
      fmc_error_t *error;

      size_t peer_name_sz;
      const char *peer_name_ptr;
      ytp_sequence_peer_name(seq, peer, &peer_name_sz, &peer_name_ptr, &error);
      std::string_view peer_name{peer_name_ptr, peer_name_sz};

      size_t channel_name_sz;
      const char *channel_name_ptr;
      ytp_sequence_ch_name(seq, channel, &channel_name_sz, &channel_name_ptr,
                           &error);
      std::string_view channel_name{channel_name_ptr, channel_name_sz};

      std::string_view data(data_ptr, data_sz);

      std::cout << "Data | p: " << peer << " (" << peer_name
                << ") | c: " << channel << " (" << channel_name
                << ") | data: " << data << std::endl;
    };

    ytp_sequence_indx_cb(seq, channel, cb, seq, &error);
  };

  ytp_sequence_ch_cb(seq, ch_cb, seq, &error);

  auto ctrl_cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                    uint64_t time, size_t data_sz, const char *data_ptr) {
    auto seq = (ytp_sequence_t *)closure;
    fmc_error_t *error;

    std::string_view data(data_ptr, data_sz);

    std::cout << "Ctrl | c: " << channel << " | t: " << time << " | d: " << data
              << std::endl;
  };

  ytp_sequence_indx_cb(seq, YTP_CHANNEL_DIR, ctrl_cb, seq, &error);
  ytp_sequence_indx_cb(seq, YTP_CHANNEL_SUB, ctrl_cb, seq, &error);

  auto peer_cb = [](void *closure, ytp_peer_t peer, size_t sz,
                    const char *name) {
    fmc_error_t *error;

    auto seq = (ytp_sequence_t *)closure;
    std::string_view peer_name(name, sz);

    std::cout << "Peer | i: " << peer << " | n: " << peer_name << std::endl;
  };

  ytp_sequence_peer_cb(seq, peer_cb, seq, &error);

  while (run) {
    ytp_sequence_poll(seq, &error);
  }

  ytp_sequence_del(seq, &error);
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
