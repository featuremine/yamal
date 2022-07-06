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

/**
 * @file utils.hpp
 * @author Featuremine Corporation
 * @date 16 May 2022
 * @brief Yamal test utils
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_UTILS_HPP__
#define __FM_YTP_UTILS_HPP__

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_mmap.h> // apr_mmap_t
#include <ytp/yamal.h>
#include <algorithm> // std::min()
#include <array>

#if defined(WIN32)
#define FLOORLOG2(X)                                                       \
  ((unsigned)(8 * sizeof(unsigned long long) - __lzcnt64((X)) - 1))
#else
#define FLOORLOG2(X)                                                       \
  ((unsigned)(8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))
#endif

struct sample {
  sample() = default;
  sample(const sample &) = delete;
  sample &operator=(const sample &) = delete;
  virtual ~sample() {}
  virtual double value() = 0;
};

struct log_bucket : sample {
  static constexpr uint64_t N = 64;
  log_bucket() { buckets_.fill(0ull); }
  void clear() { buckets_.fill(0ull); }
  void sample(uint64_t val) {
    uint64_t idx = (val != 0) * FLOORLOG2(val);
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

int64_t cur_time_ns(void);

void print_percentile(std::ostream &os, log_bucket &buckets, double percentile);

ytp_status_t mmap_sync(apr_mmap_t *map, apr_size_t size);

#endif // __FM_YTP_UTILS_HPP__