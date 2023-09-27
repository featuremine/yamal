/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file memory.hpp
 * @author Federico Ravchina
 * @date 21 Jun 2021
 * @brief File contains C++ declaration of memory API
 *
 * This file contains C++ declaration of memory API.
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>
#include <memory>
#include <stdlib.h>
#include <string_view>

namespace fmc {
namespace hidden {
struct autofree_destructor {
  inline void operator()(void *ptr) const { free(ptr); }
};
} // namespace hidden

template <typename T>
using autofree = std::unique_ptr<T, hidden::autofree_destructor>;

class buffer {
public:
  buffer() : data_(nullptr), sz_(0) {}
  buffer(void *data, size_t sz) : data_(data), sz_(sz) {}
  operator std::string_view() { return std::string_view((char *)data_, sz_); }
  void *data() { return data_; }
  size_t size() { return sz_; }

private:
  void *data_;
  size_t sz_;
};

} // namespace fmc
