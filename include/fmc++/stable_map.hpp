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

#pragma once

#include <fmc++/as_ref.hpp>
#include <fmc++/mpl.hpp>

#include <memory>
#include <unordered_map>

namespace fmc {

template <typename K, typename T>
struct stable_map : private std::unordered_map<K, std::unique_ptr<T>> {
  using std::unordered_map<K, std::unique_ptr<T>>::unordered_map;
  using M = std::unordered_map<K, std::unique_ptr<T>>;

  using base_iterator = typename M::iterator;
  using value_type = T;

  struct iterator : base_iterator {
    iterator(base_iterator &&base) : base_iterator(std::move(base)) {}

    std::pair<const K &, T &> operator*() noexcept {
      base_iterator &base = *this;
      return {base->first, *base->second};
    }
    as_ref<std::pair<const K &, T &>> operator->() noexcept {
      base_iterator &base = *this;
      return {base->first, *base->second};
    }
  };

  iterator find(const K &key) { return M::find(key); }

  template<typename ArgK, typename ArgV>
  std::pair<iterator, bool> emplace(ArgK &&k, ArgV &&v) {
    return M::emplace(std::forward<ArgK>(k), std::make_unique<ArgV>(std::forward<ArgV>(v)));
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
};

}
