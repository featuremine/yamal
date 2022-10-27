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
