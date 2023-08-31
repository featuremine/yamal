/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file ordered_map.hpp
 * @author Maxim Trokhimtchouk
 * @date 18 Apr 2018
 * @brief Definition of the ordered map
 */

#pragma once

#include <algorithm>
#include <functional>
#include <vector>

namespace fmc {

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
  template <class A, class... Args> iterator emplace(A &&a, Args &&...args) {
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

} // namespace fmc
