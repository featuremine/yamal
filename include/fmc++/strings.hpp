/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
#include <fmc/fxpt128.h>

namespace fmc {

// Remove starting trailing blanks
inline std::string_view ltrim(std::string_view s) {
  return s.substr(s.find_first_not_of("\t\n\v\f\r "));
}

// Remove ending trailing blanks
inline std::string_view rtrim(std::string_view s) {
  return s.substr(0, 1 + std::min(s.size(), s.find_last_not_of("\t\n\v\f\r ")));
}

// Remove starting and ending blanks
inline std::string_view trim(std::string_view s) { return ltrim(rtrim(s)); }

inline std::tuple<std::string_view, std::string_view, std::string_view>
split(std::string_view a, std::string_view sep) {
  auto pos = a.find_first_of(sep);
  if (pos >= a.size())
    return {a, std::string_view(), std::string_view()};
  return {a.substr(0, pos), a.substr(pos, 1), a.substr(pos + 1)};
}

inline bool ends_with(std::string_view a, std::string_view b) {
  return a.size() >= b.size() && a.substr(a.size() - b.size()) == b;
}

inline bool starts_with(std::string_view a, std::string_view b) {
  return a.substr(0, b.size()) == b;
}

// Determines if name is a file name or a command to be executed and
// whose output is the input of csv_play. The result is put in first.
//
// It is a command if name is of form: \s*command\s*|\s*
//
// If the command is detected, then second contain the command to be
// executed (without trailing blanks and '|'). Otherwise, second is a
// file name.
inline std::pair<bool, std::string_view> ends_with_pipe(std::string_view name) {
  if (auto sv = trim(name); ends_with(sv, "|")) {
    return {true, sv.substr(0, sv.size() - 1)}; // remove '|'
  }
  return {false, name};
}

inline std::pair<bool, std::string_view>
begins_with_pipe(std::string_view name) {
  if (auto sv = trim(name); starts_with(sv, "|")) {
    return {true, sv.substr(1)}; // remove '|'
  }
  return {false, name};
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

inline std::string_view to_string_view_double(char *buf, double value,
                                              int precision) {
  if (!isfinite(value)) {
    if (!isinf(value)) {
      constexpr std::string_view rep{"nan"};
      memcpy(buf, rep.data(), rep.size());
      return std::string_view{buf, rep.size()};
    }
    if (value > 0.0) {
      constexpr std::string_view rep{"inf"};
      memcpy(buf, rep.data(), rep.size());
      return std::string_view{buf, rep.size()};
    } else {
      constexpr std::string_view rep{"-inf"};
      memcpy(buf, rep.data(), rep.size());
      return std::string_view{buf, rep.size()};
    }
  }

  fmc_fxpt128_t x;
  fmc_fxpt128_from_double(&x, value);
  struct fmc_fxpt128_format_t format = {.precision = (int)precision};
  auto sz = fmc_fxpt128_to_string_opt(buf, FMC_FXPT128_STR_SIZE, &x, &format);
  if (sz == 2 && ((buf[0] == '-') & (buf[1] == '0')) ) {
    sz = 1;
    buf[0] = '0';
    buf[1] = '\0';
  }
  return std::string_view(buf, sz);
}

inline std::pair<double, std::string_view>
_from_string_view_double(const std::string_view &s) noexcept {
  if (s.empty())
    return {0.0, std::string_view{}};
  constexpr std::string_view::size_type LEN = 31;
  char buf[LEN + 1] = {};
  memcpy(buf, s.data(), std::min(s.size(), LEN));
  char *endptr = {};
  double res = strtod(buf, &endptr);
  return {res, std::string_view{s.data(), uint64_t(endptr - buf)}};
}

} // namespace fmc
