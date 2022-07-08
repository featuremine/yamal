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
 * @file convert.hpp
 * @author Maxim Trokhimtchouk
 * @date 4 Oct 2017
 * @brief File contains C++ definition of conversion interface
 *
 * This file describes conversion interface
 */

#pragma once

namespace fmc {

template <class From, class To> struct conversion {
  To operator()(From from) { return static_cast<To>(from); }
};

template <class To, class From> To to(From from) {
  return conversion<From, To>()(from);
}

} // namespace fmc
