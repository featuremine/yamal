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

#define FM_COUNTER_ENABLE

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstring>
#include <tclap/CmdLine.h>

#include <fmc++/counters.hpp>
#include <fmc++/ordered_map.hpp>

#include <fmc/endianness.h>
#include <fmc/process.h>
#include <fmc/signals.h>
#include <fmc/time.h>

#include <ytp/sequence.h>
#include <ytp/version.h>

using namespace std::chrono_literals;

static std::atomic<bool> run = true;
static void sig_handler(int s) { run = false; }

class precision_sampler {
public:
  using buckets_t = fmc::ordered_multimap<uint64_t, uint64_t>;
  void sample(uint64_t x) {
    using namespace std;
    auto s = double(x);
    auto c = pow(10.0, max(floor(log10(s)) - 1, 0.0));
    auto b = uint64_t(round(s / c) * c);
    if (auto where = buckets_.find(b); where == buckets_.end()) {
      where = buckets_.emplace(b, 1);
    } else {
      where->second++;
    }
  }
  buckets_t::const_iterator sample_index(uint64_t count) const {
    uint64_t current = 0;
    for (auto it = buckets_.cbegin(); it != buckets_.end(); ++it) {
      current += it->second;
      if (current >= count) {
        return it;
      }
    }
    return buckets_.end();
  }
  double percentile(double p) const {
    uint64_t total = 0;
    for (auto &&[b, c] : buckets_) {
      total += c;
    }

    uint64_t sample_count = (int64_t(total * p) + 99ull) / 100ull;

    auto it = sample_index(sample_count);
    return it == buckets_.end() ? NAN : double(it->first);
  }
  void clear() { buckets_.clear(); }

private:
  buckets_t buckets_;
};

int main(int argc, char **argv) {
  fmc_set_signal_handler(sig_handler);

  bool is_source = argc > 1 && std::string_view(argv[1]) == "source";
  bool is_sink = argc > 1 && std::string_view(argv[1]) == "sink";

  if (!is_source && !is_sink) {
    std::cout << "Use:" << std::endl;
    std::cout << " " << argv[0] << " source --help" << std::endl;
    std::cout << " " << argv[0] << " sink --help" << std::endl;
    return -1;
  }

  TCLAP::CmdLine cmd("ytp performance tester", ' ', YTP_VERSION);

  TCLAP::ValueArg<std::string> fileArg("f", "file",
                                       is_sink
                                           ? "ytp file to read the messages"
                                           : "ytp file to publish the messages",
                                       true, "", "path");

  TCLAP::ValueArg<std::string> channelName(
      "c", "channel", "name of the channel to use", false,
      "performance_test_channel", "string");

  TCLAP::ValueArg<size_t> messagesArg("m", "messages",
                                      "number of messages to publish", false,
                                      1000000, "integer");

  TCLAP::ValueArg<size_t> sizeArg(
      "s", "size", "size of the messages to publish", false, 256, "bytes");

  TCLAP::ValueArg<size_t> rateArg("r", "rate",
                                  "number of messages to publish in one second",
                                  false, 1000000, "messages");

  TCLAP::ValueArg<int> affinityArg("a", "affinity",
                                   "set the CPU affinity of the main process",
                                   false, 0, "cpuid");

  TCLAP::ValueArg<int> priorityArg(
      "x", "priority", "set the priority of the main process (0-100)", false, 0,
      "priority");

  cmd.add(fileArg);
  cmd.add(channelName);
  if (is_source) {
    cmd.add(messagesArg);
    cmd.add(sizeArg);
    cmd.add(rateArg);
  }
  cmd.add(affinityArg);
  cmd.add(priorityArg);

  std::vector<const char *> argv_ex(argc - 1);
  for (int i = 2; i < argc; ++i) {
    argv_ex[i - 1] = argv[i];
  }
  std::string new_command = std::string(argv[0]) + " " + std::string(argv[1]);
  argv_ex[0] = new_command.c_str();

  cmd.parse(argv_ex.size(), argv_ex.data());

  const std::string &filename = fileArg.getValue();

  fmc_error_t *error;
  auto fd = fmc_fopen(filename.c_str(), fmc_fmode::READWRITE, &error);
  if (error) {
    std::cerr << "Unable to open file " << filename << ": "
              << fmc_error_msg(error) << std::endl;
    return -1;
  }

  auto *seq = ytp_sequence_new(fd, &error);
  if (error) {
    std::cerr << "Unable to create the ytp sequence: " << fmc_error_msg(error)
              << std::endl;
    return -1;
  }

  std::string_view peername = "performance_tester";
  auto peer =
      ytp_sequence_peer_decl(seq, peername.size(), peername.data(), &error);
  if (error) {
    std::cerr << "Unable to declare the ytp peer: " << fmc_error_msg(error)
              << std::endl;
    return -1;
  }

  std::string_view channelname = channelName.getValue();
  auto channel =
      ytp_sequence_ch_decl(seq, peer, fmc_cur_time_ns(), channelname.size(),
                           channelname.data(), &error);
  if (error) {
    std::cerr << "Unable to declare the ytp peer: " << fmc_error_msg(error)
              << std::endl;
    return -1;
  }

  if (affinityArg.isSet()) {
    fmc_error_t *error;
    auto cur_thread = fmc_tid_cur(&error);
    if (error) {
      std::string message;
      message += "Error getting current thread: ";
      message += fmc_error_msg(error);
      std::cerr << message << std::endl;
    } else {
      auto cpuid = affinityArg.getValue();
      fmc_set_affinity(cur_thread, cpuid, &error);
      if (error) {
        std::string message;
        message += "Error set affinity: ";
        message += fmc_error_msg(error);
        std::cerr << message << std::endl;
      } else {
        if (priorityArg.isSet()) {
          auto priority = priorityArg.getValue();
          fmc_set_sched_fifo(cur_thread, priority, &error);
          if (error) {
            std::string message;
            message += "Error setting fifo scheduler: ";
            message += fmc_error_msg(error);
            std::cerr << message << std::endl;
          }
        }
      }
    }
  }

  if (is_source) {
    auto sleep_ms = std::chrono::microseconds(0);
    if (rateArg.isSet()) {
      auto rate = rateArg.getValue();
      if (rate <= 0) {
        std::cerr << "Rate must be non-negative" << std::endl;
        return -1;
      }
      sleep_ms = std::chrono::microseconds(
          std::chrono::microseconds(std::chrono::seconds(1)).count() / rate);
    }

    size_t msg_count = 0;
    auto msg_size = std::max(sizeArg.getValue(), sizeof(size_t));
    std::string payload(msg_size - sizeof(size_t), '.');

    size_t prev_messages_count = 0;
    int64_t next_timer = fmc_cur_time_ns();

    size_t curr_seqnum = 0;
    for (ssize_t i = messagesArg.getValue();
         (!messagesArg.isSet() || --i >= 0) && run;) {
      std::this_thread::sleep_for(sleep_ms);

      auto *ptr = ytp_sequence_reserve(seq, msg_size, &error);
      if (error) {
        std::cerr << "Unable to reserve: " << fmc_error_msg(error) << std::endl;
        return -1;
      }

      auto &msg_seqnum = *reinterpret_cast<size_t *>(ptr);
      msg_seqnum = fmc_htobe64(curr_seqnum++);
      memcpy(ptr + sizeof(size_t), payload.data(), payload.size());

      auto now = fmc_cur_time_ns();
      ytp_sequence_commit(seq, peer, channel, now, ptr, &error);
      ++msg_count;
      if (error) {
        std::cerr << "Unable to commit: " << fmc_error_msg(error) << std::endl;
        return -1;
      }

      if (next_timer <= now) {
        constexpr auto one_sec =
            std::chrono::nanoseconds(std::chrono::seconds(1)).count();
        auto timer_delayed = next_timer - now;
        next_timer = now + one_sec;
        std::cout << msg_count - prev_messages_count
                  << " messages sent in the last "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::nanoseconds(timer_delayed + one_sec))
                         .count()
                  << "ms" << std::endl;
        prev_messages_count = msg_count;
      }
    }

    std::cout << msg_count << " messages with a size of " << sizeArg.getValue()
              << " bytes were published to " << filename << std::endl;
  } else if (is_sink) {

    struct stats_t {
      std::string_view peer_name;
      ytp_sequence_t *seq;
      size_t messages_count_;
      int64_t first_message_;
      int64_t last_message_;
      size_t total_size_;
      int64_t total_time_;
      size_t seqnum_;
      int64_t out_of_sequence_count_ = 0;
      precision_sampler buckets_;

      stats_t() { reset(); }

      void set_peer_name(std::string_view peer_name) {
        this->peer_name = peer_name;
      }

      void reset() {
        messages_count_ = 0;
        first_message_ = 0;
        last_message_ = 0;
        total_size_ = 0;
        total_time_ = 0;
        buckets_.clear();
      }

      void sample(const char *data_ptr, size_t data_sz, uint64_t msg_time) {
        size_t msg_seqnum =
            fmc_be64toh(*reinterpret_cast<const size_t *>(data_ptr));
        auto now = fmc_cur_time_ns();
        // now - time = how long it took from writing to receiving the message
        uint64_t time_write_read = now - msg_time;

        auto msg_sz = *reinterpret_cast<const size_t *>(data_ptr);
        if (first_message_ == 0) {
          first_message_ = now;
        }
        messages_count_ += 1;
        last_message_ = now;
        total_size_ += data_sz;
        total_time_ += time_write_read;

        if (msg_seqnum == 0) {
          seqnum_ = 0;
        }
        if (msg_seqnum != seqnum_++) {
          ++out_of_sequence_count_;
        }

        buckets_.sample(time_write_read);
      }

      void print() {
        auto experiment_time =
            std::chrono::nanoseconds(last_message_ - first_message_);
        auto experiment_time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(
                experiment_time);

        // Sum of all messages times (from write to read)
        auto total_time_ns = std::chrono::nanoseconds(total_time_);
        auto total_time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(
                total_time_ns);

        if (out_of_sequence_count_ > 0) {
          std::cout << out_of_sequence_count_
                    << " messages were out of sequence\n";
        }

        if (messages_count_ == 0) {
          std::cout << "No messages\n";
        } else if (total_time_us.count() < 10) {
          std::cout << "Time difference between the first message and the last "
                       "one was too small\n";
        } else {
          std::cout << "Total: " << messages_count_ << " messages, "
                    << ((double)total_size_) / 1024.0 / 1024.0
                    << " MB\n"

                       "Experiment time: "
                    << experiment_time_us.count()
                    << " microseconds\n"

                       "Total message processing time: "
                    << total_time_us.count()
                    << " microseconds\n"

                       "Data rate: "
                    << ((double)total_size_) * 1000000.0 /
                           ((double)experiment_time_us.count()) / 1024.0 /
                           1024.0
                    << " MB/sec, "
                    << (messages_count_ * 1000000ll) /
                           experiment_time_us.count()
                    << " msgs/sec\n"

                       "Average time for each message: "
                    << total_time_ns.count() / messages_count_
                    << " nanoseconds\n";

          std::vector<double> percentiles{25.0, 50.0, 75.0, 90.0,
                                          95.0, 99.0, 100.0};
          for (double &percentile : percentiles) {
            std::cout << percentile
                      << "% percentile: " << buckets_.percentile(percentile)
                      << " nanoseconds" << std::endl;
          }
        }
      }
    } closure;

    ytp_sequence_data_cb_t on_message =
        [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
           uint64_t msg_time, size_t data_sz, const char *data_ptr) {
          auto &stats = *reinterpret_cast<stats_t *>(closure);
          stats.sample(data_ptr, data_sz, msg_time);
        };

    ytp_sequence_indx_cb(seq, channel, on_message, &closure, &error);
    if (error) {
      std::cerr << "Unable to register callback: " << fmc_error_msg(error)
                << std::endl;
      return -1;
    }

    int64_t next_timer = fmc_cur_time_ns();

    while (run) {
      while (ytp_sequence_poll(seq, &error))
        ;
      auto now = fmc_cur_time_ns();
      if (next_timer <= now) {
        constexpr auto one_sec =
            std::chrono::nanoseconds(std::chrono::seconds(1)).count();
        auto timer_delayed = next_timer - now;
        next_timer = now + one_sec;
        closure.print();
        closure.reset();
      }
    }
  }

  ytp_sequence_del(seq, &error);
  if (error) {
    std::cerr << "Unable to delete the ytp sequence: " << fmc_error_msg(error)
              << std::endl;
    return -1;
  }

  fmc_fclose(fd, &error);
  if (error) {
    std::cerr << "Unable to close the file descriptor: " << fmc_error_msg(error)
              << std::endl;
    return -1;
  }
}
