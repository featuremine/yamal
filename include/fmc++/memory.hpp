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
