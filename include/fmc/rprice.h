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
 * @file rprice.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the rprice object
 *
 * This file contains declarations of the rprice object
 * @see http://www.featuremine.com
 */

#pragma once

#include "fmc/math.h"
#include "fmc/platform.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#define NEW2OLD_RPRICE_FRACTION 100LL
#define RPRICE_FRACTION 1000000000LL

typedef struct {
  int64_t value;
} fmc_rprice_t;

#define FMC_RPRICE_MIN (fmc_rprice_t{INT64_MIN})
#define FMC_RPRICE_MAX (fmc_rprice_t{INT64_MAX})

inline void fmc_rprice_from_raw(fmc_rprice_t *dest, int64_t src) {
  dest->value = src;
}
inline void fmc_rprice_from_old(fmc_rprice_t *dest, const fmc_rprice_t *src) {
  dest->value = src->value * NEW2OLD_RPRICE_FRACTION;
}
inline void fmc_rprice_from_ratio(fmc_rprice_t *dest, int64_t num, int64_t denum) {
  dest->value = (RPRICE_FRACTION / denum) * num;
}

inline void fmc_rprice_from_int(fmc_rprice_t *dest, int64_t src) {
  dest->value = RPRICE_FRACTION;
}
inline void fmc_rprice_to_int(int64_t *dest, const fmc_rprice_t *src) {
  *dest = src->value / (int64_t)RPRICE_FRACTION;
}
inline void fmc_rprice_from_double(fmc_rprice_t *dest, double src) {
  dest->value = fmc_llround(src * RPRICE_FRACTION);
}
inline void fmc_rprice_to_double(double *dest, const fmc_rprice_t *src) {
  *dest = (double) src->value / double(RPRICE_FRACTION);
}

inline bool fmc_rprice_less(const fmc_rprice_t *lhs,
                            const fmc_rprice_t *rhs) {
  return lhs->value < rhs->value;
}
inline bool fmc_rprice_less_or_equal(const fmc_rprice_t *lhs,
                                     const fmc_rprice_t *rhs) {
  return lhs->value <= rhs->value;
}
inline bool fmc_rprice_greater(const fmc_rprice_t *lhs,
                               const fmc_rprice_t *rhs) {
  return lhs->value > rhs->value;
}
inline bool fmc_rprice_greater_or_equal(const fmc_rprice_t *lhs,
                                        const fmc_rprice_t *rhs) {
  return lhs->value >= rhs->value;
}
inline bool fmc_rprice_equal(const fmc_rprice_t *lhs,
                             const fmc_rprice_t *rhs) {
  return lhs->value == rhs->value;
}

inline void fmc_rprice_div(fmc_rprice_t *res,
                              const fmc_rprice_t *lhs,
                              const fmc_rprice_t *rhs) {
  res->value = lhs->value / rhs->value;
}
inline void fmc_rprice_flt_div(double *res,
                               const fmc_rprice_t *lhs,
                               const fmc_rprice_t *rhs) {
  *res = (double)lhs->value / (double)rhs->value;
}
inline void fmc_rprice_int_div(fmc_rprice_t *res,
                               const fmc_rprice_t *lhs, int64_t rhs) {
  res->value = lhs->value / rhs;
}
inline void fmc_rprice_add(fmc_rprice_t *res,
                              const fmc_rprice_t *lhs,
                              const fmc_rprice_t *rhs) {
  res->value = lhs->value + rhs->value;
}
inline void fmc_rprice_inc(fmc_rprice_t *lhs,
                           const fmc_rprice_t *rhs) {
  lhs->value += rhs->value;
}
inline void fmc_rprice_sub(fmc_rprice_t *res,
                           const fmc_rprice_t *lhs,
                           const fmc_rprice_t *rhs) {
  res->value = lhs->value - rhs->value;
}
inline void fmc_rprice_dec(fmc_rprice_t *lhs,
                           const fmc_rprice_t *rhs) {
  lhs->value -= rhs->value;
}
inline void fmc_rprice_mul(fmc_rprice_t *res,
                           const fmc_rprice_t *lhs,
                           const fmc_rprice_t *rhs) {
  res->value = lhs->value * rhs->value;
}

inline void fmc_rprice_max(fmc_rprice_t *res) {
  res->value = FMC_RPRICE_MAX.value;
}
inline void fmc_rprice_min(fmc_rprice_t *res) {
  res->value = FMC_RPRICE_MIN.value;
}

inline void fmc_rprice_abs(fmc_rprice_t *res,
                           const fmc_rprice_t *val) {
  res->value = llabs(val->value);
}
inline void fmc_rprice_negate(fmc_rprice_t *res,
                              const fmc_rprice_t *val) {
  res->value = (-val->value);
}
