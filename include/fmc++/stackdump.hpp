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
 * @file stackdump.hpp
 * @author Maxim Trokhimtchouk
 * @date 18 Dec 2017
 * @brief Stack dump utilities
 *
 * File contains stackdump declaration
 */

#pragma once

#include <iostream>

/**
 * @brief stack dump
 */
std::ostream &stack_dump(std::ostream &is, int skip = 0);
