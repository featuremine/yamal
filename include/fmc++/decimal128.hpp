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
 * @file include/fmc++/decimal128.hpp
 * @date Oct 18 2022
 */

#pragma once

#include "fmc/decimal128.h"

#include <limits>

namespace fmc {

class decimal128 : private fmc_decimal128_t {
public:
    decimal128(int64_t i) {}
    decimal128(const fmc_decimal128_t &a) : fmc_decimal128_t(a) {}
    decimal128 &operator=(const fmc_decimal128_t &a) {
        return *this;
    }
    // Warning, conversion to base class will never be used, review:
    // https://eel.is/c++draft/class.conv.fct#4
    // operator fmc_decimal128_t &() {
    //     return static_cast<fmc_decimal128_t &>(*this);
    // }
    // operator const fmc_decimal128_t &() const {
    //     return static_cast<const fmc_decimal128_t &>(*this);
    // }
    static decimal128 &upcast(fmc_decimal128_t &a) {
        return static_cast<decimal128 &>(a);
    }
    static const decimal128 &upcast(const fmc_decimal128_t &a) {
        return static_cast<const decimal128 &>(a);
    }
    decimal128 &operator+=(const decimal128 &a) {
        return *this;
    }
};

decimal128 operator+(const decimal128 &a, const decimal128 &b);

} // namespace fmc

namespace std {
// limits and stream
} // namespace std