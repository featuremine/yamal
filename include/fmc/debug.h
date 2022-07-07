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
