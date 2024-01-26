/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file ordered_map.hpp
 * @author Andres Rangel
 * @date 25 Jan 2023
 * @brief Definition of the pool
 */

#pragma once

#include <cstddef>
#include <vector>

namespace fmc {

template <typename T>
class pool_t {
  public:
  pool_t(size_t init_sz, size_t inc_sz)
  : size_(inc_sz)
  {
    allocate(init_sz);
  }

  T * get() {
    if (curr_ == blocks_.back())
      allocate(size_);
    return --curr_;
  }

  ~pool_t() {
    for (auto head : blocks_)
      delete[] head;
  }

  private:
  void allocate(size_t sz) {
    curr_ = new T[sz]();
    blocks_.push_back(curr_);
    curr_ += sz;
  }

  std::vector<T *> blocks_;
  T* curr_ = nullptr;
  size_t size_ = 0;
};

} // namespace fmc
