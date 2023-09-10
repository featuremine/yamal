/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file counters.hpp
 * @author Maxim Trokhimtchouk
 * @date 19 Mar 2018
 * @brief File contains various performance counters used throughout the code
 * @see http://www.featuremine.com
 */

#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iterator>
#include <memory>

#include <fmc/files.h>
#include <fmc/platform.h>
#include <fmc/time.h>
#include <thread>
#include <unordered_map>
#if defined(FMC_SYS_WIN)
#include <cstdint>
#include <intrin.h>
#include <limits>
#define __builtin_expect(X, Y) X
#elif defined(FMC_AMD64)
#include <cxxabi.h>
#include <pthread.h>
#include <x86intrin.h>
#endif

#if defined(FMC_SYS_LINUX)
#include <sys/stat.h>
#endif // DEBUG
#ifdef __GNUC__
#include <cxxabi.h>
#elif _MSC_VER
#define __builtin_expect(X, Y) X
#endif

#include <fmc++/mpl.hpp>
#include <fmc++/ordered_map.hpp>
#include <fmc/alignment.h>
#include <fmc/platform.h>

#include <array>
#include <cmath>

#if defined(FMC_SYS_MACH)
#include <mach/mach_time.h>
#endif

namespace fmc {
namespace counter {
using namespace std;
using namespace chrono;

#define FMC_COUNT_NANO_AVG_START(counter_name)                                 \
  static auto counter_name =                                                   \
      counter::counter_record<counter::nano_record<counter::avg>>(             \
          #counter_name);                                                      \
  counter_name.start()

#define FMC_COUNT_NANO_AVG_STOP(counter_name) counter_name.stop()

struct sample {
  sample() = default;
  sample(const sample &) = delete;
  sample &operator=(const sample &) = delete;
  virtual ~sample() {}
  virtual double value() = 0;
};

template <uint8_t L2W> struct ewma : sample {
  void sample(uint64_t val) { avg_shl_ = (avg_shl_ - (avg_shl_ >> L2W)) + val; }
  double value() override { return double(avg_shl_) / double(1ULL << L2W); }
  uint64_t avg_shl_ = 0;
};

struct avg : sample {
  void sample(uint64_t val) {
    ++count_;
    value_ += (double(val) - value_) / double(count_);
  }
  double value() override { return value_; }
  double value_ = 0;
  uint64_t count_ = 0;
};

struct min : sample {
  void sample(uint64_t val) { min_ = std::min(val, min_); }
  double value() override { return min_; }
  uint64_t min_ = std::numeric_limits<uint64_t>::max();
};

struct max : sample {
  void sample(uint64_t val) { max_ = std::max(val, max_); }
  double value() override { return max_; }
  uint64_t max_ = std::numeric_limits<uint64_t>::min();
};

struct last : sample {
  void sample(double val) { val_ = val; }
  double value() override { return val_; }
  double val_ = 0;
};

inline pair<uint64_t, unsigned> floor_log10(uint64_t val) {
  unsigned idx = (val != 0) * (1 + FMC_FLOORLOG2(val) * 3 / 10);
  uint64_t pow10;
  switch (idx) {
  case 0:
    pow10 = 1ULL;
    break;
  case 1:
    pow10 = 1ULL;
    break;
  case 2:
    pow10 = 10ULL;
    break;
  case 3:
    pow10 = 100ULL;
    break;
  case 4:
    pow10 = 1000ULL;
    break;
  case 5:
    pow10 = 10000ULL;
    break;
  case 6:
    pow10 = 100000ULL;
    break;
  case 7:
    pow10 = 1000000ULL;
    break;
  case 8:
    pow10 = 10000000ULL;
    break;
  case 9:
    pow10 = 100000000ULL;
    break;
  case 10:
    pow10 = 1000000000ULL;
    break;
  case 11:
    pow10 = 10000000000ULL;
    break;
  case 12:
    pow10 = 100000000000ULL;
    break;
  case 13:
    pow10 = 1000000000000ULL;
    break;
  case 14:
    pow10 = 10000000000000ULL;
    break;
  case 15:
    pow10 = 100000000000000ULL;
    break;
  case 16:
    pow10 = 1000000000000000ULL;
    break;
  case 17:
    pow10 = 10000000000000000ULL;
    break;
  case 18:
    pow10 = 100000000000000000ULL;
    break;
  case 19:
    pow10 = 1000000000000000000ULL;
    break;
  case 20:
    pow10 = 10000000000000000000ULL;
    break;
  default:
    pow10 = 10000000000000000000ULL;
  }
  unsigned bump = val / 10 >= pow10;
  idx += bump;
  pow10 *= bump * 10ULL + !bump;
  return {pow10, idx};
}

struct log_bucket : sample {
  static constexpr uint64_t N = 64;
  log_bucket() { buckets_.fill(0ull); }
  void clear() { buckets_.fill(0ull); }
  void sample(uint64_t val) {
    uint64_t idx = (val != 0) * FMC_FLOORLOG2(val);
    ++buckets_[std::min(N, idx)];
  }
  double bucket_upper_bound(uint64_t i) { return (1ull << (i + 1)) - 1; }
  uint64_t sample_index(uint64_t count) {
    uint64_t current = 0;
    for (uint64_t i = 0ull; i <= N; ++i) {
      current += buckets_[i];
      if (current >= count) {
        return i;
      }
    }
    return N;
  }
  double percentile(double percentile) {
    uint64_t total = 0;
    for (auto occurrences : buckets_) {
      total += occurrences;
    }

    uint64_t sample_count = (int64_t(total * percentile) + 99ull) / 100ull;

    return bucket_upper_bound(sample_index(sample_count));
  }
  double value() override { return percentile(50); }
  std::array<uint64_t, N + 1> buckets_;
};

class precision_sampler : sample {
public:
  using buckets_t = fmc::ordered_multimap<uint64_t, uint64_t>;
  void sample(uint64_t x) {
    auto c = floor_log10(x).first;
    auto b = uint64_t((x / c) * c);
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
  uint64_t total() const {
    uint64_t total = 0ULL;
    for (auto &&[b, c] : buckets_) {
      total += c;
    }
    return total;
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
  double value() override { return percentile(50); }
  void clear() { buckets_.clear(); }

private:
  buckets_t buckets_;
};

class samples {
private:
  samples(const samples &) = delete;
  using map_t = unordered_map<std::string, sample *>;
  using iterator =
      iterator_mapper<typename map_t::iterator, dereference_second>;
  using const_iterator =
      iterator_mapper<typename map_t::const_iterator, dereference_second>;

public:
  samples() = default;
  ~samples() {
    for (auto &&item : map_) {
      delete item.second;
    }
  }
  template <class T> T &get(string_view raw_key) {
    std::string key(raw_key);
    auto where = map_.find(key);
    if (where != map_.end()) {
      auto *sampl = dynamic_cast<T *>(where->second);
      fmc_runtime_error_unless(sampl)
          << "registering sample with key " << key << " of type "
          << type_name<sample>() << " already registered with different type  ";
      return *sampl;
    }
    auto *container = new T();
    map_.emplace(key, container);
    return *container;
  }
  iterator begin() { return map_.begin(); };
  iterator end() { return map_.end(); };
  iterator find(string_view raw_key) {
    std::string key(raw_key);
    return map_.find(key);
  };
  const_iterator begin() const { return map_.begin(); };
  const_iterator end() const { return map_.end(); };
  const_iterator find(string_view raw_key) const {
    std::string key(raw_key);
    return map_.find(key);
  }

private:
  map_t map_;
};

struct rdtsc {
  int64_t operator()() {
#if defined(FMC_SYS_MACH)
    return mach_absolute_time();
#elif defined(FMC_AMD64)
    return __rdtsc();
#elif defined(FMC_SYS_ARM)
    // source:
    // https://github.com/google/benchmark/blob/v1.1.0/src/cycleclock.h#L116
    int64_t virtual_timer_value;
    asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
    return virtual_timer_value;
#else
#error "rdtsc is not defined";
#endif
  }
};

#ifdef FMC_SYS_MACH
struct nanoseconds {
  nanoseconds() { mach_timebase_info(&clock_timebase_); }
  int64_t operator()() {
    return __int128(mach_absolute_time()) * __int128(clock_timebase_.numer) /
           clock_timebase_.denom;
  }
  mach_timebase_info_data_t clock_timebase_;
};
#else
struct nanoseconds {
  int64_t operator()() { return fmc_cur_time_ns(); }
};
#endif

template <class Counter, class Sample> struct record : Sample {
  using type = invoke_result_t<Counter>;
  template <typename... Args>
  record(Args &&...args) : Sample(std::forward<Args>(args)...) {}
  void start() { val_ = counter_(); }
  void stop() { Sample::sample(counter_() - val_); }
  type val_;
  Counter counter_;
};

template <class Record> struct scoped_sampler {
  scoped_sampler(Record &inst) : inst_(inst) { inst_.start(); }
  ~scoped_sampler() { inst_.stop(); }
  Record &inst_;
};

template <template <class> class Record> struct single_scope_sampler {
  template <class... Args>
  single_scope_sampler(Args &&...args) : inst_(args...) {
    inst_.start();
  }
  ~single_scope_sampler() { inst_.stop(); }
  Record<last> inst_;
};

inline uint64_t ticks_per_millisec() {
  auto start = rdtsc()();
  this_thread::sleep_for(milliseconds(1));
  auto end = rdtsc()();
  return end - start;
}

inline uint64_t ticks_per_millisec_once() {
  static uint64_t var = ticks_per_millisec();
  return var;
}

inline uint64_t ticks_per_sec() {
  auto start = rdtsc()();
  this_thread::sleep_for(seconds(1));
  auto end = rdtsc()();
  return end - start;
}

inline uint64_t ticks_per_sec_once() {
  static uint64_t var = ticks_per_sec();
  return var;
}

template <class Sample> struct tick_to_nano : Sample {
  double value() override {
    return Sample::value() * double(1000000) /
           double(ticks_per_millisec_once());
  }
};

template <class Sample> using nano_record = record<nanoseconds, Sample>;

template <class Sample>
using rdtsc_record = record<rdtsc, tick_to_nano<Sample>>;
template <uint8_t L2W> using rdtsc_ewma = rdtsc_record<ewma<L2W>>;

using rdtsc_avg = rdtsc_record<avg>;

template <class Record, class Args>
Record &counter_init(Args args, string_view name) {
  if constexpr (tuple_has<Args, counter::samples &>) {
    return get<samples &>(args).template get<Record>(name);
  } else {
    static Record record;
    return record;
  }
}

struct counter_outfile_handler {
  counter_outfile_handler() {
    char *file_path = std::getenv("FMC_COUNTER_PATH");
    if (file_path != NULL) {
      outfile_.open(file_path, std::ofstream::out);
      fmc_runtime_error_unless(outfile_.is_open());
    }
  }

  std::ofstream outfile_;
};

static counter_outfile_handler counter_outfile_handler_;

template <class Record> struct counter_record {
  counter_record(const char *name) { name_ = name; }

  void start() { nr_.start(); }

  void stop() { nr_.stop(); }

  ~counter_record() {
    counter_outfile_handler_.outfile_ << name_ << "=" << nr_.value()
                                      << std::endl;
  }
  Record nr_;
  const char *name_;
};

template <class Record>
using rdtsc_counter_record = counter_record<rdtsc_record<Record>>;

using rdtsc_elapsed_counter = single_scope_sampler<rdtsc_counter_record>;
} // namespace counter
} // namespace fmc

#define FMC_COUNTER_INIT(ARGS, PERF, NAME)                                     \
  PERF(::fmc::counter::counter_init<std::remove_reference_t<decltype(PERF)>>(  \
      ARGS, NAME))

#define FMC_SCOPED_SAMPLE(X)                                                   \
  fmc::counter::scoped_sampler X##_scoped_sampler_##__LINE__(X);

#if defined(FMC_COUNTER_ENABLE)

#define FMC_NANO_EWMA(X)                                                       \
  static ::fmc::counter::counter_record<                                       \
      ::fmc::counter::rdtsc_record<::fmc::counter::ewma<10>>>                  \
      counter_##X##__LINE__("FMC_NANO_EWMA(" #X ")");                          \
  ::fmc::counter::scoped_sampler<                                              \
      ::fmc::counter::rdtsc_record<::fmc::counter::ewma<10>>>                  \
      scoped_sampler_##X##__LINE__(counter_##X##__LINE__.nr_)

#define FMC_NANO_AVG(X)                                                        \
  static ::fmc::counter::counter_record<                                       \
      ::fmc::counter::rdtsc_record<::fmc::counter::avg>>                       \
      counter_##X##__LINE__("FMC_NANO_AVG(" #X ")");                           \
  ::fmc::counter::scoped_sampler<                                              \
      ::fmc::counter::rdtsc_record<::fmc::counter::avg>>                       \
      scoped_sampler_##X##__LINE__(counter_##X##__LINE__.nr_)

#else

#define FMC_NANO_EWMA(X)

#define FMC_NANO_AVG(X)

#endif
