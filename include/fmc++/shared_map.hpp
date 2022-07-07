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

/**
 * @file shared_map.hpp
 * @author Federico Ravchina
 * @date 22 Jul 2021
 * @brief File contains C++ implementation of shared_map
 *
 * This file contains C++ implementation of shared_map.
 */

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace fmc {

template <typename K, typename V, typename H = std::hash<K>,
          typename P = std::equal_to<K>>
class shared_map {
private:
  class _shared {
  private:
    friend shared_map;

    V *find(const K &key) const noexcept {
      std::scoped_lock lock(m);
      auto it = table.find(key);
      if (it != table.end()) {
        return it->second.get();
      }
      return nullptr;
    }

    V *emplace(const K &key, std::unique_ptr<V> value) noexcept {
      std::scoped_lock lock(m);
      auto it = table.emplace(key, std::move(value)).first;
      return it->second.get();
    }

    mutable std::mutex m;
    std::unordered_map<K, std::unique_ptr<V>, H, P> table;
  };

public:
  using base = std::unordered_map<K, V *, H, P>;
  using base_iterator = typename base::iterator;
  using base_const_iterator = typename base::const_iterator;
  // static_assert(std::is_same<base_iterator, base_const_iterator>::value);

  class iterator {
  private:
    using first_type = typename base_iterator::value_type::first_type;
    using second_type = typename std::remove_pointer<
        typename base_iterator::value_type::second_type>::type;

  public:
    using difference_type = typename base_iterator::difference_type;
    using value_type = std::pair<const first_type &, second_type &>;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = typename base_iterator::iterator_category;

    iterator(base_iterator iter) : iter_(std::move(iter)) {}
    bool operator==(const iterator &a) const { return iter_ == a.iter_; }
    bool operator!=(const iterator &a) const { return iter_ != a.iter_; }
    value_type operator*() {
      return std::pair<const first_type &, second_type &>(iter_->first,
                                                          *iter_->second);
    }

  private:
    base_iterator iter_;
  };

  class const_iterator {
  private:
    using first_type = typename base_iterator::value_type::first_type;
    using second_type = typename std::remove_pointer<
        typename base_iterator::value_type::second_type>::type;

  public:
    using difference_type = typename base_iterator::difference_type;
    using value_type = std::pair<const first_type &, const second_type &>;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = typename base_iterator::iterator_category;

    const_iterator(base_const_iterator iter) : iter_(std::move(iter)) {}
    bool operator==(const const_iterator &a) const { return iter_ == a.iter_; }
    bool operator!=(const const_iterator &a) const { return iter_ != a.iter_; }
    value_type operator*() {
      return std::pair<const first_type &, const second_type &>(iter_->first,
                                                                *iter_->second);
    }

  private:
    base_const_iterator iter_;
  };

public:
  shared_map() noexcept : shared_(std::make_shared<_shared>()) {}

  shared_map(const shared_map &rhs) noexcept : shared_(rhs.shared_) {}
  shared_map(shared_map &rhs) noexcept
      : shared_map(static_cast<const shared_map &>(rhs)) {}

  shared_map &operator=(const shared_map &rhs) noexcept {
    shared_ = rhs.shared_;
    local_ = nullptr;
  }

  shared_map(shared_map &&rhs) noexcept
      : shared_(std::move(rhs.shared_)), local_(std::move(rhs.local_)) {}

  shared_map &operator=(shared_map &&rhs) noexcept {
    shared_ = std::move(rhs.shared_);
    local_ = std::move(rhs.local_);
  }

  iterator find(const K &key) noexcept {
    auto it = local_.find(key);
    if (it != local_.end()) {
      return it;
    }

    if (auto *n = shared_->find(key); n) {
      return local_.emplace(key, n).first;
    }

    return end();
  }
  const_iterator find(const K &key) const noexcept {
    auto it = local_.find(key);
    if (it != local_.end()) {
      return it;
    }

    return end();
  }

  iterator end() noexcept { return local_.end(); }
  const_iterator end() const noexcept { return local_.end(); }

  bool contains(const K &key) const noexcept {
    return local_.find(key) != local_.end();
  }

  std::pair<iterator, bool> insert(const K &key,
                                   std::unique_ptr<V> value) noexcept {
    auto it = local_.find(key);
    if (it != local_.end()) {
      return {it, false};
    }
    return local_.emplace(key, shared_->emplace(key, std::move(value)));
  }

private:
  std::shared_ptr<_shared> shared_;
  std::unordered_map<K, V *, H, P> local_;
};

} // namespace fmc
