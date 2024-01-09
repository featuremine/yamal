/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file include/fmc++/fxpt128.hpp
 * @date Oct 18 2022
 */

#pragma once

#include "fmc++/convert.hpp"
#include "fmc++/memory.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/side.hpp"

#include "fmc/alignment.h"
#include "fmc/fxpt128.h"

#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <ostream>

namespace fmc {

class fxpt128 : public fmc_fxpt128_t {
public:
  fxpt128();
  fxpt128(const fmc_fxpt128_t &c);
  fxpt128(const fmc_rprice_t &c);
  fxpt128(int);
  fxpt128(double);
  fxpt128(int64_t);
  fxpt128(uint64_t low, uint64_t high);

  static std::pair<fxpt128, std::string_view>
  from_string_view(fmc::string_view buf);
  std::string_view to_string_view(
      fmc::buffer buf,
      const fmc_fxpt128_format_t *format = &FXPT128_default_format) const;

  constexpr fxpt128 &upcast(fmc_fxpt128_t &a) noexcept;
  constexpr const fxpt128 &upcast(const fmc_fxpt128_t &a) noexcept;

  explicit operator double() const;
  explicit operator int64_t() const;
  explicit operator int() const;
  explicit operator rprice() const;
  explicit operator bool() const;

  bool operator!() const;
  fxpt128 operator~() const;
  fxpt128 operator-() const;
  fxpt128 &operator|=(const fxpt128 &rhs);
  fxpt128 &operator&=(const fxpt128 &rhs);
  fxpt128 &operator^=(const fxpt128 &rhs);
  fxpt128 &operator+=(const fxpt128 &rhs);
  fxpt128 &operator-=(const fxpt128 &rhs);
  fxpt128 &operator*=(const fxpt128 &rhs);
  fxpt128 &operator/=(const fxpt128 &rhs);
  fxpt128 &operator%=(const fxpt128 &rhs);
  fxpt128 &operator<<=(int amount);
  fxpt128 &operator>>=(int amount);
};

inline fxpt128::fxpt128() : fmc_fxpt128_t{0} {}

inline fxpt128::fxpt128(const fmc_fxpt128_t &c) : fmc_fxpt128_t{c} {}

inline fxpt128::fxpt128(const fmc_rprice_t &c) {
  fmc_fxpt128_from_rprice(this, &c);
}

inline fxpt128::fxpt128(int v) { fmc_fxpt128_from_int(this, v); }

inline fxpt128::fxpt128(double v) { fmc_fxpt128_from_double(this, v); }

inline fxpt128::fxpt128(int64_t v) { fmc_fxpt128_from_int(this, v); }

inline fxpt128::fxpt128(uint64_t low, uint64_t high) {
  lo = low;
  hi = high;
}

inline std::pair<fxpt128, std::string_view>
fxpt128::from_string_view(std::string_view buf) {
  fxpt128 res;
  const char *endptr = buf.data() + buf.size();
  fmc_fxpt128_from_string(&res, buf.data(), &endptr);
  return make_pair(res, std::string_view(buf.data(), endptr - buf.data()));
}

inline std::string_view
fxpt128::to_string_view(fmc::buffer buf,
                        const fmc_fxpt128_format_t *format) const {
  return std::string_view(
      buf.data(),
      fmc_fxpt128_to_string_opt(buf.data(), buf.size() - 1, this, format));
}

inline constexpr fxpt128 &fxpt128::upcast(fmc_fxpt128_t &a) noexcept {
  return static_cast<fxpt128 &>(a);
}

inline constexpr const fxpt128 &
fxpt128::upcast(const fmc_fxpt128_t &a) noexcept {
  return static_cast<const fxpt128 &>(a);
}

inline fxpt128::operator double() const { return fmc_fxpt128_to_double(this); }

inline fxpt128::operator int64_t() const { return fmc_fxpt128_to_int(this); }

inline fxpt128::operator int() const { return (int)fmc_fxpt128_to_int(this); }

inline fxpt128::operator rprice() const {
  fxpt128 tmp{(int64_t)FMC_RPRICE_FRACTION};
  tmp *= *this;
  return rprice::from_raw(int64_t(tmp));
}

inline fxpt128::operator bool() const { return lo || hi; }

inline bool fxpt128::operator!() const { return !lo && !hi; }

inline fxpt128 fxpt128::operator~() const {
  fxpt128 r;
  fmc_fxpt128_not(&r, this);
  return r;
}

inline fxpt128 fxpt128::operator-() const {
  fxpt128 r;
  fmc_fxpt128_neg(&r, this);
  return r;
}

inline fxpt128 &fxpt128::operator|=(const fxpt128 &rhs) {
  fmc_fxpt128_or(this, this, &rhs);
  return *this;
}

inline fxpt128 &fxpt128::operator&=(const fxpt128 &rhs) {
  fmc_fxpt128_and(this, this, &rhs);
  return *this;
}

inline fxpt128 &fxpt128::operator^=(const fxpt128 &rhs) {
  fmc_fxpt128_xor(this, this, &rhs);
  return *this;
}

inline fxpt128 &fxpt128::operator+=(const fxpt128 &rhs) {
  fmc_fxpt128_add(this, this, &rhs);
  return *this;
}

inline fxpt128 &fxpt128::operator-=(const fxpt128 &rhs) {
  fmc_fxpt128_sub(this, this, &rhs);
  return *this;
}

inline fxpt128 &fxpt128::operator*=(const fxpt128 &rhs) {
  fmc_fxpt128_mul(this, this, &rhs);
  return *this;
}

inline fxpt128 &fxpt128::operator/=(const fxpt128 &rhs) {
  fmc_fxpt128_div(this, this, &rhs);
  return *this;
}

inline fxpt128 &fxpt128::operator%=(const fxpt128 &rhs) {
  fmc_fxpt128_mod(this, this, &rhs);
  return *this;
}

inline fxpt128 &fxpt128::operator<<=(int amount) {
  fmc_fxpt128_shl(this, this, amount);
  return *this;
}

inline fxpt128 &fxpt128::operator>>=(int amount) {
  fmc_fxpt128_sar(this, this, amount);
  return *this;
}

static inline fxpt128 operator|(const fxpt128 &lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs);
  return r |= rhs;
}

static inline fxpt128 operator&(const fxpt128 &lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs);
  return r &= rhs;
}

static inline fxpt128 operator^(const fxpt128 &lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs);
  return r ^= rhs;
}

static inline fxpt128 operator+(const fxpt128 &lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs);
  return r += rhs;
}

static inline fxpt128 operator-(const fxpt128 &lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs);
  return r -= rhs;
}

static inline fxpt128 operator*(const fxpt128 &lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs);
  return r *= rhs;
}

static inline fxpt128 operator*(const int64_t lhs, const fxpt128 &rhs) {
  fxpt128 r((int64_t)lhs);
  return r *= rhs;
}

static inline fxpt128 operator*(const uint64_t lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs, 0);
  return r *= rhs;
}

static inline fxpt128 operator*(const fxpt128 &rhs, const uint64_t lhs) {
  fxpt128 r(lhs, 0);
  return r *= rhs;
}

static inline fxpt128 operator/(const fxpt128 &lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs);
  return r /= rhs;
}

static inline fxpt128 operator/(const int32_t lhs, const fxpt128 &rhs) {
  fxpt128 r((int64_t)lhs);
  return r /= rhs;
}

static inline fxpt128 operator/(const int64_t lhs, const fxpt128 &rhs) {
  fxpt128 r((int64_t)lhs);
  return r /= rhs;
}

static inline fxpt128 operator/(const uint32_t lhs, const fxpt128 &rhs) {
  fxpt128 r((uint64_t)lhs, 0);
  return r /= rhs;
}

static inline fxpt128 operator/(const uint64_t lhs, const fxpt128 &rhs) {
  fxpt128 r((uint64_t)lhs, 0);
  return r /= rhs;
}

static inline fxpt128 operator%(const fxpt128 &lhs, const fxpt128 &rhs) {
  fxpt128 r(lhs);
  return r %= rhs;
}

static inline fxpt128 operator<<(const fxpt128 &lhs, int amount) {
  fxpt128 r(lhs);
  return r <<= amount;
}

static inline fxpt128 operator>>(const fxpt128 &lhs, int amount) {
  fxpt128 r(lhs);
  return r >>= amount;
}

static inline bool operator<(const fxpt128 &lhs, const fxpt128 &rhs) {
  return fmc_fxpt128_cmp(&lhs, &rhs) < 0;
}

static inline bool operator>(const fxpt128 &lhs, const fxpt128 &rhs) {
  return fmc_fxpt128_cmp(&lhs, &rhs) > 0;
}

static inline bool operator<=(const fxpt128 &lhs, const fxpt128 &rhs) {
  return fmc_fxpt128_cmp(&lhs, &rhs) <= 0;
}

static inline bool operator>=(const fxpt128 &lhs, const fxpt128 &rhs) {
  return fmc_fxpt128_cmp(&lhs, &rhs) >= 0;
}

static inline bool operator==(const fxpt128 &lhs, const fxpt128 &rhs) {
  return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}

static inline bool operator!=(const fxpt128 &lhs, const fxpt128 &rhs) {
  return lhs.lo != rhs.lo || lhs.hi != rhs.hi;
}

} // namespace fmc

static inline fmc_fxpt128_t operator|(const fmc_fxpt128_t &lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs);
  return r |= rhs;
}

static inline fmc_fxpt128_t operator&(const fmc_fxpt128_t &lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs);
  return r &= rhs;
}

static inline fmc_fxpt128_t operator^(const fmc_fxpt128_t &lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs);
  return r ^= rhs;
}

static inline fmc_fxpt128_t operator+(const fmc_fxpt128_t &lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs);
  return r += rhs;
}

static inline fmc_fxpt128_t operator-(const fmc_fxpt128_t &lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs);
  return r -= rhs;
}

static inline fmc_fxpt128_t operator*(const fmc_fxpt128_t &lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs);
  return r *= rhs;
}

static inline fmc_fxpt128_t operator*(const int64_t lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r((int64_t)lhs);
  return r *= rhs;
}

static inline fmc_fxpt128_t operator*(const uint64_t lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs, 0);
  return r *= rhs;
}

static inline fmc_fxpt128_t operator*(const fmc_fxpt128_t &rhs,
                                      const uint64_t lhs) {
  fmc::fxpt128 r(lhs, 0);
  return r *= rhs;
}

static inline fmc_fxpt128_t operator/(const fmc_fxpt128_t &lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs);
  return r /= rhs;
}

static inline fmc_fxpt128_t operator/(const int32_t lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r((int64_t)lhs);
  return r /= rhs;
}

static inline fmc_fxpt128_t operator/(const int64_t lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r((int64_t)lhs);
  return r /= rhs;
}

static inline fmc_fxpt128_t operator/(const uint32_t lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r((uint64_t)lhs, 0);
  return r /= rhs;
}

static inline fmc_fxpt128_t operator/(const uint64_t lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r((uint64_t)lhs, 0);
  return r /= rhs;
}

static inline fmc_fxpt128_t operator%(const fmc_fxpt128_t &lhs,
                                      const fmc_fxpt128_t &rhs) {
  fmc::fxpt128 r(lhs);
  return r %= rhs;
}

static inline fmc_fxpt128_t operator<<(const fmc_fxpt128_t &lhs, int amount) {
  fmc::fxpt128 r(lhs);
  return r <<= amount;
}

static inline fmc_fxpt128_t operator>>(const fmc_fxpt128_t &lhs, int amount) {
  fmc::fxpt128 r(lhs);
  return r >>= amount;
}

static inline bool operator<(const fmc_fxpt128_t &lhs,
                             const fmc_fxpt128_t &rhs) {
  return fmc_fxpt128_cmp(&lhs, &rhs) < 0;
}

static inline bool operator>(const fmc_fxpt128_t &lhs,
                             const fmc_fxpt128_t &rhs) {
  return fmc_fxpt128_cmp(&lhs, &rhs) > 0;
}

static inline bool operator<=(const fmc_fxpt128_t &lhs,
                              const fmc_fxpt128_t &rhs) {
  return fmc_fxpt128_cmp(&lhs, &rhs) <= 0;
}

static inline bool operator>=(const fmc_fxpt128_t &lhs,
                              const fmc_fxpt128_t &rhs) {
  return fmc_fxpt128_cmp(&lhs, &rhs) >= 0;
}

static inline bool operator==(const fmc_fxpt128_t &lhs,
                              const fmc_fxpt128_t &rhs) {
  return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}

static inline bool operator!=(const fmc_fxpt128_t &lhs,
                              const fmc_fxpt128_t &rhs) {
  return lhs.lo != rhs.lo || lhs.hi != rhs.hi;
}

static inline fmc_fxpt128_t operator-(const fmc_fxpt128_t &lhs) {
  fmc_fxpt128_t r(lhs);
  fmc_fxpt128_neg(&r, &lhs);
  return r;
}

namespace std {
template <> struct numeric_limits<fmc::fxpt128> {
  static const bool is_specialized = true;

  static fmc::fxpt128 min() noexcept { return FXPT128_min; }
  static fmc::fxpt128 max() noexcept { return FXPT128_max; }

  static const int digits = 127;
  static const int digits10 = 38;
  static const bool is_signed = true;
  static const bool is_integer = false;
  static const bool is_exact = false;
  static const int radix = 2;
  static fmc::fxpt128 epsilon() noexcept { return FXPT128_smallest; }
  static fmc::fxpt128 round_error() noexcept { return FXPT128_one; }

  static const int min_exponent = 0;
  static const int min_exponent10 = 0;
  static const int max_exponent = 0;
  static const int max_exponent10 = 0;

  static const bool has_infinity = false;
  static const bool has_quiet_NaN = false;
  static const bool has_signaling_NaN = false;
  static const float_denorm_style has_denorm = denorm_absent;
  static const bool has_denorm_loss = false;

  static fmc::fxpt128 infinity() noexcept { return FXPT128_zero; }
  static fmc::fxpt128 quiet_NaN() noexcept { return FXPT128_zero; }
  static fmc::fxpt128 signaling_NaN() noexcept { return FXPT128_zero; }
  static fmc::fxpt128 denorm_min() noexcept { return FXPT128_zero; }

  static const bool is_iec559 = false;
  static const bool is_bounded = true;
  static const bool is_modulo = true;

  static const bool traps = numeric_limits<uint64_t>::traps;
  static const bool tinyness_before = false;
  static const float_round_style round_style = round_toward_zero;
};

inline ostream &operator<<(ostream &os, const fmc_fxpt128_t &r) {
  fmc::static_buffer<FMC_FXPT128_STR_SIZE> buf;
  os << static_cast<const fmc::fxpt128 &>(r).to_string_view(buf);
  return os;
}

inline string to_string(const fmc_fxpt128_t &r) {
  fmc::static_buffer<FMC_FXPT128_STR_SIZE> buf;
  return string(static_cast<const fmc::fxpt128 &>(r).to_string_view(buf));
}

inline istream &operator>>(istream &os, fmc_fxpt128_t &r) {
  string str;
  os >> str;
  auto res = fmc::fxpt128::from_string_view(str);
  fmc_runtime_error_unless(res.second == string_view(str))
      << "unable to build fixed point from string";
  fmc_fxpt128_copy(&r, &res.first);
  return os;
}

template <typename T>
inline typename std::enable_if_t<std::is_base_of_v<fmc_fxpt128_t, T>, bool>
isinf(T x) {
  return false;
}

template <typename T>
inline typename std::enable_if_t<std::is_base_of_v<fmc_fxpt128_t, T>, bool>
isfinite(T x) {
  return true;
}

template <typename T>
inline
    typename std::enable_if_t<std::is_base_of_v<fmc_fxpt128_t, T>, fmc::fxpt128>
    abs(T x) {
  fmc::fxpt128 res;
  fmc_fxpt128_abs(&res, &x);
  return res;
}

template <typename T>
inline
    typename std::enable_if_t<std::is_base_of_v<fmc_fxpt128_t, T>, fmc::fxpt128>
    pow(T x, uint64_t n) {
  fmc::fxpt128 res;
  // TODO: Implement
  return res;
}

template <typename T>
inline typename std::enable_if_t<std::is_base_of_v<fmc_fxpt128_t, T>, bool>
isnan(T x) {
  // TODO: Implement
  return false;
}

template <> struct hash<fmc::fxpt128> {
  size_t operator()(const fmc::fxpt128 &k) const noexcept {
    return fmc_hash_combine(std::hash<uint64_t>{}(k.hi),
                            std::hash<uint64_t>{}(k.lo));
  }
};

} // namespace std

namespace fmc {
template <> struct sided_initializer<fmc::fxpt128> {
  static constexpr bool is_specialized = true;
  static fmc::fxpt128 min() noexcept {
    return std::numeric_limits<fmc::fxpt128>::min();
  }
  static fmc::fxpt128 max() noexcept {
    return std::numeric_limits<fmc::fxpt128>::max();
  }
};

static inline fxpt128 safe_add(const fxpt128 &lhs, const fxpt128 &rhs) {
  auto max_bound = std::numeric_limits<fxpt128>::max() - max(rhs, fxpt128{});
  auto min_bound = std::numeric_limits<fxpt128>::min() - min(rhs, fxpt128{});
  return std::max(std::min(lhs, max_bound), min_bound) + rhs;
}

static inline fxpt128 safe_sub(const fxpt128 &lhs, const fxpt128 &rhs) {
  auto max_bound = std::numeric_limits<fxpt128>::max() + min(rhs, fxpt128{});
  auto min_bound = std::numeric_limits<fxpt128>::min() + max(rhs, fxpt128{});
  return std::max(std::min(lhs, max_bound), min_bound) - rhs;
}

} // namespace fmc
