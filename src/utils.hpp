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
#include <algorithm> // std::min() std::equal_range
#include <array>
#include <functional> // std::less()
#include <utility> // std::pair std::move std::forward
#include <vector>

#if defined(__linux__) || defined(DARWIN)
#include <pthread.h>
typedef pthread_t _tid;
#elif defined(WIN32)
#include <windows.h>
typedef HANDLE _tid;
#endif

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

template <class Key, class T, class Compare = std::less<Key>>
class ordered_multimap {
public:
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<Key, T>;
  using size_type = std::size_t;
  using key_compare = Compare;
  using reference = value_type &;
  using const_reference = const value_type &;
  using container = std::vector<value_type>;
  using iterator = typename container::iterator;
  using const_iterator = typename container::const_iterator;
  ordered_multimap() = default;
  explicit ordered_multimap(const key_compare &comp) : comp_({comp}) {}
  ordered_multimap(ordered_multimap &&rhs) noexcept
      : comp_(std::move(rhs.comp_)), items_(std::move(rhs.items_)) {}
  ordered_multimap &operator=(ordered_multimap &&rhs) noexcept {
    comp_ = std::move(rhs.comp_);
    items_ = std::move(rhs.items_);
  }
  ordered_multimap(const ordered_multimap &rhs) = default;
  ordered_multimap &operator=(const ordered_multimap &rhs) = default;

  iterator begin() { return items_.begin(); }
  iterator end() { return items_.end(); }
  const_iterator begin() const { return items_.begin(); }
  const_iterator end() const { return items_.end(); }
  const_iterator cbegin() const { return items_.cbegin(); }
  const_iterator cend() const { return items_.cend(); }
  bool empty() const noexcept { return items_.empty(); }
  void clear() noexcept { items_.clear(); }
  template <class K> iterator lower_bound(const K &x) {
    return std::lower_bound(begin(), end(), x, comp_);
  }
  template <class K> const_iterator lower_bound(const K &x) const {
    return std::lower_bound(begin(), end(), x, comp_);
  }
  template <class K> iterator upper_bound(const K &x) {
    return std::upper_bound(begin(), end(), x, comp_);
  }
  template <class K> const_iterator upper_bound(const K &x) const {
    return std::upper_bound(begin(), end(), x, comp_);
  }
  template <class K> std::pair<iterator, iterator> equal_range(const K &x) {
    return std::equal_range(begin(), end(), x, comp_);
  }
  template <class K>
  std::pair<const_iterator, const_iterator> equal_range(const K &x) const {
    return std::equal_range(begin(), end(), x, comp_);
  }
  iterator insert(value_type &&value) {
    auto where = upper_bound(value.first);
    return items_.insert(where, value);
  }
  template <class K> iterator find(const K &x) {
    auto where = lower_bound(x);
    if (where == end())
      return end();
    if (where->first == x)
      return where;
    return end();
  }
  template <class K> const_iterator find(const K &x) const {
    auto where = lower_bound(x);
    if (where == end())
      return end();
    if (where->first == x)
      return where;
    return end();
  }
  template <class A, class... Args> iterator emplace(A &&a, Args &&... args) {
    auto where = lower_bound(a);
    return items_.emplace(where, std::forward<A>(a),
                          std::forward<Args>(args)...);
  }
  iterator erase(iterator pos) { return items_.erase(pos); }

private:
  struct {
    template <class K> bool operator()(const value_type &v, const K &x) {
      return cmp_(v.first, x);
    }
    key_compare cmp_;
  } comp_;
  container items_;
};

/**
 * @brief Returns the current thread id
 * @return _tid
 */
_tid _tid_cur();

/**
 * @brief Sets a thread's CPU affinity
 * @param tid thread id
 * @param cpuid CPU id
 */
ytp_status_t _set_affinity(_tid tid, int cpuid);

/**
 * @brief Sets current thread's CPU affinity
 * @param cpuid CPU id
 */
ytp_status_t _set_cur_affinity(int cpuid);

/**
 * @brief Sets the priority of a thread using normal scheduling
 *
 * @param tid thread id
 */
ytp_status_t _set_sched_normal(_tid tid);

/**
 * @brief Sets the priority of a thread using FIFO scheduling
 *
 * @param tid thread id
 * @param priority priority being 1 the lowest and 99 the maximum
 */
ytp_status_t _set_sched_fifo(_tid tid, int priority);

#endif // __FM_YTP_UTILS_HPP__