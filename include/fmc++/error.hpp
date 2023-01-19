/******************************************************************************

        COPYRIGHT (c) 2023 by Featuremine Corporation.
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
 * @file include/fmc++/error.hpp
 * @date Jan 18 2023
 */

#pragma once

#include "fmc/error.h"

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
