/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file fs.hpp
 * @author Federico Ravchina
 * @date 10 May 2021
 * @brief File contains C++ declaration of fs API
 *
 * This file contains C++ declaration of fs API.
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>

#if defined(FMC_SYS_LINUX) && defined(__GNUC__) && __GNUC__ < 8
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
