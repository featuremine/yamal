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
 * @file rational64.hpp
 * @author Maxim Trokhimtchouk
 * @date 29 Dec 2018
 * @brief File contains C++ definitions for the rational64 object
 *
 * This file contains C++ declarations of the rational64 object operations
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "fmc/rational64.h"
}

#include "fmc++/side.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

namespace fmc {

class rational64 : public fmc_rational64_t {
public:
  constexpr rational64(const fmc_rational64_t &a) noexcept
      : fmc_rational64_t(a) {}
  rational64(int i) noexcept { fmc_rational64_from_int(this, i); }
  rational64(int64_t i) noexcept { fmc_rational64_from_int(this, i); }
  rational64(uint i) noexcept { fmc_rational64_from_int(this, i); }
  rational64(uint64_t i) noexcept { fmc_rational64_from_int(this, i); }
  rational64(double d) noexcept { fmc_rational64_from_double(this, d, 32); }
  constexpr rational64() noexcept : fmc_rational64_t{0, 1} {}
  constexpr rational64(int32_t num, int32_t den) noexcept
      : fmc_rational64_t{num, den} {}
  rational64 &operator=(const fmc_rational64_t &a) noexcept {
    num = a.num;
    den = a.den;
    return *this;
  }
  rational64 &operator=(const int &a) noexcept {
    fmc_rational64_from_int(this, a);
    return *this;
  }
  rational64 &operator=(const int64_t &a) noexcept {
    fmc_rational64_from_int(this, a);
    return *this;
  }
  rational64 &operator=(const uint &a) noexcept {
    fmc_rational64_from_int(this, a);
    return *this;
  }
  rational64 &operator=(const uint64_t &a) noexcept {
    fmc_rational64_from_int(this, a);
    return *this;
  }
  rational64 &operator=(const double &a) noexcept {
    fmc_rational64_from_double(this, a, 32);
    return *this;
  }
  rational64 &operator=(const float &a) noexcept {
    fmc_rational64_from_double(this, a, 32);
    return *this;
  }
  static constexpr rational64 &upcast(fmc_rational64_t &a) noexcept {
    return static_cast<rational64 &>(a);
  }
  static constexpr const rational64 &
  upcast(const fmc_rational64_t &a) noexcept {
    return static_cast<const rational64 &>(a);
  }
  rational64 &operator+=(const rational64 &a) noexcept {
    fmc_rational64_inc(this, &a);
    return *this;
  }
  rational64 &operator-=(const rational64 &a) noexcept {
    fmc_rational64_dec(this, &a);
    return *this;
  }
  rational64 operator-() const noexcept {
    rational64 res;
    fmc_rational64_negate(&res, this);
    return res;
  }
  explicit operator int() const noexcept {
    int64_t res;
    fmc_rational64_to_int(&res, this);
    return res;
  }
  explicit operator int64_t() const noexcept {
    int64_t res;
    fmc_rational64_to_int(&res, this);
    return res;
  }
  explicit operator double() const noexcept {
    double res;
    fmc_rational64_to_double(&res, this);
    return res;
  }
  explicit operator float() const noexcept {
    double res;
    fmc_rational64_to_double(&res, this);
    return res;
  }
};

template <> struct sided_initializer<fmc::rational64> {
  static constexpr bool is_specialized = true;
  static fmc::rational64 min() noexcept { return FMC_RATIONAL64_MIN; }
  static fmc::rational64 max() noexcept { return FMC_RATIONAL64_MAX; }
};

template <> struct conversion<fmc_rational64_t, double> {
  double operator()(fmc_rational64_t x) {
    double ret;
    fmc_rational64_to_double(&ret, &x);
    return ret;
  }
};

template <> struct conversion<double, fmc_rational64_t> {
  fmc_rational64_t operator()(double x) {
    rational64 ret;
    fmc_rational64_from_double(&ret, x, 32);
    return ret;
  }
};

} // namespace fmc

inline fmc::rational64 operator/(fmc::rational64 a, fmc::rational64 b) {
  fmc::rational64 ret;
  fmc_rational64_div(&ret, &a, &b);
  return ret;
}

inline bool operator==(fmc::rational64 a, fmc::rational64 b) {
  return fmc_rational64_equal(&a, &b);
}

inline bool operator!=(fmc::rational64 a, fmc::rational64 b) {
  return fmc_rational64_notequal(&a, &b);
}

inline fmc::rational64 operator+(fmc::rational64 a, fmc::rational64 b) {
  fmc::rational64 ret;
  fmc_rational64_add(&ret, &a, &b);
  return ret;
}

inline fmc::rational64 operator-(fmc::rational64 a, fmc::rational64 b) {
  fmc::rational64 ret;
  fmc_rational64_sub(&ret, &a, &b);
  return ret;
}

inline bool operator<(fmc::rational64 a, fmc::rational64 b) {
  return fmc_rational64_less(&a, &b);
}

inline bool operator>(fmc::rational64 a, fmc::rational64 b) {
  return fmc_rational64_greater(&a, &b);
}

inline bool operator<=(fmc::rational64 a, fmc::rational64 b) {
  return !fmc_rational64_greater(&a, &b);
}

inline bool operator>=(fmc::rational64 a, fmc::rational64 b) {
  return !fmc_rational64_less(&a, &b);
}

inline fmc::rational64 operator*(fmc::rational64 a, fmc::rational64 b) {
  fmc::rational64 ret;
  fmc_rational64_mul(&ret, &a, &b);
  return ret;
}

namespace std {
template <> class numeric_limits<fmc::rational64> {
public:
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = true;
  static constexpr bool is_integer = false;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = true;
  static constexpr bool has_quiet_NaN = true;
  static constexpr bool has_signaling_NaN = false;
  static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
  static constexpr bool has_denorm_loss = false;
  static constexpr std::float_round_style round_style = std::round_toward_zero;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = false;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
  static constexpr bool traps = false;
  static constexpr bool tinyness_before = true;
  static constexpr fmc::rational64 min() noexcept {
    return fmc::rational64(numeric_limits<int32_t>::min(), 1);
  }
  static constexpr fmc::rational64 lowest() noexcept {
    return fmc::rational64(numeric_limits<int32_t>::min(), 1);
  }
  static constexpr fmc::rational64 max() noexcept {
    return fmc::rational64(numeric_limits<int32_t>::max(), 1);
  }
  static constexpr fmc::rational64 epsilon() noexcept {
    return fmc::rational64(0, 1);
  }
  static constexpr fmc::rational64 round_error() noexcept {
    return fmc::rational64(1, 2);
  }
  static constexpr fmc::rational64 quiet_NaN() noexcept {
    return fmc::rational64(0, 0);
  }
  static constexpr fmc::rational64 infinity() noexcept {
    return fmc::rational64(1, 0);
  }
};

inline ostream &operator<<(ostream &s, const fmc::rational64 &x) {
  return s << x.num << "/" << x.den;
}
inline istream &operator>>(istream &s, fmc::rational64 &x) {
  string str;
  s >> str;
  size_t div_pos = str.find("/");
  if (div_pos == string::npos) {
    div_pos = str.find(".");
    auto den = str.substr(div_pos + 1, str.size() - div_pos - 1);
    int32_t buff = 10;
    for (size_t i = 0; i < den.size() - 1; ++i)
      buff *= 10;
    x.num = stoi(str.substr(0, div_pos)) * buff + stoi(den);
    x.den = buff;
  } else {
    x.num = stoi(str.substr(0, div_pos));
    x.den = stoi(str.substr(div_pos + 1, str.size() - div_pos - 1));
  }
  return s;
}

template<typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::rational64> || std::is_same_v<T, fmc_rational64_t>, bool>
isinf(T x)
{
  return fmc_rational64_is_inf(&x);
}

template<typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::rational64> || std::is_same_v<T, fmc_rational64_t>, bool>
isfinite(T x)
{
  return fmc_rational64_is_finite(&x);
}

template<typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::rational64> || std::is_same_v<T, fmc_rational64_t>, fmc::rational64>
abs(T x)
{
  fmc::rational64 res;
  fmc_rational64_abs(&res, &x);
  return res;
}

template<typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::rational64> || std::is_same_v<T, fmc_rational64_t>, bool>
isnan(T x)
{
  return fmc_rational64_is_nan(&x);
}

inline string to_string(const fmc::rational64 &x) {
  return to_string(x.num) + "/" + to_string(x.den);
}

}; // namespace std
