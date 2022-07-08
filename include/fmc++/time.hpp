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
