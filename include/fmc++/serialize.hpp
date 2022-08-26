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
