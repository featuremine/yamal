/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file config.cpp
 * @author Maxim Trokhimtchouk
 * @date 9 Oct 2017
 * @brief File contains C++ definition of config query interface
 *
 * This file describes config query interface
 */

#include <fmc++/config/config.hpp>
#include <fmc++/mpl.hpp> // fmc::typify

#include <ostream>     // std::ostream
#include <string>      // std::string
#include <string_view> // std::string_view

namespace fmc {
namespace configs {
namespace interface {

// *** node ***

array &node::get(typify<array>) { return get(typify<array &>()); }

section &node::get(typify<section>) { return get(typify<section &>()); }

std::string node::to_s() { return get(typify<std::string>()); }

array &node::to_a() { return get(typify<array &>()); }

section &node::to_d() { return get(typify<section &>()); }

node &node::operator[](std::string_view key) { return as<section>()[key]; }

std::ostream &operator<<(std::ostream &s, const node &n) {
  s << n.str();
  return s;
}

// *** section ***

node &section::operator[](std::string_view key) { return get(key); }

// *** array ***

node &array::operator[](size_t key) { return get(key); }

bool node::has(std::string_view key) {
  try {
    auto &s = to_d();
    return s.has(key);
  } catch (const std::exception &e) {
    // ignore exception, unable to convert to section
  }
  return false;
}

} // namespace interface
} // namespace configs
} // namespace fmc
