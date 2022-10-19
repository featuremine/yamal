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
 * @file include/fmc++/decimal128.hpp
 * @date Oct 18 2022
 */

#pragma once

#include "fmc/decimal128.h"

#include <limits>
#include <ostream>
#include <cmath>

namespace fmc {

class decimal128 : public fmc_decimal128_t {
public:
  decimal128(const fmc_decimal128_t &a) : fmc_decimal128_t(a) {}
  decimal128(int64_t i) : decimal128(fmc_decimal128_from_int(i)) {}
  decimal128 &operator=(const fmc_decimal128_t &a) {
    return *this;
  }
  static decimal128 &upcast(fmc_decimal128_t &a) {
    return static_cast<decimal128 &>(a);
  }
  static const decimal128 &upcast(const fmc_decimal128_t &a) {
    return static_cast<const decimal128 &>(a);
  }
  decimal128 &operator+=(const decimal128 &a) {
    fmc_decimal128_inc(this, a);
    return *this;
  }
  decimal128 &operator-=(const decimal128 &a) {
    fmc_decimal128_dec(this, a);
    return *this;
  }
};

inline bool operator==(const decimal128 &a, const decimal128 &b) {
  return fmc_decimal128_equal(a, b);
}

inline bool operator!=(const decimal128 &a, const decimal128 &b) {
  return !fmc_decimal128_equal(a, b);
}

inline bool operator<(const decimal128 &a, const decimal128 &b) {
  return fmc_decimal128_less(a, b);
}

inline bool operator<=(const decimal128 &a, const decimal128 &b) {
  return fmc_decimal128_less_or_equal(a, b);
}

inline bool operator>(const decimal128 &a, const decimal128 &b) {
  return fmc_decimal128_greater(a, b);
}

inline bool operator>=(const decimal128 &a, const decimal128 &b) {
  return fmc_decimal128_greater_or_equal(a, b);
}

inline decimal128 operator+(const decimal128 &a, const decimal128 &b) {
  return decimal128::upcast(fmc_decimal128_add(a, b));
}

inline decimal128 operator-(const decimal128 &a, const decimal128 &b) {
  return decimal128::upcast(fmc_decimal128_sub(a, b));
}

inline decimal128 operator*(const decimal128 &a, const decimal128 &b) {
  return decimal128::upcast(fmc_decimal128_mul(a, b));
}

inline decimal128 operator/(const decimal128 &a, const decimal128 &b) {
  return decimal128::upcast(fmc_decimal128_div(a, b));
}

inline decimal128 operator/(const decimal128 &a, const int64_t &b) {
  return decimal128::upcast(fmc_decimal128_int_div(a, b));
}

} // namespace fmc

namespace std {
// limits and stream
template<>
class numeric_limits<fmc::decimal128> {
public:
  static fmc::decimal128 min() noexcept {
    return fmc::decimal128::upcast(fmc_decimal128_min());
  }
  static fmc::decimal128 max() noexcept {
    return fmc::decimal128::upcast(fmc_decimal128_max());
  }
  static fmc::decimal128 infinity() noexcept {
    return fmc::decimal128::upcast(fmc_decimal128_inf());
  }
  static fmc::decimal128 quiet_NaN() noexcept {
    return fmc::decimal128::upcast(fmc_decimal128_qnan());
  }
  static fmc::decimal128 signaling_NaN() noexcept {
    return fmc::decimal128::upcast(fmc_decimal128_snan());
  }
};

ostream &operator<<(ostream &os, const fmc::decimal128 &r) {
  char str[FMC_DECIMAL128_STR_SIZE];
  fmc_decimal128_to_str(r, str);
  os << str;
  return os;
}

inline bool isinf(fmc::decimal128 x) noexcept {
 return fmc_decimal128_is_inf(x);
}

inline bool isnan(fmc::decimal128 x) noexcept {
 return fmc_decimal128_is_nan(x);
}

} // namespace std