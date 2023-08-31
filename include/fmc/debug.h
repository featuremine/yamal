/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file debug.h
 * @author Maxim Trokhimtchouk
 * @date 27 Apr 2019
 * @brief C++ utilities
 *
 * This file defines debug macro
 */

#pragma once

#include <signal.h>

#define FMC_BREAKPOINT raise(SIGTRAP)
