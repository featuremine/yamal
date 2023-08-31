/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file rational64.h
 * @author Maxim Trokhimtchouk
 * @date 28 Dec 2018
 * @brief File contains C definitions of the rational object
 *
 * This file contains declarations of the rational object
 * @see http://www.featuremine.com
 */

#pragma once

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "fmc/platform.h"
#include "fmc/rprice.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int32_t num;
  int32_t den;
} fmc_rational64_t;

#define FMC_RATIONAL64_MIN (fmc_rational64_t{INT32_MIN, 1})
#define FMC_RATIONAL64_MAX (fmc_rational64_t{INT32_MAX, 1})

FMMODFUNC void fmc_rational64_zero(fmc_rational64_t *dest);
FMMODFUNC void fmc_rational64_max(fmc_rational64_t *dest);
FMMODFUNC void fmc_rational64_min(fmc_rational64_t *dest);
FMMODFUNC void fmc_rational64_new(fmc_rational64_t *dest, int32_t num,
                                  int32_t den);
FMMODFUNC void fmc_rational64_new2(fmc_rational64_t *dest, int64_t num,
                                   int64_t den);
FMMODFUNC void fmc_rational64_from_double(fmc_rational64_t *dest, double value);
FMMODFUNC void fmc_rational64_from_int(fmc_rational64_t *dest, int value);
FMMODFUNC void fmc_rational64_from_rprice(fmc_rational64_t *dest,
                                          fmc_rprice_t *src);
FMMODFUNC void fmc_rational64_to_int(int64_t *dest,
                                     const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_to_double(double *dest,
                                        const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_to_rprice(fmc_rprice_t *dest,
                                        const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_div(fmc_rational64_t *res,
                                  const fmc_rational64_t *lhs,
                                  const fmc_rational64_t *rhs);
FMMODFUNC void fmc_rational64_add(fmc_rational64_t *res,
                                  const fmc_rational64_t *lhs,
                                  const fmc_rational64_t *rhs);
FMMODFUNC bool fmc_rational64_less(const fmc_rational64_t *lhs,
                                   const fmc_rational64_t *rhs);
FMMODFUNC bool fmc_rational64_greater(const fmc_rational64_t *lhs,
                                      const fmc_rational64_t *rhs);
FMMODFUNC bool fmc_rational64_equal(const fmc_rational64_t *lhs,
                                    const fmc_rational64_t *rhs);
FMMODFUNC bool fmc_rational64_notequal(const fmc_rational64_t *lhs,
                                       const fmc_rational64_t *rhs);
FMMODFUNC void fmc_rational64_sub(fmc_rational64_t *res,
                                  const fmc_rational64_t *lhs,
                                  const fmc_rational64_t *rhs);
FMMODFUNC void fmc_rational64_mul(fmc_rational64_t *res,
                                  const fmc_rational64_t *lhs,
                                  const fmc_rational64_t *rhs);
FMMODFUNC void fmc_rational64_inf(fmc_rational64_t *dest);
FMMODFUNC void fmc_rational64_nan(fmc_rational64_t *dest);
FMMODFUNC void fmc_rational64_dec(fmc_rational64_t *dest,
                                  const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_inc(fmc_rational64_t *dest,
                                  const fmc_rational64_t *src);
FMMODFUNC bool fmc_rational64_is_nan(const fmc_rational64_t *src);
FMMODFUNC bool fmc_rational64_is_inf(const fmc_rational64_t *src);
FMMODFUNC bool fmc_rational64_is_finite(const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_abs(fmc_rational64_t *dest,
                                  const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_negate(fmc_rational64_t *dest,
                                     const fmc_rational64_t *src);

#ifdef __cplusplus
}
#endif
