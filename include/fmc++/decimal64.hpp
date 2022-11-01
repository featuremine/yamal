/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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
 * @file decimal64.hpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions for the decimal64 object
 *
 * This file contains C++ declarations of the decimal64 object operations
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "fmc/decimal64.h"
}

#include <functional>

inline double operator/(fm_decimal64_t a, fm_decimal64_t b) {
  return fm_decimal64_div(a, b);
}

inline fm_decimal64_t operator/(fm_decimal64_t a, int b) {
  return fm_decimal64_intdiv(a, b);
}

inline bool operator==(fm_decimal64_t a, fm_decimal64_t b) {
  return fm_decimal64_equal(a, b);
}

inline bool operator!=(fm_decimal64_t a, fm_decimal64_t b) {
  return !fm_decimal64_equal(a, b);
}

inline fm_decimal64_t operator+(fm_decimal64_t a, fm_decimal64_t b) {
  return fm_decimal64_add(a, b);
}

inline fm_decimal64_t &operator+=(fm_decimal64_t &a, fm_decimal64_t b) {
  fm_decimal64_inplace_add(&a, b);
  return a;
}

inline fm_decimal64_t &operator-=(fm_decimal64_t &a, fm_decimal64_t b) {
  fm_decimal64_inplace_sub(&a, b);
  return a;
}

inline fm_decimal64_t operator-(fm_decimal64_t a, fm_decimal64_t b) {
  return fm_decimal64_sub(a, b);
}

inline bool operator<(fm_decimal64_t a, fm_decimal64_t b) {
  return fm_decimal64_less(a, b);
}

inline bool operator>(fm_decimal64_t a, fm_decimal64_t b) {
  return fm_decimal64_less(b, a);
}

inline bool operator<=(fm_decimal64_t a, fm_decimal64_t b) {
  return !fm_decimal64_less(b, a);
}

inline bool operator>=(fm_decimal64_t a, fm_decimal64_t b) {
  return !fm_decimal64_less(a, b);
}

inline fm_decimal64_t operator*(fm_decimal64_t a, int64_t b) {
  return fm_decimal64_mul(a, b);
}

inline fm_decimal64_t operator*(int64_t a, fm_decimal64_t b) {
  return fm_decimal64_mul(b, a);
}

namespace std {

template <>
struct hash<fm_decimal64_t>
{
  size_t operator()(const fm_decimal64_t& k) const
  {
    return std::hash<int64_t>{}(k.value);
  }
};

}