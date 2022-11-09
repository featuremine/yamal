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
 * @file rprice.hpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions for the rprice object
 *
 * This file contains C++ declarations of the rprice object operations
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "fmc/rprice.h"
}

#include "fmc++/convert.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/side.hpp"

#include <functional>
#include <iomanip>
#include <iostream>

namespace fmc {

class rprice : public fmc_rprice_t {
public:
  constexpr rprice(const fmc_rprice_t &a) noexcept : fmc_rprice_t(a) {}
  rprice(int i) noexcept { fmc_rprice_from_int(this, i); }
  rprice(int64_t i) noexcept { fmc_rprice_from_int(this, i); }
  rprice(uint i) noexcept { fmc_rprice_from_int(this, i); }
  rprice(uint64_t i) noexcept { fmc_rprice_from_int(this, i); }
  rprice(double d) noexcept { fmc_rprice_from_double(this, d); }
  constexpr rprice() noexcept : fmc_rprice_t{0} {}
  rprice &operator=(const fmc_rprice_t &a) noexcept {
    value = a.value;
    return *this;
  }
  rprice &operator=(const int &a) noexcept {
    fmc_rprice_from_int(this, a);
    return *this;
  }
  rprice &operator=(const int64_t &a) noexcept {
    fmc_rprice_from_int(this, a);
    return *this;
  }
  rprice &operator=(const uint &a) noexcept {
    fmc_rprice_from_int(this, a);
    return *this;
  }
  rprice &operator=(const uint64_t &a) noexcept {
    fmc_rprice_from_int(this, a);
    return *this;
  }
  rprice &operator=(const double &a) noexcept {
    fmc_rprice_from_double(this, a);
    return *this;
  }
  rprice &operator=(const float &a) noexcept {
    fmc_rprice_from_double(this, a);
    return *this;
  }
  static constexpr rprice &upcast(fmc_rprice_t &a) noexcept {
    return static_cast<rprice &>(a);
  }
  static constexpr const rprice &upcast(const fmc_rprice_t &a) noexcept {
    return static_cast<const rprice &>(a);
  }
  rprice &operator+=(const rprice &a) noexcept {
    fmc_rprice_inc(this, &a);
    return *this;
  }
  rprice &operator-=(const rprice &a) noexcept {
    fmc_rprice_dec(this, &a);
    return *this;
  }
  rprice operator-() const noexcept {
    rprice res;
    fmc_rprice_negate(&res, this);
    return res;
  }
  explicit operator int() const noexcept {
    int64_t res;
    fmc_rprice_to_int(&res, this);
    return res;
  }
  explicit operator int64_t() const noexcept {
    int64_t res;
    fmc_rprice_to_int(&res, this);
    return res;
  }
  explicit operator double() const noexcept {
    double res;
    fmc_rprice_to_double(&res, this);
    return res;
  }
  explicit operator float() const noexcept {
    double res;
    fmc_rprice_to_double(&res, this);
    return res;
  }
};

template <> struct sided_initializer<fmc::rprice> {
  static constexpr bool is_specialized = true;
  static fmc::rprice min() noexcept { return FMC_RPRICE_MIN; }
  static fmc::rprice max() noexcept { return FMC_RPRICE_MAX; }
};

template <> struct conversion<fmc_rprice_t, double> {
  double operator()(fmc_rprice_t x) {
    double ret;
    fmc_rprice_to_double(&ret, &x);
    return ret;
  }
};

template <> struct conversion<double, fmc_rprice_t> {
  fmc_rprice_t operator()(double x) {
    rprice ret;
    fmc_rprice_from_double(&ret, x);
    return ret;
  }
};

} // namespace fmc

inline fmc::rprice operator/(const fmc::rprice a,
                             const fmc::rprice b) noexcept {
  fmc::rprice ret;
  fmc_rprice_div(&ret, &a, &b);
  return ret;
}

inline fmc::rprice operator/(const fmc::rprice a, const int b) noexcept {
  fmc::rprice ret;
  fmc_rprice_int_div(&ret, &a, b);
  return ret;
}

inline bool operator==(const fmc::rprice a, const fmc::rprice b) noexcept {
  return fmc_rprice_equal(&a, &b);
}

inline bool operator!=(const fmc::rprice a, const fmc::rprice b) noexcept {
  return !fmc_rprice_equal(&a, &b);
}

inline fmc::rprice operator+(const fmc::rprice a,
                             const fmc::rprice b) noexcept {
  fmc::rprice ret;
  fmc_rprice_add(&ret, &a, &b);
  return ret;
}

inline fmc::rprice operator-(const fmc::rprice a,
                             const fmc::rprice b) noexcept {
  fmc::rprice ret;
  fmc_rprice_sub(&ret, &a, &b);
  return ret;
}

inline bool operator<(const fmc::rprice a, const fmc::rprice b) noexcept {
  return fmc_rprice_less(&a, &b);
}

inline bool operator>(const fmc::rprice a, const fmc::rprice b) noexcept {
  return fmc_rprice_greater(&a, &b);
}

inline bool operator<=(const fmc::rprice a, const fmc::rprice b) noexcept {
  return fmc_rprice_less_or_equal(&a, &b);
}

inline bool operator>=(const fmc::rprice a, const fmc::rprice b) noexcept {
  return fmc_rprice_greater_or_equal(&a, &b);
}

inline fmc::rprice operator*(const fmc::rprice a,
                             const fmc::rprice b) noexcept {
  fmc::rprice ret;
  fmc_rprice_mul(&ret, &a, &b);
  return ret;
}

inline fmc::rprice operator*(const fmc::rprice a, const int64_t b) noexcept {
  fmc::rprice ret, db;
  fmc_rprice_from_int(&db, b);
  fmc_rprice_mul(&ret, &a, &db);
  return ret;
}

inline fmc::rprice operator*(const int64_t a, const fmc::rprice b) noexcept {
  fmc::rprice ret, da;
  fmc_rprice_from_int(&da, a);
  fmc_rprice_mul(&ret, &da, &b);
  return ret;
}

namespace std {

inline ostream &operator<<(ostream &s, const fmc::rprice &x) {
  using namespace std;
  return s << setprecision(15) << fmc::to<double>(x);
}

inline istream &operator>>(istream &s, fmc::rprice &x) {
  using namespace std;
  double d;
  s >> d;
  fmc_runtime_error_unless(s.eof()) << "unable to stream data into rprice";
  x = fmc::to<fmc::rprice>(d);
  return s;
}

inline string to_string(fmc::rprice &x) {
  return to_string(fmc::to<double>(x));
}

template <> class numeric_limits<fmc::rprice> {
public:
  static fmc::rprice min() noexcept { return FMC_RPRICE_MIN; }
  static fmc::rprice max() noexcept { return FMC_RPRICE_MAX; }
  static fmc::rprice epsilon() noexcept { return fmc::rprice((int64_t)0); }
};

template <typename T>
inline typename std::enable_if_t<
    std::is_same_v<T, fmc::rprice> || std::is_same_v<T, fmc_rprice_t>, bool>
isinf(T x) {
  return false;
}

template <typename T>
inline typename std::enable_if_t<
    std::is_same_v<T, fmc::rprice> || std::is_same_v<T, fmc_rprice_t>, bool>
isfinite(T x) {
  return true;
}

template <typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::rprice> ||
                                     std::is_same_v<T, fmc_rprice_t>,
                                 fmc::rprice>
abs(T x) {
  fmc::rprice res;
  fmc_rprice_abs(&res, &x);
  return res;
}

template <typename T>
inline typename std::enable_if_t<
    std::is_same_v<T, fmc::rprice> || std::is_same_v<T, fmc_rprice_t>, bool>
isnan(T x) {
  return false;
}

template <> struct hash<fmc::rprice> {
  size_t operator()(const fmc::rprice &k) const {
    return std::hash<int64_t>{}(k.value);
  }
};

} // namespace std
