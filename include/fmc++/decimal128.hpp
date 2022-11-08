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

#include "fmc++/convert.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/side.hpp"

#include "fmc/alignment.h"
#include "fmc/decimal128.h"

#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <ostream>

namespace fmc {

class decimal128 : public fmc_decimal128_t {
public:
  constexpr decimal128(const fmc_decimal128_t &a) noexcept
      : fmc_decimal128_t(a) {}
  decimal128(int i) noexcept { fmc_decimal128_from_int(this, i); }
  decimal128(int64_t i) noexcept { fmc_decimal128_from_int(this, i); }
  decimal128(uint i) noexcept { fmc_decimal128_from_uint(this, i); }
  decimal128(uint64_t i) noexcept { fmc_decimal128_from_uint(this, i); }
  decimal128(fmc_rprice_t d) noexcept {
    static decimal128 dec64div((int64_t)FMC_RPRICE_FRACTION);

    decimal128 dd;
    fmc_decimal128_from_int(&dd, d.value);
    fmc_decimal128_div(this, &dd, &dec64div);
  }
  decimal128(double d) noexcept { fmc_decimal128_from_double(this, d); }
  constexpr decimal128() noexcept : fmc_decimal128_t{{0, 0}} {}
  decimal128 &operator=(const fmc_decimal128_t &a) noexcept {
    memcpy(this->longs, a.longs, sizeof(a.longs));
    return *this;
  }
  decimal128 &operator=(const int &a) noexcept {
    fmc_decimal128_from_int(this, a);
    return *this;
  }
  decimal128 &operator=(const int64_t &a) noexcept {
    fmc_decimal128_from_int(this, a);
    return *this;
  }
  decimal128 &operator=(const uint &a) noexcept {
    fmc_decimal128_from_uint(this, a);
    return *this;
  }
  decimal128 &operator=(const uint64_t &a) noexcept {
    fmc_decimal128_from_uint(this, a);
    return *this;
  }
  decimal128 &operator=(const double &a) noexcept {
    fmc_decimal128_from_double(this, a);
    return *this;
  }
  decimal128 &operator=(const float &a) noexcept {
    fmc_decimal128_from_double(this, a);
    return *this;
  }
  static constexpr decimal128 &upcast(fmc_decimal128_t &a) noexcept {
    return static_cast<decimal128 &>(a);
  }
  static constexpr const decimal128 &
  upcast(const fmc_decimal128_t &a) noexcept {
    return static_cast<const decimal128 &>(a);
  }
  decimal128 &operator+=(const decimal128 &a) noexcept {
    fmc_decimal128_inc(this, &a);
    return *this;
  }
  decimal128 &operator-=(const decimal128 &a) noexcept {
    fmc_decimal128_dec(this, &a);
    return *this;
  }
  decimal128 operator-() const noexcept {
    decimal128 res;
    fmc_decimal128_negate(&res, this);
    return res;
  }
  explicit operator int() const noexcept {
    int64_t ret;
    fmc_error_t *err;
    fmc_decimal128_to_int(&ret, this, &err);
    return ret;
  }
  explicit operator int64_t() const noexcept {
    int64_t ret;
    fmc_error_t *err;
    fmc_decimal128_to_int(&ret, this, &err);
    return ret;
  }
  explicit operator double() const noexcept {
    double value;
    fmc_decimal128_to_double(&value, this);
    return value;
  }
  explicit operator float() const noexcept {
    double value;
    fmc_decimal128_to_double(&value, this);
    return value;
  }
  explicit operator rprice() const noexcept {
    static decimal128 dec64mul((int64_t)FMC_RPRICE_FRACTION);
    decimal128 tmp;
    fmc_decimal128_mul(&tmp, this, &dec64mul);
    int64_t num;
    fmc_error_t *err;
    fmc_decimal128_to_int(&num, &tmp, &err);
    rprice ret;
    fmc_rprice_from_raw(&ret, num);
    return ret;
  }
};

template <> struct conversion<fmc_decimal128_t, double> {
  double operator()(fmc_decimal128_t x) noexcept {
    double value;
    fmc_decimal128_to_double(&value, &x);
    return value;
  }
};

template <> struct conversion<double, fmc_decimal128_t> {
  fmc_decimal128_t operator()(double x) noexcept {
    fmc_decimal128_t res;
    fmc_decimal128_from_double(&res, x);
    return res;
  }
};

} // namespace fmc

inline bool operator==(const fmc_decimal128_t &a,
                       const fmc_decimal128_t &b) noexcept {
  return fmc_decimal128_equal(&a, &b);
}

inline bool operator!=(const fmc_decimal128_t &a,
                       const fmc_decimal128_t &b) noexcept {
  return !fmc_decimal128_equal(&a, &b);
}

inline bool operator<(const fmc_decimal128_t &a,
                      const fmc_decimal128_t &b) noexcept {
  return fmc_decimal128_less(&a, &b);
}

inline bool operator<=(const fmc_decimal128_t &a,
                       const fmc_decimal128_t &b) noexcept {
  return fmc_decimal128_less_or_equal(&a, &b);
}

inline bool operator>(const fmc_decimal128_t &a,
                      const fmc_decimal128_t &b) noexcept {
  return fmc_decimal128_greater(&a, &b);
}

inline bool operator>=(const fmc_decimal128_t &a,
                       const fmc_decimal128_t &b) noexcept {
  return fmc_decimal128_greater_or_equal(&a, &b);
}

inline fmc::decimal128 operator+(const fmc::decimal128 &a,
                                 const fmc::decimal128 &b) noexcept {
  fmc::decimal128 res;
  fmc_decimal128_add(&res, &a, &b);
  return res;
}

inline fmc::decimal128 operator-(const fmc::decimal128 &a,
                                 const fmc::decimal128 &b) noexcept {
  fmc::decimal128 res;
  fmc_decimal128_sub(&res, &a, &b);
  return res;
}

inline fmc::decimal128 operator*(const fmc::decimal128 &a,
                                 const fmc::decimal128 &b) noexcept {
  fmc::decimal128 res;
  fmc_decimal128_mul(&res, &a, &b);
  return res;
}

inline fmc::decimal128 operator/(const fmc::decimal128 &a,
                                 const fmc::decimal128 &b) noexcept {
  fmc::decimal128 res;
  fmc_decimal128_div(&res, &a, &b);
  return res;
}

inline fmc::decimal128 operator/(const fmc::decimal128 &a,
                                 const int64_t &b) noexcept {
  return a / fmc::decimal128(b);
}

namespace std {

template <> class numeric_limits<fmc::decimal128> {
public:
  static fmc::decimal128 min() noexcept {
    fmc::decimal128 res;
    fmc_decimal128_min(&res);
    return res;
  }
  static fmc::decimal128 max() noexcept {
    fmc::decimal128 res;
    fmc_decimal128_max(&res);
    return res;
  }
  static fmc::decimal128 infinity() noexcept {
    fmc::decimal128 res;
    fmc_decimal128_inf(&res);
    return res;
  }
  static fmc::decimal128 quiet_NaN() noexcept {
    fmc::decimal128 res;
    fmc_decimal128_qnan(&res);
    return res;
  }
  static fmc::decimal128 signaling_NaN() noexcept {
    fmc::decimal128 res;
    fmc_decimal128_snan(&res);
    return res;
  }
  static fmc::decimal128 epsilon() noexcept {
    return fmc::decimal128((int64_t)0);
  }
};

inline ostream &operator<<(ostream &os, const fmc_decimal128_t &r) {
  char str[FMC_DECIMAL128_STR_SIZE];
  fmc_decimal128_to_str(str, &r);
  os << str;
  return os;
}

inline string to_string(const fmc_decimal128_t &r) {
  char str[FMC_DECIMAL128_STR_SIZE];
  fmc_decimal128_to_str(str, &r);
  return string(str);
}

inline istream &operator>>(istream &os, fmc_decimal128_t &r) {
  string str;
  os >> str;
  fmc_error_t *err;
  fmc_decimal128_from_str(&r, str.c_str(), &err);
  fmc_runtime_error_unless(!err) << "unable to build decimal from string";
  return os;
}

template <typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::decimal128> ||
                                     std::is_same_v<T, fmc_decimal128_t>,
                                 bool>
isinf(T x) {
  return fmc_decimal128_is_inf(&x);
}

template <typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::decimal128> ||
                                     std::is_same_v<T, fmc_decimal128_t>,
                                 bool>
isfinite(T x) {
  return fmc_decimal128_is_finite(&x);
}

template <typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::decimal128> ||
                                     std::is_same_v<T, fmc_decimal128_t>,
                                 fmc::decimal128>
abs(T x) {
  fmc::decimal128 res;
  fmc_decimal128_abs(&res, &x);
  return res;
}

template <typename T>
inline typename std::enable_if_t<std::is_same_v<T, fmc::decimal128> ||
                                     std::is_same_v<T, fmc_decimal128_t>,
                                 bool>
isnan(T x) {
  return fmc_decimal128_is_nan(&x);
}

template <> struct hash<fmc_decimal128_t> {
  size_t operator()(const fmc_decimal128_t &k) const noexcept {
    static_assert(sizeof(k.longs) / sizeof(int64_t) == 2);
    return fmc_hash_combine(std::hash<int64_t>{}(*(int64_t *)&k.longs[0]),
                            std::hash<int64_t>{}(*(int64_t *)&k.longs[1]));
  }
};

} // namespace std

namespace fmc {

template <> struct sided_initializer<decimal128> {
  static constexpr bool is_specialized = true;
  static decimal128 min() noexcept {
    return -std::numeric_limits<fmc::decimal128>::infinity();
  }
  static decimal128 max() noexcept {
    return std::numeric_limits<fmc::decimal128>::infinity();
  }
};

} // namespace fmc
