/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once

#include <string>

#include <fmc++/misc.hpp>

#include "endianess.h"

template <typename... Args> static std::string buildmsg(Args... args) {
  std::string ret;
  fmc::for_each(fmc::overloaded{
                    [&](uint64_t arg) {
                      ret.resize(ret.size() + sizeof(uint64_t));
                      uint64_t val = htoye64(arg);
                      std::memcpy(ret.data() + ret.size() - sizeof(uint64_t),
                                  &val, sizeof(uint64_t));
                    },
                    [&](uint32_t arg) {
                      using T = decltype(arg);
                      ret.resize(ret.size() + sizeof(uint32_t));
                      T val = htoye32(arg);
                      std::memcpy(ret.data() + ret.size() - sizeof(uint32_t),
                                  &val, sizeof(uint32_t));
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
