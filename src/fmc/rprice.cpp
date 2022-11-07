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
 * @file rprice.c
 * @date 3 Nov 2022
 * @brief File contains C implementation of rprice object methods
 *
 * This file contains implementation of rprice object methods
 * @see http://www.featuremine.com
 */

#include "fmc/rprice.h"
#include "fmc/math.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void fmc_rprice_from_raw(fmc_rprice_t *dest, int64_t src) { dest->value = src; }
void fmc_rprice_from_old(fmc_rprice_t *dest, const fmc_rprice_t *src) {
  dest->value = src->value * FMC_NEW2OLD_RPRICE_FRACTION;
}
void fmc_rprice_from_ratio(fmc_rprice_t *dest, int64_t num, int64_t denum) {
  dest->value = (FMC_RPRICE_FRACTION / denum) * num;
}

void fmc_rprice_from_int(fmc_rprice_t *dest, int64_t src) {
  dest->value = src * FMC_RPRICE_FRACTION;
}
void fmc_rprice_to_int(int64_t *dest, const fmc_rprice_t *src) {
  *dest = src->value / FMC_RPRICE_FRACTION;
}
void fmc_rprice_from_double(fmc_rprice_t *dest, double src) {
  dest->value = fmc_llround(src * FMC_RPRICE_FRACTION);
}
void fmc_rprice_to_double(double *dest, const fmc_rprice_t *src) {
  *dest = (double)src->value / double(FMC_RPRICE_FRACTION);
}

bool fmc_rprice_less(const fmc_rprice_t *lhs, const fmc_rprice_t *rhs) {
  return lhs->value < rhs->value;
}
bool fmc_rprice_less_or_equal(const fmc_rprice_t *lhs,
                              const fmc_rprice_t *rhs) {
  return lhs->value <= rhs->value;
}
bool fmc_rprice_greater(const fmc_rprice_t *lhs, const fmc_rprice_t *rhs) {
  return lhs->value > rhs->value;
}
bool fmc_rprice_greater_or_equal(const fmc_rprice_t *lhs,
                                 const fmc_rprice_t *rhs) {
  return lhs->value >= rhs->value;
}
bool fmc_rprice_equal(const fmc_rprice_t *lhs, const fmc_rprice_t *rhs) {
  return lhs->value == rhs->value;
}

void fmc_rprice_div(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                    const fmc_rprice_t *rhs) {
  __int128 tmp1 = __int128(lhs->value) * __int128(FMC_RPRICE_FRACTION);
  __int128 tmp2 = __int128(rhs->value);
  res->value = int64_t(tmp1 / tmp2);
}
void fmc_rprice_int_div(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                        int64_t rhs) {
  res->value = lhs->value / rhs;
}
void fmc_rprice_add(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                    const fmc_rprice_t *rhs) {
  res->value = lhs->value + rhs->value;
}
void fmc_rprice_inc(fmc_rprice_t *lhs, const fmc_rprice_t *rhs) {
  lhs->value += rhs->value;
}
void fmc_rprice_sub(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                    const fmc_rprice_t *rhs) {
  res->value = lhs->value - rhs->value;
}
void fmc_rprice_dec(fmc_rprice_t *lhs, const fmc_rprice_t *rhs) {
  lhs->value -= rhs->value;
}
void fmc_rprice_mul(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                    const fmc_rprice_t *rhs) {
  __int128 tmp1 = __int128(lhs->value) * __int128(rhs->value);
  __int128 tmp2 = __int128(FMC_RPRICE_FRACTION);
  res->value = int64_t(tmp1 / tmp2);
}

void fmc_rprice_max(fmc_rprice_t *res) { res->value = FMC_RPRICE_MAX.value; }
void fmc_rprice_min(fmc_rprice_t *res) { res->value = FMC_RPRICE_MIN.value; }

void fmc_rprice_abs(fmc_rprice_t *res, const fmc_rprice_t *val) {
  res->value = llabs(val->value);
}
void fmc_rprice_negate(fmc_rprice_t *res, const fmc_rprice_t *val) {
  res->value = (-val->value);
}

void fmc_rprice_round(int64_t *dest, const fmc_rprice_t *src) {
  *dest = (src->value + fmc_sign(src->value) * (FMC_RPRICE_FRACTION / 2)) /
          FMC_RPRICE_FRACTION;
}
