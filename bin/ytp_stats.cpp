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

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_signal.h> // apr_signal()
#include <apr_file_io.h> // apr_file_t
#include <apr_pools.h> // apr_pool_t
#include <ytp/errno.h> // ytp_status_t ytp_strerror()
#include <ytp/sequence.h>
#include <ytp/version.h>

#include <atomic>
#include <deque>
#include <string_view>
#include <unordered_set>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip> // std::setfill
#include <time.h>

#include "utils.hpp"

std::string time_to_string(uint64_t nanosecs) {
    time_t secs = nanosecs/1000000000;
    char       buf[64];

    // Format time, "yyyy-mm-dd hh:mm:ss.nnnnnnnnn"
    struct tm ts = *localtime(&secs);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.", &ts);
    std::ostringstream os;
    os << buf << std::setfill('0') << std::setw(9) << nanosecs % 1000000000;
    return os.str();
}

struct channel_stats_t {
  std::string_view name_;
  size_t count_ = 0;
  uint64_t last_ts = 0;
};

struct context_t {
  context_t(const char *filename, bool peer, bool channel, bool data)
      : channel_(channel), data_(data) {
    rv_ = ytp_initialize(); // also calls apr_initialize(), apr_pool_initialize(), apr_signal_init()
    if(rv_) {
      char error_str[128];
      std::cerr << ytp_strerror(rv_, error_str, sizeof(error_str)) << std::endl;
      return;
    }
    rv_ = apr_pool_create(&pool_, NULL);
    if(rv_) {
      char error_str[128];
      std::cerr << ytp_strerror(rv_, error_str, sizeof(error_str)) << std::endl;
      return;
    }
    rv_ = apr_file_open(&f_, filename,
                      APR_FOPEN_CREATE | APR_FOPEN_READ,
                      APR_FPROT_OS_DEFAULT, pool_);
    if(rv_) {
      std::cerr << "Unable to open file: " << filename << std::endl;
      char error_str[128];
      std::cerr << ytp_strerror(rv_, error_str, sizeof(error_str)) << std::endl;
      return;
    }
    rv_ = ytp_sequence_new(&seq_, f_);
    if(rv_) {
      std::cerr << "Unable to create the ytp sequence: " << std::endl;
      char error_str[128];
      std::cerr << ytp_strerror(rv_, error_str, sizeof(error_str)) << std::endl;
      return;
    }

    if (!data_ && !channel_ && !peer) {
      data_ = true;
      channel_ = true;
      peer = true;
    }

    if (data_ || channel_) {
      ytp_sequence_ch_cb(
          seq_,
          [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
             uint64_t msg_time, size_t sz, const char *name) {
            context_t &self = *reinterpret_cast<context_t *>(closure);
            auto &stats =
                self.channels_.emplace_back(channel_stats_t{}, closure);
            stats.first.name_ = std::string_view(name, sz);

            if (self.data_) {
              ytp_sequence_indx_cb(
                  self.seq_, channel,
                  [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                     uint64_t msg_time, size_t sz, const char *data) {
                    auto &closure_pair =
                        *reinterpret_cast<std::pair<channel_stats_t, void *> *>(
                            closure);
                    context_t &self =
                        *reinterpret_cast<context_t *>(closure_pair.second);
                    self.on_data(closure_pair.first, peer, channel, msg_time,
                                 std::string_view(data, sz));
                  },
                  &stats);
            }

            if (self.channel_) {
              self.on_channel(peer, channel, msg_time,
                              std::string_view(name, sz));
            }
          },
          this);
    }

    if (peer) {
      ytp_sequence_peer_cb(
          seq_,
          [](void *closure, ytp_peer_t peer, size_t sz, const char *name) {
            context_t &self = *reinterpret_cast<context_t *>(closure);
            self.on_peer(peer, std::string_view(name, sz));
          },
          this);
    }
  }

  ~context_t() {
    if(seq_) {
      rv_ = ytp_sequence_del(seq_);
      if(rv_) {
        char error_str[128];
        std::cerr << ytp_strerror(rv_, error_str, sizeof(error_str)) << std::endl;
        return;
      }
    }
    if(f_) {
      rv_ = apr_file_close(f_);
      if(rv_) {
        char error_str[128];
        std::cerr << ytp_strerror(rv_, error_str, sizeof(error_str)) << std::endl;
        return;
      }
    }
    if(pool_) {
      apr_pool_destroy(pool_);
    }
    ytp_terminate();
  }

  void poll() {
    bool new_data;
    while ( !(rv_ = ytp_sequence_poll(seq_, &new_data)) && new_data) {
      ++count_;
    }
  }

  std::ostream &log() { return std::cout; }

  void on_channel(ytp_peer_t peer, ytp_channel_t channel, uint64_t msg_time,
                  std::string_view name) {
    std::cout << " CHNL " << time_to_string(msg_time) << " " << name << std::endl;
  }

  void on_peer(ytp_peer_t peer, std::string_view name) {
    std::cout << " PEER " << name << std::endl;
  }

  void on_data(channel_stats_t &stats, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t msg_time, std::string_view data) {
    current_data_.emplace(&stats);
    ++stats.count_;
    stats.last_ts = msg_time;
  }

  void print_stats() {
    if (!current_data_.empty()) {
      for (auto *stats : current_data_) {
        std::cout << " DATA " << time_to_string(stats->last_ts) << " " << stats->name_ << " " << stats->count_ << std::endl;
      }
    }
  }

  void reset() {
    for (auto *stats : current_data_) {
      stats->count_ = 0;
    }
    current_data_.clear();
  }

  ytp_status_t rv_ = YTP_STATUS_OK;
  apr_pool_t *pool_ = nullptr;
  apr_file_t *f_ = nullptr;
  ytp_sequence_t *seq_ = nullptr;
  size_t count_ = 0;
  std::deque<std::pair<channel_stats_t, void *>> channels_;
  std::unordered_set<channel_stats_t *> current_data_;
  bool channel_;
  bool data_;
};

static std::atomic<bool> run = true;
static void sig_handler(int s) { run = false; }

int main(int argc, char **argv) {
  ytp_status_t rv = ytp_initialize(); // also calls apr_initialize(), apr_pool_initialize(), apr_signal_init()
  if(rv) {
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }
  apr_signal(SIGQUIT, sig_handler);
  apr_signal(SIGABRT, sig_handler);
  apr_signal(SIGTERM, sig_handler);
  apr_signal(SIGINT, sig_handler);

  TCLAP::CmdLine cmd("ytp statistics tool", ' ', YTP_VERSION);

  TCLAP::UnlabeledValueArg<std::string> ytpArg("ytp", "YTP path", true, "/path",
                                               "ytp_path");

  TCLAP::SwitchArg followArg("f", "follow", "output appended data every second",
                             false);

  TCLAP::SwitchArg peerArg("p", "peer", "peer announcements", false);

  TCLAP::SwitchArg channelArg("c", "channel", "channel announcements", false);

  TCLAP::SwitchArg dataArg("d", "data", "data statistics", false);

  cmd.add(ytpArg);
  cmd.add(followArg);
  cmd.add(peerArg);
  cmd.add(channelArg);
  cmd.add(dataArg);

  cmd.parse(argc, argv);

  context_t context(ytpArg.getValue().c_str(), peerArg.getValue(),
                    channelArg.getValue(), dataArg.getValue());
  if (context.rv_) {
    ytp_terminate();
    return -1;
  }

  if (followArg.getValue()) {
    int64_t next_timer = cur_time_ns();
    constexpr auto stats_interval =
        std::chrono::nanoseconds(std::chrono::seconds(1)).count();

    while (run) {
      context.poll();
      if (context.rv_) {
        ytp_terminate();
        return -1;
      }

      auto now = cur_time_ns();
      if (next_timer <= now) {
        next_timer = now + stats_interval;
        context.print_stats();
        context.reset();
      }
    }
  } else {
    context.poll();
    context.print_stats();
  }
  ytp_terminate();
  return 0;
}
