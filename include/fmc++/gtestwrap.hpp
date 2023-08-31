/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file gtestwrap.hpp
 * @author Maxim Trokhimtchouk
 * @date 11 Aug 2017
 * @brief Wrapper for clean compilation of gtest
 *
 * @see http://www.featuremine.com
 */

#pragma once

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include <gtest/gtest.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <fmc++/mpl.hpp>
#include <fmc/platform.h>
#include <fmc/test.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#if defined(FMC_SYS_UNIX)
#include <sys/wait.h>
#include <unistd.h>
#endif // FMC_SYS_UNIX
