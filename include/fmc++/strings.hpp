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
 * @file strings.cpp
 * @author Leandro Rabindranath Leon
 * @date 9 July 2018
 * @brief File contains string utilities
 *
 * This file contains C++ utilities to work with strings
 * @see http://www.featuremine.com
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <limits>
#include <math.h>
#include <stdint.h>
#include <string>
#include <utility>

#include <fmc/platform.h>

namespace fmc {

// Remove starting trailing blanks
inline void ltrim(std::string &s) {
  s.erase(s.begin(),
          find_if(s.begin(), s.end(), [](int c) { return !isspace(c); }));
}

// Remove ending trailing blanks
inline void rtrim(std::string &s) {
  s.erase(
      find_if(s.rbegin(), s.rend(), [](int c) { return !isspace(c); }).base(),
      s.end());
}

// Remove starting and ending blanks
inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

// Return true is s ends with suffix.
// This function emulates equivalent in C++20 (that will be a method
// of string class)
inline bool ends_with(const std::string &s, const std::string &suffix) {
  return s.size() >= suffix.size() &&
         0 == s.compare(s.size() - suffix.size(), suffix.size(), suffix);
}

// Return true is s starts with prefix.
// This function emulates equivalent in C++20 (that will be a method
// of string class)
inline bool starts_with(const std::string &s, const std::string &prefix) {
  return s.size() >= prefix.size() && 0 == s.compare(0, prefix.size(), prefix);
}

// Determines if name is a file name or a command to be executed and
// whose output is the input of csv_play. The result is put in first.
//
// It is a command if name is of form: \s*command\s*|\s*
//
// If the command is detected, then second contain the command to be
// executed (without trailing blanks and '|'). Otherwise, second is a
// file name.
inline std::pair<bool, std::string> ends_with_pipe(std::string name) {
  trim(name); // removes all trailing blanks
  if (ends_with(name, "|")) {
    return {true, name.erase(name.size() - 1, 1)}; // remove '|'
  }
  return {false, move(name)};
}

inline std::pair<bool, std::string> begins_with_pipe(std::string name) {
  trim(name); // removes all trailing blanks
  if (starts_with(name, "|")) {
    return {true, name.erase(0, 1)}; // remove '|'
  }
  return {false, move(name)};
}

template <class T>
std::pair<T, std::string_view> from_string_view(std::string_view view);

template <class T>
inline std::pair<T, std::string_view>
_from_string_view_unsigned(std::string_view view) {
  size_t size = 0;
  T result = 0;
  const auto bound = std::numeric_limits<T>::max();
  const auto bound_div_10 = std::numeric_limits<T>::max() / 10;
  for (; size < view.size(); ++size) {
    T digit = view[size] - static_cast<T>('0');
    if ((view[size] < '0') | (digit > 9) | (bound_div_10 < result)) {
      break;
    }
    auto next = result * 10;
    if (bound - next < digit) {
      break;
    }
    result = next + digit;
  }
  return {result, view.substr(0, size)};
}

template <class T>
inline std::pair<T, std::string_view>
_from_string_view_signed(std::string_view view) {
  if (view.size() > 0 && view[0] == '-') {
    if (view.size() == 1) {
      return {0, view.substr(0, 0)};
    }
    size_t size = 1;
    T result = 0;
    const auto bound = std::numeric_limits<T>::min();
    const auto bound_div_10 = std::numeric_limits<T>::min() / 10;
    for (; size < view.size(); ++size) {
      T digit = view[size] - static_cast<T>('0');
      if ((view[size] < '0') | (digit > 9) | (bound_div_10 > result)) {
        break;
      }
      auto next = result * 10;
      if (bound - next > -digit) {
        break;
      }
      result = next - digit;
    }
    return {result, view.substr(0, size)};
  } else {
    return fmc::_from_string_view_unsigned<T>(view);
  }
}

std::pair<double, std::string_view>
_from_string_view_double(const std::string_view &s) noexcept;

template <>
inline std::pair<double, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_double(view);
}

template <>
inline std::pair<uint8_t, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_unsigned<uint8_t>(view);
}

template <>
inline std::pair<uint16_t, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_unsigned<uint16_t>(view);
}

template <>
inline std::pair<uint32_t, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_unsigned<uint32_t>(view);
}

template <>
inline std::pair<uint64_t, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_signed<uint64_t>(view);
}

template <>
inline std::pair<int8_t, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_signed<int8_t>(view);
}

template <>
inline std::pair<int16_t, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_signed<int16_t>(view);
}

template <>
inline std::pair<int32_t, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_signed<int32_t>(view);
}

template <>
inline std::pair<int64_t, std::string_view>
from_string_view(std::string_view view) {
  return fmc::_from_string_view_signed<int64_t>(view);
}

template <class T>
inline std::string_view
to_string_view_unsigned_fixed_length(std::string_view view, T value) {
  char *start_num_ptr = (char *)&view[0];
  memset(start_num_ptr, '0', view.size());
  auto ptr = start_num_ptr + view.size() - 1;
  auto j = value;
  for (; j != 0 && ptr >= start_num_ptr; --ptr) {
    *ptr = j % 10 + '0';
    j = j / 10;
  }
  return view;
}

template <class T>
inline std::string_view
to_string_view_signed_fixed_length(std::string_view view, T value) {
  if (value >= 0) {
    return to_string_view_unsigned_fixed_length(view, value);
  }

  *((char *)&view[0]) = '-';
  to_string_view_unsigned_fixed_length(
      std::string_view(&view[1], view.size() - 1), value * -1);

  return view;
}

template <class T>
inline std::string_view to_string_view_unsigned(char *buf, T value) {
  const unsigned MAX_LEN = 20;
  char tmp[MAX_LEN] = {'0'};
  auto ptr = &tmp[MAX_LEN];
  auto j = value;
  do {
    *(--ptr) = j % 10 + '0';
    j = j / 10;
  } while (j != 0);
  auto sz = unsigned(&tmp[MAX_LEN] - ptr);
  memcpy(buf, ptr, sz);
  return std::string_view(buf, sz);
}

template <class T>
inline std::string_view to_string_view_signed(char *buf, T value) {
  if (value >= 0) {
    return to_string_view_unsigned(buf, value);
  }
  *buf = '-';
  std::string_view view = to_string_view_unsigned(buf + 1, -1 * value);
  return std::string_view(buf, view.size() + 1);
}

inline std::string_view to_string_view_double_unsigned(char *buf, double value,
                                                       size_t precision) {
  if (isnan(value)) {
    std::memcpy(buf, "nan", 3);
    return std::string_view(buf, 3);
  }

  if (isinf(value)) {
    std::memcpy(buf, "inf", 3);
    return std::string_view(buf, 3);
  }

  int64_t factor = int64_t(pow(10, precision));
  int64_t very_big_int = int64_t(round(value * factor));

  const unsigned MAX_LEN = 21;
  char tmp[MAX_LEN] = {'0'};
  auto ptr = &tmp[MAX_LEN];
  auto j = very_big_int;
  int64_t count = precision;
  auto trailing = true;
  do {
    if (!count--) {
      *(--ptr) = '.';
      continue;
    }
    auto digit = j % 10;
    if (digit || !trailing || count == 0) {
      *(--ptr) = digit + '0';
      trailing = false;
    }
    j = j / 10;
  } while (j != 0 || count >= -1);
  auto sz = unsigned(&tmp[MAX_LEN] - ptr);
  memcpy(buf, ptr, sz);
  return std::string_view(buf, sz);
}

inline std::string_view to_string_view_double(char *buf, double value,
                                              size_t precision) {
  if (value >= 0.0) {
    return to_string_view_double_unsigned(buf, value, precision);
  }
  *buf = '-';
  std::string_view view =
      to_string_view_double_unsigned(buf + 1, -1.0 * value, precision);
  return std::string_view(buf, view.size() + 1);
}

inline std::pair<double, std::string_view>
_from_string_view_double(const std::string_view &s) noexcept {
  if (s.empty()) {
    return {0.0, s.substr(0, 0)};
  }
  constexpr unsigned n = 18; // Max size numerical value
  bool negative = s[0] == '-';
  const auto se = negative ? std::string_view(s.data() + 1, s.size() - 1) : s;
  unsigned long long sum = 0;
  unsigned point_pos = 1; // > 1 indicates that point has been seen
  unsigned i = 0;         // num of characters (including period)
  unsigned long long digit_count = 1;
  for (auto it = se.rbegin(), e = se.rend(); it != e; ++it, ++i) {
    if (i + 1 > n) {
      return {0.0, s.substr(0, 0)};
    }
    const unsigned d = *it;
    if (!::isdigit(d) && d != '.') {
      return {0.0, s.substr(0, 0)};
    }

    if (d == '.') {
      if (point_pos > 1) { // has it already been seen a point?
        return {0.0, s.substr(0, 0)};
      }
      point_pos = digit_count;
      if (point_pos == 1) {
        return {0.0, s.substr(0, 0)};
      }
      continue;
    }
    sum += (d - '0') * digit_count;
    digit_count =
        digit_count * 10; // only increased if a digit was seen (not the period)
  }

  if (negative) {
    return {-double(sum) / point_pos, s};
  }

  return {double(sum) / point_pos, s};
}

} // namespace fmc
