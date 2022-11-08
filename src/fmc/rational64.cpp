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
#include "fmc/math.h"

#include <fenv.h>
#include <numeric>
#include <iostream>
#include <algorithm>

void fmc_rational64_zero(fmc_rational64_t *dest) {
  dest->num = 0;
  dest->den = 1;
}

void fmc_rational64_max(fmc_rational64_t *res) {
  res->num = FMC_RATIONAL64_MAX.num;
  res->den = FMC_RATIONAL64_MAX.den;
}

void fmc_rational64_min(fmc_rational64_t *res) {
  res->num = FMC_RATIONAL64_MIN.num;
  res->den = FMC_RATIONAL64_MIN.den;
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

// TODO: needs to find closest representable rational number
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

void fmc_rational64_from_double(fmc_rational64_t *res, double n) {
  if (std::isnan(n)) {
    fmc_rational64_nan(res);
    return;
  }

  int32_t mantissa = (fmc_double_mantissa(n) + (1ll << 52ll)) >> 22ll;
  int32_t exp = 30 - int32_t(fmc_double_exp(n) - 1023ll);
  int32_t sgn = fmc_double_sign(n);
  sgn = !sgn - sgn;
  int32_t p = exp - std::min(30, exp);
  uint32_t tmp = mantissa >> p;
  exp -= p;
  int32_t num = sgn * tmp;
  int32_t den = (1ll << exp) * (exp >= 0);

  fmc_rational64_new(res, num, den);
}

void fmc_rational64_from_rprice(fmc_rational64_t *dest, fmc_rprice_t *src) {
  fmc_rational64_new2(dest, src->value, FMC_RPRICE_FRACTION);
}

void fmc_rational64_from_int(fmc_rational64_t *dest, int value) {
  dest->num = value;
  dest->den = 1;
}

void fmc_rational64_to_int(int64_t *dest, const fmc_rational64_t *src) {
  *dest = src->num / src->den;
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

void fmc_rational64_to_rprice(fmc_rprice_t *dest, const fmc_rational64_t *src) {
  fmc_rprice_from_ratio(dest, int64_t(src->num), int64_t(src->den));
}

void fmc_rational64_div(fmc_rational64_t *dest, const fmc_rational64_t *lhs,
                        const fmc_rational64_t *rhs) {
  auto num = int64_t(lhs->num) * int64_t(rhs->den);
  auto den = int64_t(lhs->den) * int64_t(rhs->num);
  fmc_rational64_new2(dest, num, den);
}

void fmc_rational64_mul(fmc_rational64_t *dest, const fmc_rational64_t *lhs,
                        const fmc_rational64_t *rhs) {
  auto num = int64_t(lhs->num) * int64_t(rhs->num);
  auto den = int64_t(lhs->den) * int64_t(rhs->den);
  fmc_rational64_new2(dest, num, den);
}

void fmc_rational64_add(fmc_rational64_t *dest, const fmc_rational64_t *lhs,
                        const fmc_rational64_t *rhs) {
  auto num = int64_t(lhs->num) * int64_t(rhs->den) +
             int64_t(rhs->num) * int64_t(lhs->den);
  auto den = int64_t(lhs->den) * int64_t(rhs->den);
  fmc_rational64_new2(dest, num, den);
}

void fmc_rational64_sub(fmc_rational64_t *dest, const fmc_rational64_t *lhs,
                        const fmc_rational64_t *rhs) {
  auto num = int64_t(lhs->num) * int64_t(rhs->den) -
             int64_t(rhs->num) * int64_t(lhs->den);
  auto den = int64_t(lhs->den) * int64_t(rhs->den);
  fmc_rational64_new2(dest, num, den);
}

bool fmc_rational64_less(const fmc_rational64_t *lhs,
                         const fmc_rational64_t *rhs) {
  return int64_t(lhs->num) * int64_t(rhs->den) <
         int64_t(rhs->num) * int64_t(lhs->den);
}

bool fmc_rational64_greater(const fmc_rational64_t *lhs,
                            const fmc_rational64_t *rhs) {
  return int64_t(lhs->num) * int64_t(rhs->den) >
         int64_t(rhs->num) * int64_t(lhs->den);
}

bool fmc_rational64_equal(const fmc_rational64_t *lhs,
                          const fmc_rational64_t *rhs) {
  return int64_t(lhs->num) * int64_t(rhs->den) ==
         int64_t(rhs->num) * int64_t(lhs->den);
}

bool fmc_rational64_notequal(const fmc_rational64_t *lhs,
                             const fmc_rational64_t *rhs) {
  return int64_t(lhs->num) * int64_t(rhs->den) !=
         int64_t(rhs->num) * int64_t(lhs->den);
}

void fmc_rational64_inf(fmc_rational64_t *dest) {
  dest->num = 1;
  dest->den = 0;
}

void fmc_rational64_nan(fmc_rational64_t *dest) {
  dest->num = 0;
  dest->den = 0;
}

bool fmc_rational64_is_nan(const fmc_rational64_t *src) {
  return src->num == 0 && src->den == 0;
}

bool fmc_rational64_is_inf(const fmc_rational64_t *src) {
  return (src->num == 1 || src->num == -1) && src->den == 0;
}

bool fmc_rational64_is_finite(const fmc_rational64_t *src) {
  return !fmc_rational64_is_inf(src) && !fmc_rational64_is_nan(src);
}

void fmc_rational64_abs(fmc_rational64_t *dest, const fmc_rational64_t *src) {
  dest->num = std::abs(src->num);
  dest->den = src->den;
}

void fmc_rational64_negate(fmc_rational64_t *dest,
                           const fmc_rational64_t *src) {
  dest->num = -src->num;
  dest->den = src->den;
}

void fmc_rational64_dec(fmc_rational64_t *res, const fmc_rational64_t *src) {
  fmc_rational64_sub(res, res, src);
}

void fmc_rational64_inc(fmc_rational64_t *res, const fmc_rational64_t *src) {
  fmc_rational64_add(res, res, src);
}
