/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file convert.hpp
 * @author Maxim Trokhimtchouk
 * @date 4 Oct 2017
 * @brief File contains C++ definition of conversion interface
 *
 * This file describes conversion interface
 */

#pragma once

namespace fmc {

template <class From, class To> struct conversion {
  To operator()(From from) { return static_cast<To>(from); }
};

template <class To, class From> To to(From from) {
  return conversion<From, To>()(from);
}

} // namespace fmc
