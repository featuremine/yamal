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
 * @file decimal.hpp
 * @brief Decimal C++ utilities
 * @see http://www.featuremine.com
 */

#pragma once

#include "fmc/decimal.h"

#include <ostream>

namespace std {
inline ostream &operator<<(ostream &s, const _Decimal128 &x) {
  return s;
}
inline istream &operator>>(istream &s, const _Decimal128 &x) {
  return s;
}
}