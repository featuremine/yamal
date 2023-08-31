/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file time.hpp
 * @author Maxim Trokhimtchouk
 * @date 14 Oct 2017
 * @brief File contains C++ implementation of time
 *
 * This file describes platform time
 */

#pragma once

#include <fmc++/convert.hpp>
#include <fmc/platform.h>
#include <fmc/time.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>

namespace fmc {
class Int64 {
public:
  Int64() : value_() {}
  constexpr Int64(int64_t v) : value_(v) {}
  operator int64_t() const { return value_; }
  Int64 &operator=(const Int64 &i) {
    value_ = i.value_;
    return *this;
  }
  friend bool operator==(const Int64 &a, const Int64 &b);
  friend Int64 operator*(const Int64 &a, const Int64 &b);
  friend Int64 operator/(const Int64 &a, const Int64 &b);
  friend Int64 operator+(const Int64 &a, const Int64 &b);
  friend Int64 &operator+=(Int64 &a, const Int64 &b);
  friend Int64 operator-(const Int64 &a, const Int64 &b);

private:
  int64_t value_ = 0;
};
inline bool operator==(const Int64 &a, const Int64 &b) {
  return a.value_ == b.value_;
}
inline Int64 operator*(const Int64 &a, const Int64 &b) {
  return Int64(a.value_ * b.value_);
}
inline Int64 operator/(const Int64 &a, const Int64 &b) {
  return Int64(a.value_ / b.value_);
}
inline Int64 operator+(const Int64 &a, const Int64 &b) {
  return Int64(a.value_ + b.value_);
}
inline Int64 &operator+=(Int64 &a, const Int64 &b) {
  a.value_ += b.value_;
  return a;
}
inline Int64 operator-(const Int64 &a, const Int64 &b) {
  return Int64(a.value_ - b.value_);
}
} // namespace fmc

namespace std {

template <typename T>
struct common_type<T, enable_if_t<is_integral_v<T>, fmc::Int64>> {
  using type = T;
};
template <typename T>
struct common_type<enable_if_t<is_integral_v<T>, fmc::Int64>, T> {
  using type = T;
};

namespace chrono {
template <> struct duration_values<fmc::Int64> {
  static constexpr fmc::Int64 min() {
    return fmc::Int64(numeric_limits<int64_t>::min());
  }
  static constexpr fmc::Int64 zero() { return fmc::Int64(0); }
  static constexpr fmc::Int64 max() {
    return fmc::Int64(numeric_limits<int64_t>::max());
  }
};
} // namespace chrono
} // namespace std

namespace fmc {
using namespace std;

/**
 * @brief fmc platform time definition
 */
using time = std::chrono::duration<Int64, std::nano>;

} // namespace fmc

namespace std {

/**
 * @brief Time output stream overload.
 *
 * @param s Output stream.
 * @param x Platform time object
 *
 * @return Output stream after receiving formatted fmc time object.
 */
inline std::ostream &operator<<(std::ostream &s, const fmc::time &x) {
  using namespace std;
  using namespace chrono;
  auto epoch = time_point<system_clock>(
      duration_cast<time_point<system_clock>::duration>(x));
  auto t = system_clock::to_time_t(epoch);
  struct tm tm;
  fmc_gmtime(&t, &tm);
  return s << put_time(&tm, "%F %T") << '.' << setw(9) << setfill('0')
           << (x % seconds(1)).count();
}

inline std::string to_string(const fmc::time &x) {
  using namespace std;
  using namespace chrono;
  auto epoch = time_point<system_clock>(
      duration_cast<time_point<system_clock>::duration>(x));
  auto t = system_clock::to_time_t(epoch);
  struct tm tm;
  fmc_gmtime(&t, &tm);
  auto nns = to_string((x % seconds(1)).count()).substr(0, 9);
  char buff[100];
  auto n = strftime(buff, 100, "%F %T", &tm);
  if (n == 0) {
    return string();
  }
  return string(buff, n) + '.' + string(9 - nns.size(), '0') + nns;
}

} // namespace std

inline int64_t operator/(fmc_time64_t a, fmc_time64_t b) {
  return fmc_time64_div(a, b);
}

inline bool operator==(fmc_time64_t a, fmc_time64_t b) {
  return fmc_time64_equal(a, b);
}

inline bool operator!=(fmc_time64_t a, fmc_time64_t b) {
  return !fmc_time64_equal(a, b);
}

inline fmc_time64_t operator+(fmc_time64_t a, fmc_time64_t b) {
  return fmc_time64_add(a, b);
}

inline fmc_time64_t &operator+=(fmc_time64_t &a, const fmc_time64_t &b) {
  fmc_time64_inc(&a, b);
  return a;
}

inline fmc_time64_t operator-(fmc_time64_t a, fmc_time64_t b) {
  return fmc_time64_sub(a, b);
}

inline bool operator<(fmc_time64_t a, fmc_time64_t b) {
  return fmc_time64_less(a, b);
}

inline bool operator>(fmc_time64_t a, fmc_time64_t b) {
  return fmc_time64_less(b, a);
}

inline bool operator<=(fmc_time64_t a, fmc_time64_t b) {
  return !fmc_time64_less(b, a);
}

inline bool operator>=(fmc_time64_t a, fmc_time64_t b) {
  return !fmc_time64_less(a, b);
}

inline fmc_time64_t operator*(fmc_time64_t a, int64_t b) {
  return fmc_time64_mul(a, b);
}

inline fmc_time64_t operator*(int64_t a, fmc_time64_t b) {
  return fmc_time64_mul(b, a);
}

inline fmc_time64_t operator/(fmc_time64_t a, int64_t b) {
  return fmc_time64_int_div(a, b);
}

namespace std {
inline std::ostream &operator<<(std::ostream &s, const fmc_time64_t &x) {
  using namespace std;
  using namespace chrono;
  auto nanos = nanoseconds(fmc_time64_to_nanos(x));
  auto epoch = time_point<system_clock>(
      duration_cast<time_point<system_clock>::duration>(nanos));
  auto t = system_clock::to_time_t(epoch);
  auto tm = *gmtime(&t);
  return s << put_time(&tm, "%F %T") << '.' << setw(9) << setfill('0')
           << (nanos % seconds(1)).count();
}
inline istream &operator>>(istream &s, fmc_time64_t &x) {
  using namespace std;
  using namespace chrono;
  std::tm t = {};
  unsigned nanos;
  s >> get_time(&t, "%Y-%m-%d %H:%M:%S.") >> setw(9) >> nanos;
  auto epoch_sec = system_clock::from_time_t(timegm(&t)).time_since_epoch();
  auto dur = duration_cast<nanoseconds>(epoch_sec) + nanoseconds(nanos);
  x = fmc_time64_from_nanos(dur.count());
  return s;
}

/**
 * @brief Smaller than operator overload for fmc time and fmc_time64_t
 * objects
 *
 * @param jt Platform time object.
 * @param et fmc_time64_t object.
 *
 * @return result of comparison.
 */
inline bool operator<(const fmc::time &jt, const fmc_time64_t &et) {
  return jt < std::chrono::nanoseconds(fmc_time64_to_nanos(et));
}

/**
 * @brief Smaller than operator overload for fmc_time64_t and fmc time
 * objects
 *
 * @param et fmc_time64_t object.
 * @param jt Platform time object.
 *
 * @return Result of comparison.
 */
inline bool operator<(const fmc_time64_t &et, const fmc::time &jt) {
  return std::chrono::nanoseconds(fmc_time64_to_nanos(et)) < jt;
}

/**
 * @brief Greater than operator overload for fmc time and fmc_time64_t
 * objects
 *
 * @param jt Platform time object.
 * @param et fmc_time64_t object.
 *
 * @return Result of comparison.
 */
inline bool operator>(const fmc::time &jt, const fmc_time64_t &et) {
  return jt > std::chrono::nanoseconds(fmc_time64_to_nanos(et));
}

/**
 * @brief Greater than operator overload for fmc_time64_t and fmc time
 * objects
 *
 * @param jt Platform time object.
 * @param et fmc_time64_t object.
 *
 * @return Result of comparison.
 */
inline bool operator>(const fmc_time64_t &et, const fmc::time &jt) {
  return std::chrono::nanoseconds(fmc_time64_to_nanos(et)) > jt;
}

} // namespace std

namespace fmc {
template <> struct conversion<fmc_time64_t, fmc::time> {
  fmc::time operator()(fmc_time64_t x) {
    return std::chrono::nanoseconds(fmc_time64_to_nanos(x));
  }
};

template <> struct conversion<fmc::time, fmc_time64_t> {
  fmc_time64_t operator()(fmc::time x) {
    return fmc_time64_from_nanos(std::chrono::nanoseconds(x).count());
  }
};
} // namespace fmc
