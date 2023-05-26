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

#pragma once

#include <string>

#include <fmc++/misc.hpp>

#include "yamal.hpp"

template <typename... Args> static std::string buildmsg(Args... args) {
  std::string ret;
  fmc::for_each(fmc::overloaded{
                    [&](uint64_t arg) {
                      ret.resize(ret.size() + sizeof(uint64_t));
                      uint64_t val = htoye64(arg);
                      std::memcpy(ret.data() + ret.size() - sizeof(uint64_t),
                                  &val, sizeof(uint64_t));
                    },
                    [&](uint16_t arg) {
                      using T = decltype(arg);
                      ret.resize(ret.size() + sizeof(uint16_t));
                      T val = htoye16(arg);
                      std::memcpy(ret.data() + ret.size() - sizeof(uint16_t),
                                  &val, sizeof(uint16_t));
                    },
                    [&](std::string_view arg) {
                      ret.resize(ret.size() + arg.size());
                      std::memcpy(ret.data() + ret.size() - arg.size(),
                                  arg.data(), arg.size());
                    },
                },
                args...);
  return ret;
}
