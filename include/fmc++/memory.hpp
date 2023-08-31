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

namespace fmc {
namespace hidden {
struct autofree_destructor {
  inline void operator()(void *ptr) const { free(ptr); }
};
} // namespace hidden

template <typename T>
using autofree = std::unique_ptr<T, hidden::autofree_destructor>;

} // namespace fmc
