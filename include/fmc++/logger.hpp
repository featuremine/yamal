/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file include/fmc++/logger.hpp
 * @date Aug 6 2021
 */

#pragma once

#include <fmc++/time.hpp>

namespace fmc {

struct logger_t {
  template <typename... Args> void info(Args &&...args) {
    out << std::chrono::system_clock::now().time_since_epoch();

    for_each(
        [&](auto &&arg) { out << ' ' << std::forward<decltype(arg)>(arg); },
        std::forward<Args>(args)...);

    out << std::endl;
  }

  std::ostream &out;
};

} // namespace fmc
