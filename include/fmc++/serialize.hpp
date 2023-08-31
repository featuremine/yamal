/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file serialize.hpp
 * @author Maxim Trokhimtchouk
 * @date 16 Oct 2017
 * @brief File contains C++ implementation of serialize
 *
 * This file describes generic interface subscription mechanism
 */

#pragma once

#include <iostream>
#include <tuple>
#include <utility>

/**
 * @brief output stream overload for pairs of objects
 */
namespace std {
template <class A, class B>
inline ostream &operator<<(ostream &s, const pair<A, B> &t) {
  s << '(' << t.first << ',' << t.second << ')';
  return s;
}

/**
 * @brief output stream overload for tuples of objects
 */
template <typename... Ts>
ostream &operator<<(ostream &os, tuple<Ts...> const &theTuple) {
  apply(
      [&os](Ts const &...tupleArgs) {
        os << '(';
        size_t n{0};
        ((os << tupleArgs << (++n != sizeof...(Ts) ? ", " : "")), ...);
        os << ')';
      },
      theTuple);
  return os;
}
} // namespace std
