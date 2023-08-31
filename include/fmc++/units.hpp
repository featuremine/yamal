/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file units.hpp
 * @author Maxim Trokhimtchouk
 * @date 4 Oct 2017
 * @brief File contains C++ implementation of units
 *
 * This file describes generic interface units mechanism
 */

#pragma once

#include <fmc++/misc.hpp>
#include <fmc++/mpl.hpp>

#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace fmc {
/**
 * @brief units class
 */
class units {
private:
  struct a_unit {
    virtual ~a_unit() {}
  };
  template <class T, class... Args> struct the_unit : a_unit {
    using Tup = tuple<Args...>;
    the_unit(T *obj, Tup tup) : ptr_(obj), tup_(tup) {}
    static size_t hash(const Tup &tup) {
      auto tup_hash = std::hash<Tup>{}(tup);
      return std::hash<tuple<size_t, size_t>>()(
          make_tuple(typeid(the_unit<T, Args...>).hash_code(), tup_hash));
    }
    unique_ptr<T> ptr_;
    Tup tup_;
  };

public:
  units() = default;
  ~units() {
    for (auto &&[key, au] : map_) {
      delete au;
    }
  }

  template <class T, class Op, class... Args> T *get(Op &&op, Args... args);

private:
  using map_t = std::unordered_multimap<size_t, a_unit *>;
  map_t map_;
}; // units

/**
 * @brief get method
 */
template <class T, class Op, class... Args>
T *units::get(Op &&op, Args... args) {
  using F = the_unit<T, decay_t<Args>...>;
  auto tup = tuple<decay_t<Args>...>(forward<Args>(args)...);
  auto hash = F::hash(tup);
  auto range = map_.equal_range(hash);
  for (auto it = range.first; it != range.second; ++it) {
    auto *unit = dynamic_cast<F *>(it->second);
    if (unit && unit->tup_ == tup) {
      return unit->ptr_.get();
    }
  }
  auto *unit = new F(op(), tup);
  map_.emplace(hash, static_cast<a_unit *>(unit));
  return unit->ptr_.get();
}
} // namespace fmc
