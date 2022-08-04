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
