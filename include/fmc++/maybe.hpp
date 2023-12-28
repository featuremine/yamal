/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file include/fmc++/maybe.hpp
 * @date Dec 23 2023
 */

#pragma once

#include "fmc++/side.hpp"

#include <iostream>
#include <optional>

namespace fmc {
template <typename T> class maybe : public std::optional<T> {
public:
  maybe() = default;
  maybe(const T &x) : std::optional<T>(x) {}
  maybe(const maybe<T> &x) : std::optional<T>(x.value()) {}

  maybe<T> operator-() const;
  maybe<T> &operator+=(const maybe<T> &rhs);
  maybe<T> &operator-=(const maybe<T> &rhs);
  maybe<T> &operator*=(const maybe<T> &rhs);
  maybe<T> &operator/=(const maybe<T> &rhs);
};

template <typename T> inline maybe<T> maybe<T>::operator-() const {
  return this->has_value() ? maybe<T>(this->value()) : maybe<T>();
}

template <typename T>
inline maybe<T> &maybe<T>::operator+=(const maybe<T> &rhs) {
  if (this->has_value() && rhs.has_value())
    this->emplace(this->value() + rhs.value());
  else
    this->reset();
  return *this;
}

template <typename T>
inline maybe<T> &maybe<T>::operator-=(const maybe<T> &rhs) {
  if (this->has_value() && rhs.has_value())
    this->emplace(this->value() - rhs.value());
  else
    this->reset();
  return *this;
}

template <typename T>
inline maybe<T> &maybe<T>::operator*=(const maybe<T> &rhs) {
  if (this->has_value() && rhs.has_value())
    this->emplace(this->value() * rhs.value());
  else
    this->reset();
  return *this;
}

template <typename T>
inline maybe<T> &maybe<T>::operator/=(const maybe<T> &rhs) {
  if (this->has_value() && rhs.has_value())
    this->emplace(this->value() / rhs.value());
  else
    this->reset();
  return *this;
}

template <typename T>
static inline maybe<T> operator+(const maybe<T> &lhs, const maybe<T> &rhs) {
  return lhs.has_value() && rhs.has_value()
             ? maybe<T>(lhs.value() + rhs.value())
             : maybe<T>();
}

template <typename T>
static inline maybe<T> operator-(const maybe<T> &lhs, const maybe<T> &rhs) {
  return lhs.has_value() && rhs.has_value()
             ? maybe<T>(lhs.value() - rhs.value())
             : maybe<T>();
}

template <typename T>
static inline maybe<T> operator*(const maybe<T> &lhs, const maybe<T> &rhs) {
  return lhs.has_value() && rhs.has_value()
             ? maybe<T>(lhs.value() * rhs.value())
             : maybe<T>();
}

template <typename T>
static inline maybe<T> operator/(const maybe<T> &lhs, const maybe<T> &rhs) {
  return lhs.has_value() && rhs.has_value()
             ? maybe<T>(lhs.value() / rhs.value())
             : maybe<T>();
}

template <typename T, typename U>
static inline maybe<T> operator+(const maybe<T> &lhs, const U &rhs) {
  return lhs.has_value() ? maybe<T>(lhs.value() + rhs) : maybe<T>();
}

template <typename T, typename U>
static inline maybe<T> operator-(const maybe<T> &lhs, const U &rhs) {
  return lhs.has_value() ? maybe<T>(lhs.value() - rhs) : maybe<T>();
}

template <typename T, typename U>
static inline maybe<T> operator*(const maybe<T> &lhs, const U &rhs) {
  return lhs.has_value() ? maybe<T>(lhs.value() * rhs) : maybe<T>();
}

template <typename T, typename U>
static inline maybe<T> operator/(const maybe<T> &lhs, const U &rhs) {
  return lhs.has_value() ? maybe<T>(lhs.value() / rhs) : maybe<T>();
}

template <typename T, typename U>
static inline maybe<T> operator+(const U &lhs, const maybe<T> &rhs) {
  return rhs.has_value() ? maybe<T>(lhs + rhs.value()) : maybe<T>();
}

template <typename T, typename U>
static inline maybe<T> operator-(const U &lhs, const maybe<T> &rhs) {
  return rhs.has_value() ? maybe<T>(lhs - rhs.value()) : maybe<T>();
}

template <typename T, typename U>
static inline maybe<T> operator*(const U &lhs, const maybe<T> &rhs) {
  return rhs.has_value() ? maybe<T>(lhs * rhs.value()) : maybe<T>();
}

template <typename T, typename U>
static inline maybe<T> operator/(const U &lhs, const maybe<T> &rhs) {
  return rhs.has_value() ? maybe<T>(lhs / rhs.value()) : maybe<T>();
}

} // namespace fmc

namespace std {
template <typename T> struct numeric_limits<fmc::maybe<T>> {
  static const bool is_specialized = true;

  static fmc::maybe<T> min() noexcept {
    return fmc::maybe<T>(numeric_limits<T>::min());
  }
  static fmc::maybe<T> max() noexcept {
    return fmc::maybe<T>(numeric_limits<T>::max());
  }

  static const int digits = numeric_limits<T>::digits;
  static const int digits10 = numeric_limits<T>::digits10;
  static const bool is_signed = numeric_limits<T>::is_signed;
  static const bool is_integer = numeric_limits<T>::is_integer;
  static const bool is_exact = numeric_limits<T>::is_exact;
  static const int radix = numeric_limits<T>::radix;
  static fmc::maybe<T> epsilon() noexcept {
    return fmc::maybe<T>(numeric_limits<T>::epsilon());
  }
  static fmc::maybe<T> round_error() {
    return fmc::maybe<T>(numeric_limits<T>::round_error());
  }

  static const int min_exponent = numeric_limits<T>::min_exponent;
  static const int min_exponent10 = numeric_limits<T>::min_exponent10;
  static const int max_exponent = numeric_limits<T>::max_exponent;
  static const int max_exponent10 = numeric_limits<T>::max_exponent10;

  static const bool has_infinity = numeric_limits<T>::has_infinity;
  static const bool has_quiet_NaN = numeric_limits<T>::has_quiet_NaN;
  static const bool has_signaling_NaN = numeric_limits<T>::has_signaling_NaN;
  static const float_denorm_style has_denorm = numeric_limits<T>::has_denorm;
  static const bool has_denorm_loss = numeric_limits<T>::has_denorm_loss;

  static fmc::maybe<T> infinity() noexcept {
    return fmc::maybe<T>(numeric_limits<T>::infinity());
  }
  static fmc::maybe<T> quiet_NaN() noexcept {
    return fmc::maybe<T>(numeric_limits<T>::quiet_NaN());
  }
  static fmc::maybe<T> signaling_NaN() noexcept {
    return fmc::maybe<T>(numeric_limits<T>::signaling_NaN());
  }
  static fmc::maybe<T> denorm_min() noexcept {
    return fmc::maybe<T>(numeric_limits<T>::denorm_min());
  }

  static const bool is_iec559 = numeric_limits<T>::is_iec559;
  static const bool is_bounded = numeric_limits<T>::is_bounded;
  static const bool is_modulo = numeric_limits<T>::is_modulo;

  static const bool traps = numeric_limits<T>::traps;
  static const bool tinyness_before = numeric_limits<T>::tinyness_before;
  static const float_round_style round_style = numeric_limits<T>::round_style;
};

template <typename T> inline bool isinf(const fmc::maybe<T> &x) {
  return x.has_value() && isinf(x.value());
}

template <typename T> inline bool isfinite(const fmc::maybe<T> &x) {
  return x.has_value() && isfinite(x.value());
}

template <typename T> inline fmc::maybe<T> abs(const fmc::maybe<T> &x) {
  return x.has_value() ? fmc::maybe<T>(abs(x.value())) : fmc::maybe<T>();
}

template <typename T>
inline fmc::maybe<T> pow(const fmc::maybe<T> &x, uint64_t n) {
  return x.has_value() ? fmc::maybe<T>(pow(x.value(), n)) : fmc::maybe<T>();
}

template <typename T> inline bool isnan(const fmc::maybe<T> &x) {
  return !x.has_value() || isnan(x.value());
}

template <typename T> struct hash<fmc::maybe<T>> {
  size_t operator()(const fmc::maybe<T> &x) const noexcept {
    return x.has_value() ? std::hash<T>{}(x) : 0;
  }
};

template <typename T>
struct is_floating_point<fmc::maybe<T>>
    : std::integral_constant<bool, is_floating_point<T>::value> {};

} // namespace std

namespace fmc {
template <typename T> struct sided_initializer<fmc::maybe<T>> {
  static constexpr bool is_specialized = true;
  static fmc::maybe<T> min() noexcept { return maybe<T>(); }
  static fmc::maybe<T> max() noexcept { return maybe<T>(); }
};

} // namespace fmc
