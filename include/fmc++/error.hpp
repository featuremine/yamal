/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file include/fmc++/error.hpp
 * @date Jan 18 2023
 */

#pragma once

#include "fmc/error.h"
#include "fmc++/logger.hpp"

#include <ostream>
#include <utility>

namespace fmc {
class error : public fmc_error {
public:
  error() noexcept { fmc_error_init_none(static_cast<fmc_error *>(this)); }
  error(std::nullptr_t) noexcept : error() {}
  error(const fmc_error &rhs) noexcept {
    fmc_error_init(static_cast<fmc_error *>(this), rhs.code, rhs.buf);
  }
  error(const error &rhs) noexcept
      : error(*static_cast<const fmc_error *>(&rhs)) {}
  error(error &&rhs) noexcept {
    fmc_error_init_mov(static_cast<fmc_error *>(this),
                       static_cast<fmc_error *>(&rhs));
  }
  error(const char *rhs) noexcept {
    fmc_error_init_sprintf(static_cast<fmc_error *>(this), "%s", rhs);
  }
  error(const std::string &rhs) noexcept : error(rhs.c_str()) {}
  ~error() { fmc_error_destroy(static_cast<fmc_error *>(this)); }
  error &operator=(const fmc_error &rhs) noexcept {
    fmc_error_cpy(static_cast<fmc_error *>(this), &rhs);
    return *this;
  }
  error &operator=(const error &rhs) noexcept {
    return this->operator=(*static_cast<const fmc_error *>(&rhs));
  }
  error &operator=(error &&rhs) noexcept {
    fmc_error_mov(static_cast<fmc_error *>(this),
                  static_cast<fmc_error *>(&rhs));
    return *this;
  }
  error &operator=(const char *err) noexcept {
    fmc_error_reset_sprintf(static_cast<fmc_error *>(this), "%s", err);
    return *this;
  }
  error &operator=(const std::string &err) noexcept {
    return this->operator=(err.c_str());
  }
  error &operator=(std::nullptr_t) noexcept {
    clear();
    return *this;
  }
  operator bool() const noexcept {
    return fmc_error_has(static_cast<const fmc_error *>(this));
  }
  void clear() noexcept {
    fmc_error_reset_none(static_cast<fmc_error *>(this));
  }
  template <typename... Args>
  void sprintf(const char *fmt, Args &&...args) noexcept {
    fmc_error_reset_sprintf(static_cast<fmc_error *>(this), fmt,
                            std::forward<Args>(args)...);
  }
  const char *c_str() const noexcept {
    return fmc_error_msg(static_cast<const fmc_error *>(this));
  }
};

} // namespace fmc

#define STR(a) #a
#define XSTR(a) STR(a)

#define RETURN_ERROR_UNLESS(COND, ERR, RET, ...)                               \
  if (__builtin_expect(!(COND), 0)) {                                          \
    std::ostringstream ss;                                                     \
    fmc::logger_t logger{ss};                                                  \
    logger.info(std::string_view("(" __FILE__ ":" XSTR(__LINE__) ")"),         \
                __VA_ARGS__);                                                  \
    fmc_error_set(ERR, "%s", ss.str().c_str());                                \
    return RET;                                                                \
  }

#define RETURN_ON_ERROR(ERR, RET, ...)                                         \
  if (__builtin_expect((*ERR) != nullptr, 0)) {                                \
    std::ostringstream ss;                                                     \
    fmc::logger_t logger{ss};                                                  \
    logger.write(__VA_ARGS__);                                                  \
    fmc_error_add(ERR, ";", "%s", ss.str().c_str());                          \
    return RET;                                                                \
  }

#define EXIT_ON_ERROR(ERR, ...)                                                \
  if (__builtin_expect((*ERR) != nullptr, 0)) {                                \
    fprintf(stderr, "%s\n", fmc_error_msg(error));                             \
    return 1;                                                                  \
  }

#define RETURN_ERROR(ERR, RET, ...)                                            \
  RETURN_ERROR_UNLESS(false, ERR, RET, __VA_ARGS__)

namespace std {

inline ostream &operator<<(ostream &os, const fmc_error &r) {
  os << fmc_error_msg(&r);
  return os;
}

inline string to_string(const fmc_error &r) { return fmc_error_msg(&r); }

inline istream &operator>>(istream &os, fmc_error &r) {
  string str;
  os >> str;
  fmc_error_init_sprintf(&r, "%s", str.c_str());
  return os;
}

} // namespace std
