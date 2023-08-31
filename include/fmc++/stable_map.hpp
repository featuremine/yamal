/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once

#include <fmc++/as_ref.hpp>
#include <fmc++/mpl.hpp>

#include <memory>
#include <unordered_map>

namespace fmc {
template <typename K, typename T> struct stable_map;
}

namespace std {
template <typename K, typename V, typename F>
void erase_if(fmc::stable_map<K, V> &m, const F &f);
}

namespace fmc {

template <typename K, typename T>
struct stable_map : private std::unordered_map<K, std::unique_ptr<T>> {
  using std::unordered_map<K, std::unique_ptr<T>>::unordered_map;
  using M = std::unordered_map<K, std::unique_ptr<T>>;

  using base_iterator = typename M::iterator;
  using value_type = T;

  struct iterator : base_iterator {
    iterator(base_iterator &&base) : base_iterator(std::move(base)) {}
    iterator(const base_iterator &base) : base_iterator(base) {}

    std::pair<const K &, T &> operator*() noexcept {
      base_iterator &base = *this;
      return {base->first, *base->second};
    }
    as_ref<std::pair<const K &, T &>> operator->() noexcept {
      base_iterator &base = *this;
      return {base->first, *base->second};
    }
  };

  stable_map(stable_map &&rhs) noexcept = default;
  stable_map &operator=(stable_map &&rhs) noexcept = default;

  iterator find(const K &key) { return M::find(key); }

  template <typename ArgK, typename ArgV>
  std::pair<iterator, bool> emplace(ArgK &&k, ArgV &&v) {
    return M::emplace(std::forward<ArgK>(k),
                      std::make_unique<ArgV>(std::forward<ArgV>(v)));
  }

  T &operator[](const K &key) {
    auto where = M::emplace(key, std::unique_ptr<T>{});
    if (where.second) {
      where.first->second = std::make_unique<T>();
    }
    return *where.first->second;
  }

  iterator begin() noexcept { return M::begin(); }

  iterator end() noexcept { return M::end(); }

  void clear() noexcept { M::clear(); }

  std::size_t size() const noexcept { return M::size(); }

  template <typename K_, typename T_, typename F>
  friend void ::std::erase_if(fmc::stable_map<K_, T_> &m, const F &f);
};

} // namespace fmc

namespace std {
template <typename K, typename V, typename F>
void erase_if(fmc::stable_map<K, V> &m, const F &f) {
  using M = typename fmc::stable_map<K, V>::M;
  using iterator = typename fmc::stable_map<K, V>::iterator;
  M &base = m;
  K to_remove[base.size()];
  auto *p = &to_remove[0];
  for (auto it = base.begin(); it != base.end(); ++it) {
    auto item = *iterator(it);
    if (f(item)) {
      *(p++) = it->first;
    }
  }
  while (p-- > &to_remove[0]) {
    base.erase(*p);
  }
}
} // namespace std
