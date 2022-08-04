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

#include <fmc++/time.hpp>
#include <fmc/signals.h>
#include <fmc/time.h>

#include <ytp/sequence.h>
#include <ytp/version.h>

#include <atomic>
#include <deque>
#include <string_view>
#include <unordered_set>

struct channel_stats_t {
  std::string_view name_;
  size_t count_ = 0;
  uint64_t last_ts = 0;
};

struct context_t {
  context_t(const char *filename, bool peer, bool channel, bool data)
      : channel_(channel), data_(data) {
    fd_ = fmc_fopen(filename, fmc_fmode::READ, &error_);
    if (error_) {
      std::cerr << "Unable to open file " << filename << ": "
                << fmc_error_msg(error_) << std::endl;
      return;
    }

    seq_ = ytp_sequence_new(fd_, &error_);
    if (error_) {
      std::cerr << "Unable to create the ytp sequence: "
                << fmc_error_msg(error_) << std::endl;
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
                  &stats, &self.error_);
            }

            if (self.channel_) {
              self.on_channel(peer, channel, msg_time,
                              std::string_view(name, sz));
            }
          },
          this, &error_);
    }

    if (peer) {
      ytp_sequence_peer_cb(
          seq_,
          [](void *closure, ytp_peer_t peer, size_t sz, const char *name) {
            context_t &self = *reinterpret_cast<context_t *>(closure);
            self.on_peer(peer, std::string_view(name, sz));
          },
          this, &error_);
    }
  }

  ~context_t() {
    if (seq_) {
      ytp_sequence_del(seq_, &error_);
      if (error_) {
        std::cerr << "Unable to delete the ytp sequence: "
                  << fmc_error_msg(error_) << std::endl;
        std::exit(-1);
      }
    }

    if (fd_ != -1) {
      fmc_fclose(fd_, &error_);
      if (error_) {
        std::cerr << "Unable to close the file descriptor: "
                  << fmc_error_msg(error_) << std::endl;
        std::exit(-1);
      }
    }
  }

  void poll() {
    while (ytp_sequence_poll(seq_, &error_)) {
      ++count_;
    }
  }

  std::ostream &log_ts() {
    return std::cout << std::chrono::system_clock::now().time_since_epoch();
  }

  std::ostream &log() { return std::cout; }

  void on_channel(ytp_peer_t peer, ytp_channel_t channel, uint64_t msg_time,
                  std::string_view name) {
    log_ts() << " CHNL " << std::to_string(fmc::time(msg_time)) << " " << name
             << std::endl;
  }

  void on_peer(ytp_peer_t peer, std::string_view name) {
    log_ts() << " PEER " << name << std::endl;
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
        log_ts() << " DATA " << std::to_string(fmc::time(stats->last_ts)) << " "
                 << stats->name_ << " " << stats->count_ << std::endl;
      }
    }
  }

  void reset() {
    for (auto *stats : current_data_) {
      stats->count_ = 0;
    }
    current_data_.clear();
  }

  fmc_error_t *error_;
  fmc_fd fd_ = -1;
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
  fmc_set_signal_handler(sig_handler);

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
  if (context.error_) {
    return -1;
  }

  if (followArg.getValue()) {
    int64_t next_timer = fmc_cur_time_ns();
    constexpr auto stats_interval =
        std::chrono::nanoseconds(std::chrono::seconds(1)).count();

    while (run) {
      context.poll();
      if (context.error_) {
        return -1;
      }

      auto now = fmc_cur_time_ns();
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
}
