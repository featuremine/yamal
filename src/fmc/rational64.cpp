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
 * @file rational.cpp
 * @author Maxim Trokhimtchouk
 * @date 28 Dec 2018
 * @brief File contains C definitions of the rational object
 *
 * This file contains declarations of the rational object
 * @see http://www.featuremine.com
 */

#include "fmc/rational64.h"
#include "fmc/rprice.h"

#include <numeric>
#include <fenv.h>

void fmc_rational64_zero(fmc_rational64_t *dest) {
    dest->num = 0;
    dest->den = 1;
}

void fmc_rational64_new(fmc_rational64_t *dest, int32_t num, int32_t den) {
  auto mult = -2 * (den < 0) + 1;
  den *= mult;
  num *= mult;
  auto div = std::gcd(num, den);
  if (div) {
    dest->num = num / div;
    dest->den = den / div;
  } else {
    dest->num = 0;
    dest->den = 0;
  }
}

void fmc_rational64_new2(fmc_rational64_t *dest, int64_t num, int64_t den) {
  auto mult = -2 * (den < 0) + 1;
  den *= mult;
  num *= mult;
  auto div = std::gcd(num, den);

  if (!div) {
    dest->num = 0;
    dest->den = 0;
    return;
  }

  auto num_n = num / div;
  auto den_n = den / div;
  if (num_n > std::numeric_limits<int32_t>::max() ||
      num_n < std::numeric_limits<int32_t>::lowest() ||
      den_n > std::numeric_limits<int32_t>::max()) {
    num_n = 0;
    den_n = 0;
    feraiseexcept(FE_OVERFLOW);
  }
  dest->num = int32_t(num_n);
  dest->den = int32_t(den_n);
}

void fmc_rational64_from_double(fmc_rational64_t*dest, double value, int32_t base) {
  dest->num = (int32_t)lround(floor(value * double(base)));
  dest->den = base;
}

void fmc_rational64_from_rprice(fmc_rational64_t*dest, fmc_rprice_t *src) {
  return fmc_rational64_new2(dest, src->value, FMC_RPRICE_FRACTION);
}

void fmc_rational64_from_int(fmc_rational64_t *dest, int value) {
  dest->num = value;
  dest->den = 1;
}

void fmc_rational64_to_double(double *dest, const fmc_rational64_t *src) {
  if (fmc_rational64_is_nan(src)) {
    *dest = std::numeric_limits<double>::quiet_NaN();
  } else {
    if (fmc_rational64_is_inf(src)) {
      *dest = src->num * std::numeric_limits<double>::infinity();
    } else {
      *dest = double(src->num) / double(src->den);
    }
  }
}

void fmc_rational64_to_rprice(fmc_rprice_t *dest, fmc_rational64_t *src) {
  fmc_rprice_from_ratio(dest, int64_t(src->num), int64_t(src->den));
}

void fmc_rational64_div(fmc_rational64_t *dest, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs) {
  auto num = int64_t(lhs->num) * int64_t(rhs->den);
  auto den = int64_t(lhs->den) * int64_t(rhs->num);
  fmc_rational64_new(dest, num, den);
}

void fmc_rational64_mul(fmc_rational64_t *dest, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs) {
  auto num = int64_t(lhs->num) * int64_t(rhs->num);
  auto den = int64_t(lhs->den) * int64_t(rhs->den);
  fmc_rational64_new(dest, num, den);
}

void fmc_rational64_add(fmc_rational64_t *dest, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs) {
  auto num = int64_t(lhs->num) * int64_t(rhs->den) + int64_t(rhs->num) * int64_t(lhs->den);
  auto den = int64_t(lhs->den) * int64_t(rhs->den);
  fmc_rational64_new(dest, num, den);
}

void fmc_rational64_sub(fmc_rational64_t *dest, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs) {
  auto num = int64_t(lhs->num) * int64_t(rhs->den) - int64_t(rhs->num) * int64_t(lhs->den);
  auto den = int64_t(lhs->den) * int64_t(rhs->den);
  fmc_rational64_new(dest, num, den);
}

bool fmc_rational64_less(fmc_rational64_t *dest, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs) {
  return int64_t(lhs->num) * int64_t(rhs->den) < int64_t(rhs->num) * int64_t(lhs->den);
}

bool fmc_rational64_greater(fmc_rational64_t *dest, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs) {
  return int64_t(lhs->num) * int64_t(rhs->den) > int64_t(rhs->num) * int64_t(lhs->den);
}

bool fmc_rational64_equal(fmc_rational64_t *dest, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs) {
  return int64_t(lhs->num) * int64_t(rhs->den) == int64_t(rhs->num) * int64_t(lhs->den);
}

bool fmc_rational64_notequal(fmc_rational64_t *dest, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs) {
  return int64_t(lhs->num) * int64_t(rhs->den) != int64_t(rhs->num) * int64_t(lhs->den);
}

void fmc_rational64_inf(fmc_rational64_t *dest) {
  dest->num = 1;
  dest->den = 0;
}

void fmc_rational64_nan(fmc_rational64_t *dest) {
  dest->num = 0;
  dest->den = 0;
}

bool fmc_rational64_is_nan(const fmc_rational64_t *src) { return src->num == 0 && src->den == 0; }

bool fmc_rational64_is_inf(const fmc_rational64_t *src) {
  return (src->num == 1 || src->num == -1) && src->den == 0;
}

void fmc_rational64_abs(fmc_rational64_t *dest, fmc_rational64_t *src) {
  dest->num = std::abs(src->num);
}
