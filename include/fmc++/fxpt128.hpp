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
#include "fmc++/mpl.hpp"
#include "fmc++/rational64.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/side.hpp"
#include "fmc++/memory.hpp"

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
   fxpt128(double);
   fxpt128(int);
   fxpt128(FXPT128_S64);
   fxpt128(FXPT128_U64 low, FXPT128_U64 high);

   static std::pair<fxpt128, std::string_view> from_string_view(fmc::string_view buf);
   std::string_view to_string_view(fmc::buffer buf, const fmc_fxpt128_format_t *format = &FXPT128_default_format) const;

   constexpr fxpt128 &upcast(fmc_fxpt128_t &a) noexcept;
   constexpr const fxpt128 &upcast(const fmc_fxpt128_t &a) noexcept;

   operator double() const;
   operator FXPT128_S64() const;
   operator int() const;
   operator bool() const;

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

inline fxpt128::fxpt128(const fmc_fxpt128_t &c)
: fmc_fxpt128_t{c}
{
}

inline fxpt128::fxpt128(double v)
{
   fmc_fxpt128_from_double(this, v);
}

inline fxpt128::fxpt128(int v)
{
   fmc_fxpt128_from_int(this, v);
}

inline fxpt128::fxpt128(FXPT128_S64 v)
{
   fmc_fxpt128_from_int(this, v);
}

inline fxpt128::fxpt128(FXPT128_U64 low, FXPT128_U64 high)
{
   lo = low;
   hi = high;
}

inline std::pair<fxpt128, std::string_view> fxpt128::from_string_view(std::string_view buf)
{
   fxpt128 res;
   const char *endptr = buf.data() + buf.size();
   fmc_fxpt128_from_string(&res, buf.data(), &endptr);
   return make_pair(res, std::string_view(buf.data(), endptr - buf.data()));
}

inline std::string_view fxpt128::to_string_view(fmc::buffer buf, const fmc_fxpt128_format_t *format) const
{
   return std::string_view(buf.data(), fmc_fxpt128_to_string_opt(buf.data(), buf.size() - 1, this, format));
}

inline constexpr fxpt128 &fxpt128::upcast(fmc_fxpt128_t &a) noexcept {
   return static_cast<fxpt128 &>(a);
}

inline constexpr const fxpt128 &fxpt128::upcast(const fmc_fxpt128_t &a) noexcept {
   return static_cast<const fxpt128 &>(a);
}

inline fxpt128::operator double() const
{
   return fmc_fxpt128_to_double(this);
}

inline fxpt128::operator FXPT128_S64() const
{
   return fmc_fxpt128_to_int(this);
}

inline fxpt128::operator int() const
{
   return (int) fmc_fxpt128_to_int(this);
}

inline fxpt128::operator bool() const
{
   return lo || hi;
}

inline bool fxpt128::operator!() const
{
   return !lo && !hi;
}

inline fxpt128 fxpt128::operator~() const
{
   fxpt128 r;
   fmc_fxpt128_not(&r, this);
   return r;
}

inline fxpt128 fxpt128::operator-() const
{
   fxpt128 r;
   fmc_fxpt128_neg(&r, this);
   return r;
}

inline fxpt128 &fxpt128::operator|=(const fxpt128 &rhs)
{
   fmc_fxpt128_or(this, this, &rhs);
   return *this;
}

inline fxpt128 &fxpt128::operator&=(const fxpt128 &rhs)
{
   fmc_fxpt128_and(this, this, &rhs);
   return *this;
}

inline fxpt128 &fxpt128::operator^=(const fxpt128 &rhs)
{
   fmc_fxpt128_xor(this, this, &rhs);
   return *this;
}

inline fxpt128 &fxpt128::operator+=(const fxpt128 &rhs)
{
   fmc_fxpt128_add(this, this, &rhs);
   return *this;
}

inline fxpt128 &fxpt128::operator-=(const fxpt128 &rhs)
{
   fmc_fxpt128_sub(this, this, &rhs);
   return *this;
}

inline fxpt128 &fxpt128::operator*=(const fxpt128 &rhs)
{
   fmc_fxpt128_mul(this, this, &rhs);
   return *this;
}

inline fxpt128 &fxpt128::operator/=(const fxpt128 &rhs)
{
   fmc_fxpt128_div(this, this, &rhs);
   return *this;
}

inline fxpt128 &fxpt128::operator%=(const fxpt128 &rhs)
{
   fmc_fxpt128_mod(this, this, &rhs);
   return *this;
}

inline fxpt128 &fxpt128::operator<<=(int amount)
{
   fmc_fxpt128_shl(this, this, amount);
   return *this;
}

inline fxpt128 &fxpt128::operator>>=(int amount)
{
   fmc_fxpt128_sar(this, this, amount);
   return *this;
}

static inline fxpt128 operator|(const fxpt128 &lhs, const fxpt128 &rhs)
{
   fxpt128 r(lhs);
   return r |= rhs;
}

static inline fxpt128 operator&(const fxpt128 &lhs, const fxpt128 &rhs)
{
   fxpt128 r(lhs);
   return r &= rhs;
}

static inline fxpt128 operator^(const fxpt128 &lhs, const fxpt128 &rhs)
{
   fxpt128 r(lhs);
   return r ^= rhs;
}

static inline fxpt128 operator+(const fxpt128 &lhs, const fxpt128 &rhs)
{
   fxpt128 r(lhs);
   return r += rhs;
}

static inline fxpt128 operator-(const fxpt128 &lhs, const fxpt128 &rhs)
{
   fxpt128 r(lhs);
   return r -= rhs;
}

static inline fxpt128 operator*(const fxpt128 &lhs, const fxpt128 &rhs)
{
   fxpt128 r(lhs);
   return r *= rhs;
}

static inline fxpt128 operator/(const fxpt128 &lhs, const fxpt128 &rhs)
{
   fxpt128 r(lhs);
   return r /= rhs;
}

static inline fxpt128 operator%(const fxpt128 &lhs, const fxpt128 &rhs)
{
   fxpt128 r(lhs);
   return r %= rhs;
}

static inline fxpt128 operator<<(const fxpt128 &lhs, int amount)
{
   fxpt128 r(lhs);
   return r <<= amount;
}

static inline fxpt128 operator>>(const fxpt128 &lhs, int amount)
{
   fxpt128 r(lhs);
   return r >>= amount;
}

static inline bool operator<(const fxpt128 &lhs, const fxpt128 &rhs)
{
   return fmc_fxpt128_cmp(&lhs, &rhs) < 0;
}

static inline bool operator>(const fxpt128 &lhs, const fxpt128 &rhs)
{
   return fmc_fxpt128_cmp(&lhs, &rhs) > 0;
}

static inline bool operator<=(const fxpt128 &lhs, const fxpt128 &rhs)
{
   return fmc_fxpt128_cmp(&lhs, &rhs) <= 0;
}

static inline bool operator>=(const fxpt128 &lhs, const fxpt128 &rhs)
{
   return fmc_fxpt128_cmp(&lhs, &rhs) >= 0;
}

static inline bool operator==(const fxpt128 &lhs, const fxpt128 &rhs)
{
   return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}

static inline bool operator!=(const fxpt128 &lhs, const fxpt128 &rhs)
{
   return lhs.lo != rhs.lo || lhs.hi != rhs.hi;
}

} // namespace fmc


namespace std {
template<>
struct numeric_limits<fmc::fxpt128>
{
   static const bool is_specialized = true;

   static fmc::fxpt128 min() throw() { return FXPT128_min; }
   static fmc::fxpt128 max() throw() { return FXPT128_max; }

   static const int digits = 127;
   static const int digits10 = 38;
   static const bool is_signed = true;
   static const bool is_integer = false;
   static const bool is_exact = false;
   static const int radix = 2;
   static fmc::fxpt128 epsilon() throw() { return FXPT128_smallest; }
   static fmc::fxpt128 round_error() throw() { return FXPT128_one; }

   static const int min_exponent = 0;
   static const int min_exponent10 = 0;
   static const int max_exponent = 0;
   static const int max_exponent10 = 0;

   static const bool has_infinity = false;
   static const bool has_quiet_NaN = false;
   static const bool has_signaling_NaN = false;
   static const float_denorm_style has_denorm = denorm_absent;
   static const bool has_denorm_loss = false;

   static fmc::fxpt128 infinity() throw() { return FXPT128_zero; }
   static fmc::fxpt128 quiet_NaN() throw() { return FXPT128_zero; }
   static fmc::fxpt128 signaling_NaN() throw() { return FXPT128_zero; }
   static fmc::fxpt128 denorm_min() throw() { return FXPT128_zero; }

   static const bool is_iec559 = false;
   static const bool is_bounded = true;
   static const bool is_modulo = true;

   static const bool traps = numeric_limits<FXPT128_U64>::traps;
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
inline typename std::enable_if_t<std::is_base_of_v<fmc_fxpt128_t, T>,
                                 fmc::fxpt128>
abs(T x) {
  fmc::fxpt128 res;
  fmc_fxpt128_abs(&res, &x);
  return res;
}

template <> struct hash<fmc::fxpt128> {
  size_t operator()(const fmc::fxpt128 &k) const noexcept {
    return fmc_hash_combine(std::hash<uint64_t>{}(*(uint64_t *)&k.hi),
                            std::hash<uint64_t>{}(*(uint64_t *)&k.lo));
  }
};

}  //namespace std
