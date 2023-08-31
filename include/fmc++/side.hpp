/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file side.hpp
 * @author Maxim Trokhimtchouk
 * @date 15 Dec 2017
 * @brief File contains C++ definition of a rational price
 */

#pragma once

#include "fmc++/convert.hpp"

#include <array>
#include <chrono>
#include <iostream>

namespace fmc {
using namespace std;

struct trade_side {
  enum SIDE { UNKNOWN = 0, BID = 1, ASK = 2 } value;
  trade_side() : value(UNKNOWN) {}
  trade_side(SIDE s) : value(s) {}
  bool operator==(const trade_side &a) const { return a.value == value; }
  bool operator!=(const trade_side &a) const { return a.value != value; }
  static array<trade_side, 2> all() { return {BID, ASK}; }
};
inline bool is_bid(trade_side side) { return side.value == trade_side::BID; }

inline bool is_ask(trade_side side) { return side.value == trade_side::ASK; }

inline bool is_unknown(trade_side side) {
  return side.value == trade_side::UNKNOWN;
}

inline trade_side other_side(trade_side side) {
  return side.value == trade_side::UNKNOWN
             ? trade_side::UNKNOWN
             : (side.value == trade_side::BID ? trade_side::ASK
                                              : trade_side::BID);
}

template <class T> struct sided_initializer {
  static constexpr bool is_specialized = false;
  static constexpr T min() noexcept { return T(); }
  static constexpr T max() noexcept { return T(); }
};

template <class T, template <class> class Initializer = sided_initializer>
struct sided : array<T, 2> {
  sided(initializer_list<T> list)
      : array<T, 2>(from_const_ptr(std::begin(list))) {}
  sided() : array<T, 2>({Initializer<T>::min(), Initializer<T>::max()}) {}
  auto &operator[](trade_side side) {
    return array<T, 2>::operator[](!is_bid(side));
  }
  auto &operator[](trade_side side) const {
    return array<T, 2>::operator[](!is_bid(side));
  }
  bool operator==(sided<T> &other) {
    return (*this)[trade_side::BID] == other[trade_side::BID] &&
           (*this)[trade_side::ASK] == other[trade_side::ASK];
  }
  bool operator!=(sided<T> &other) {
    return (*this)[trade_side::BID] != other[trade_side::BID] ||
           (*this)[trade_side::ASK] != other[trade_side::ASK];
  }

private:
  static array<T, 2> from_const_ptr(const T *it) {
    return array<T, 2>({*it, *(++it)});
  }
};

template <class T> struct better {
  better(trade_side side) : side_(side) {}
  bool operator()(const T &a, const T &b) const {
    return is_bid(side_) ? (a > b) : (a < b);
  }
  trade_side side_;
};
} // namespace fmc

namespace std {
inline ostream &operator<<(ostream &s, const fmc::trade_side x) {
  switch (x.value) {
  case fmc::trade_side::UNKNOWN:
    s << 'U';
    break;
  case fmc::trade_side::BID:
    s << 'B';
    break;
  case fmc::trade_side::ASK:
    s << 'A';
    break;
  }
  return s;
}

template <class T> inline ostream &operator<<(ostream &s, fmc::sided<T> &x) {
  s << "{" << x[fmc::trade_side::BID] << "," << x[fmc::trade_side::ASK] << "}";
  return s;
}

} // namespace std
