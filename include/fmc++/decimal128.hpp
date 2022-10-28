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
#include "fmc++/side.hpp"
#include "fmc/alignment.h"
#include "fmc/decimal128.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <ostream>

namespace fmc {

class decimal128 : public fmc_decimal128_t {
public:
  decimal128(const fmc_decimal128_t &a) : fmc_decimal128_t(a) {}
  decimal128(int64_t i) { fmc_decimal128_from_int(this, i); }
  decimal128(double d) {
    char str[FMC_DECIMAL128_STR_SIZE];
    snprintf(str, FMC_DECIMAL128_STR_SIZE, "%.15g", d);
    fmc_decimal128_from_str(this, str);
  }
  decimal128() { memset(bytes, 0, FMC_DECIMAL128_SIZE); }
  decimal128 &operator=(const fmc_decimal128_t &a) {
    memcpy(this->bytes, a.bytes, sizeof(a.bytes));
    return *this;
  }
  decimal128 &operator=(const int64_t &a) {
    *this = decimal128(a);
    return *this;
  }
  decimal128 &operator=(const double &a) {
    *this = decimal128(a);
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
  explicit operator int64_t() {
    int64_t ret;
    fmc_decimal128_to_int(&ret, this);
    return ret;
  }
  explicit operator double() {
    char str[FMC_DECIMAL128_STR_SIZE];
    fmc_decimal128_to_str(str, this);
    char *ptr = nullptr;
    return strtod(str, &ptr);
  }
};

inline bool operator==(const decimal128 &a, const decimal128 &b) noexcept {
  return fmc_decimal128_equal(&a, &b);
}

inline bool operator!=(const decimal128 &a, const decimal128 &b) noexcept {
  return !fmc_decimal128_equal(&a, &b);
}

inline bool operator<(const decimal128 &a, const decimal128 &b) noexcept {
  return fmc_decimal128_less(&a, &b);
}

inline bool operator<=(const decimal128 &a, const decimal128 &b) noexcept {
  return fmc_decimal128_less_or_equal(&a, &b);
}

inline bool operator>(const decimal128 &a, const decimal128 &b) noexcept {
  return fmc_decimal128_greater(&a, &b);
}

inline bool operator>=(const decimal128 &a, const decimal128 &b) noexcept {
  return fmc_decimal128_greater_or_equal(&a, &b);
}

inline decimal128 operator+(const decimal128 &a, const decimal128 &b) noexcept {
  decimal128 res;
  fmc_decimal128_add(&res, &a, &b);
  return res;
}

inline decimal128 operator-(const decimal128 &a, const decimal128 &b) noexcept {
  decimal128 res;
  fmc_decimal128_sub(&res, &a, &b);
  return res;
}

inline decimal128 operator*(const decimal128 &a, const decimal128 &b) noexcept {
  decimal128 res;
  fmc_decimal128_mul(&res, &a, &b);
  return res;
}

inline decimal128 operator/(const decimal128 &a, const decimal128 &b) noexcept {
  decimal128 res;
  fmc_decimal128_div(&res, &a, &b);
  return res;
}

inline decimal128 operator/(const decimal128 &a, const int64_t &b) noexcept {
  return a / decimal128(b);
}

template <> struct conversion<fmc_decimal128_t, double> {
  double operator()(fmc_decimal128_t x) {
    char str[FMC_DECIMAL128_STR_SIZE];
    fmc_decimal128_to_str(str, &x);
    char *ptr = nullptr;
    return strtod(str, &ptr);
  }
};

template <> struct conversion<double, fmc_decimal128_t> {
  fmc_decimal128_t operator()(double x) {
    fmc_decimal128_t res;
    char str[FMC_DECIMAL128_STR_SIZE];
    snprintf(str, FMC_DECIMAL128_STR_SIZE, "%.15g", x);
    fmc_decimal128_from_str(&res, str);
    return res;
  }
};

} // namespace fmc

inline bool operator==(const fmc_decimal128_t &a,
                       const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) == fmc::decimal128::upcast(b);
}

inline bool operator!=(const fmc_decimal128_t &a,
                       const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) != fmc::decimal128::upcast(b);
}

inline bool operator<(const fmc_decimal128_t &a,
                      const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) < fmc::decimal128::upcast(b);
}

inline bool operator<=(const fmc_decimal128_t &a,
                       const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) <= fmc::decimal128::upcast(b);
}

inline bool operator>(const fmc_decimal128_t &a,
                      const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) > fmc::decimal128::upcast(b);
}

inline bool operator>=(const fmc_decimal128_t &a,
                       const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) >= fmc::decimal128::upcast(b);
}

inline fmc_decimal128_t operator+(const fmc_decimal128_t &a,
                                  const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) + fmc::decimal128::upcast(b);
}

inline fmc_decimal128_t operator-(const fmc_decimal128_t &a,
                                  const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) - fmc::decimal128::upcast(b);
}

inline fmc_decimal128_t operator*(const fmc_decimal128_t &a,
                                  const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) * fmc::decimal128::upcast(b);
}

inline fmc_decimal128_t operator/(const fmc_decimal128_t &a,
                                  const fmc_decimal128_t &b) noexcept {
  return fmc::decimal128::upcast(a) / fmc::decimal128::upcast(b);
}

inline fmc_decimal128_t operator/(const fmc_decimal128_t &a,
                                  const int64_t &b) noexcept {
  return fmc::decimal128::upcast(a) / b;
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

inline ostream &operator<<(ostream &os, const fmc::decimal128 &r) noexcept {
  char str[FMC_DECIMAL128_STR_SIZE];
  fmc_decimal128_to_str(str, &r);
  os << str;
  return os;
}

inline ostream &operator<<(ostream &os, const fmc_decimal128_t &r) noexcept {
  return os << fmc::decimal128::upcast(r);
}

inline string to_string(const fmc::decimal128 &r) noexcept {
  char str[FMC_DECIMAL128_STR_SIZE];
  fmc_decimal128_to_str(str, &r);
  return string(str);
}

inline string to_string(const fmc_decimal128_t &r) noexcept {
  return to_string(fmc::decimal128::upcast(r));
}

inline istream &operator>>(istream &os, fmc::decimal128 &r) noexcept {
  string str;
  os >> str;
  fmc_decimal128_from_str(&r, str.c_str());
  return os;
}

inline istream &operator>>(istream &os, fmc_decimal128_t &r) noexcept {
  return os >> fmc::decimal128::upcast(r);
}

inline bool isinf(const fmc::decimal128 &x) noexcept {
  return fmc_decimal128_is_inf(&x);
}

inline bool isinf(const fmc_decimal128_t &x) noexcept {
  return std::isinf(fmc::decimal128::upcast(x));
}

inline bool isfinite(const fmc::decimal128 &x) noexcept {
  return fmc_decimal128_is_finite(&x);
}

inline bool isfinite(const fmc_decimal128_t &x) noexcept {
  return std::isfinite(fmc::decimal128::upcast(x));
}

inline fmc::decimal128 abs(const fmc::decimal128 &x) noexcept {
  fmc::decimal128 res;
  fmc_decimal128_abs(&res, &x);
  return res;
}

inline fmc_decimal128_t abs(fmc_decimal128_t x) noexcept {
  return std::abs(fmc::decimal128::upcast(x));
}

inline bool isnan(fmc::decimal128 x) noexcept {
  return fmc_decimal128_is_nan(&x);
}

inline bool isnan(fmc_decimal128_t x) noexcept {
  return std::isnan(fmc::decimal128::upcast(x));
}

} // namespace std

namespace fmc {

template <> struct sided_initializer<fmc::decimal128> {
  static constexpr bool is_specialized = true;
  static fmc::decimal128 min() noexcept {
    return -std::numeric_limits<fmc::decimal128>::infinity();
  }
  static fmc::decimal128 max() noexcept {
    return std::numeric_limits<fmc::decimal128>::infinity();
  }
};

} // namespace fmc
