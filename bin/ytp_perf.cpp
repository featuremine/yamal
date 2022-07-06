/******************************************************************************
    COPYRIGHT (c) 2022 by Featuremine Corporation.
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
#include <string>
#include <string_view>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstring>
#include <tclap/CmdLine.h>

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_signal.h> // apr_signal()
#include <apr_file_io.h> // apr_file_t
#include <apr_pools.h> // apr_pool_t
#include <ytp/errno.h> // ytp_status_t ytp_strerror()
#include <ytp/sequence.h>
#include <ytp/version.h>

#include "utils.hpp"
#include "endianness.h"

using namespace std::chrono_literals;

static std::atomic<bool> run = true;
static void sig_handler(int s) { run = false; }

class precision_sampler {
public:
  using buckets_t = ordered_multimap<uint64_t, uint64_t>;
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

  TCLAP::ValueArg<apr_size_t> messagesArg("m", "messages",
                                      "number of messages to publish", false,
                                      1000000, "integer");

  TCLAP::ValueArg<apr_size_t> sizeArg(
      "s", "size", "size of the messages to publish", false, 256, "bytes");

  TCLAP::ValueArg<apr_size_t> rateArg("r", "rate",
                                  "number of messages to publish in one second",
                                  false, 1000000, "messages");

  TCLAP::ValueArg<int64_t> timeArg("i", "interval", "statistics interval in ms",
                                   false, 1000, "time_ms");

  TCLAP::ValueArg<int> affinityArg("a", "affinity",
                                   "set the CPU affinity of the main process",
                                   false, 0, "cpuid");

  TCLAP::ValueArg<int> priorityArg(
      "x", "priority", "set the priority of the main process (1-99)", false, 1,
      "priority");

  cmd.add(fileArg);
  cmd.add(channelName);
  if (is_source) {
    cmd.add(messagesArg);
    cmd.add(sizeArg);
    cmd.add(rateArg);
  }
  cmd.add(timeArg);
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

  apr_pool_t *pool;
  rv = apr_pool_create(&pool, NULL);
  if(rv) {
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }
  apr_file_t *f;
  rv = apr_file_open(&f, filename.c_str(),
                     APR_FOPEN_CREATE | APR_FOPEN_READ | APR_FOPEN_WRITE,
                     APR_FPROT_OS_DEFAULT, pool);
  if(rv) {
    std::cerr << "Unable to open file: " << filename << std::endl;
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }

  ytp_sequence_t *seq;
  rv = ytp_sequence_new(&seq, f);
  if(rv) {
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }

  std::string_view peername = "performance_tester";
  ytp_peer_t peer;
  rv = ytp_sequence_peer_decl(seq, &peer, peername.size(), peername.data());
  if(rv) {
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }

  std::string_view channelname = channelName.getValue();
  ytp_channel_t channel;
  rv = ytp_sequence_ch_decl(seq, &channel, peer, cur_time_ns(), channelname.size(), channelname.data());
  if(rv) {
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }

  if (affinityArg.isSet()) {
    auto cur_thread = _tid_cur();
    auto cpuid = affinityArg.getValue();
    rv = _set_affinity(cur_thread, cpuid);
    if (rv) {
      std::string message;
      message += "Error set affinity: ";
      char error_str[128];
      message += ytp_strerror(rv, error_str, sizeof(error_str));
      std::cerr << message << std::endl;
    } else {
      if (priorityArg.isSet()) {
        auto priority = priorityArg.getValue();
        rv = _set_sched_fifo(cur_thread, priority);
        if (rv) {
          std::string message;
          message += "Error setting fifo scheduler: ";
          char error_str[128];
          message += ytp_strerror(rv, error_str, sizeof(error_str));
          std::cerr << message << std::endl;
        }
      }
    }
  }

  auto interval =
      std::chrono::nanoseconds(std::chrono::milliseconds(timeArg.getValue()))
          .count();

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

    apr_size_t msg_count = 0;
    auto msg_size = std::max(sizeArg.getValue(), sizeof(apr_size_t));
    std::string payload(msg_size - sizeof(apr_size_t), '.');

    apr_size_t prev_messages_count = 0;
    int64_t next_timer = cur_time_ns();

    apr_size_t curr_seqnum = 0;
    for (apr_ssize_t i = messagesArg.getValue();
         (!messagesArg.isSet() || --i >= 0) && run;) {
      std::this_thread::sleep_for(sleep_ms);
      char *ptr;
      rv = ytp_sequence_reserve(seq, &ptr, msg_size);
      if(rv) {
        char error_str[128];
        std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
        return rv;
      }

      auto &msg_seqnum = *reinterpret_cast<apr_size_t *>(ptr);
      msg_seqnum = _htobe64(curr_seqnum++);
      memcpy(ptr + sizeof(apr_size_t), payload.data(), payload.size());

      auto now = cur_time_ns();
      ytp_iterator_t it;
      rv = ytp_sequence_commit(seq, &it, peer, channel, now, ptr);
      if(rv) {
        char error_str[128];
        std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
        return rv;
      }
      ++msg_count;

      if (next_timer <= now) {
        auto timer_delayed = now - next_timer;
        next_timer = now + interval;
        std::cout << msg_count - prev_messages_count
                  << " messages sent in the last "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::nanoseconds(timer_delayed + interval))
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

      void sample(const char *data_ptr, apr_size_t data_sz, uint64_t msg_time) {
        apr_size_t msg_seqnum =
            _be64toh(*reinterpret_cast<const apr_size_t *>(data_ptr));
        auto now = cur_time_ns();
        uint64_t time_write_read = now - msg_time;

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
           uint64_t msg_time, apr_size_t data_sz, const char *data_ptr) {
          auto &stats = *reinterpret_cast<stats_t *>(closure);
          stats.sample(data_ptr, data_sz, msg_time);
        };

    ytp_sequence_indx_cb(seq, channel, on_message, &closure);

    int64_t next_timer = cur_time_ns();

    while (run) {
      bool new_data;
      while ( !(rv = ytp_sequence_poll(seq, &new_data)) && new_data)
        ;
      auto now = cur_time_ns();
      if (next_timer <= now) {
        next_timer = now + interval;
        closure.print();
        closure.reset();
      }
    }
  }

  rv = ytp_sequence_del(seq);
  if(rv) {
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }

  rv = apr_file_close(f);
  if(rv) {
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }
  apr_pool_destroy(pool);
  ytp_terminate();
  return 0;
}
