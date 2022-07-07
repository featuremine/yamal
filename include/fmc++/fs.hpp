/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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

#if !defined(FMC_SYS_WIN) || (defined(__GNUC__) && __GNUC__ < 8)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
