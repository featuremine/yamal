/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FMC_NEW2OLD_RPRICE_FRACTION 100LL
#define FMC_RPRICE_FRACTION 1000000000LL

typedef struct {
  int64_t value;
} fmc_rprice_t;

#define FMC_RPRICE_MIN (fmc_rprice_t{INT64_MIN})
#define FMC_RPRICE_MAX (fmc_rprice_t{INT64_MAX})

FMMODFUNC void fmc_rprice_from_raw(fmc_rprice_t *dest, int64_t src);
FMMODFUNC void fmc_rprice_from_old(fmc_rprice_t *dest, const fmc_rprice_t *src);
FMMODFUNC void fmc_rprice_from_ratio(fmc_rprice_t *dest, int64_t num,
                                     int64_t denum);

FMMODFUNC void fmc_rprice_from_int(fmc_rprice_t *dest, int64_t src);
FMMODFUNC void fmc_rprice_to_int(int64_t *dest, const fmc_rprice_t *src);
FMMODFUNC void fmc_rprice_from_double(fmc_rprice_t *dest, double src);
FMMODFUNC void fmc_rprice_to_double(double *dest, const fmc_rprice_t *src);

FMMODFUNC bool fmc_rprice_less(const fmc_rprice_t *lhs,
                               const fmc_rprice_t *rhs);
FMMODFUNC bool fmc_rprice_less_or_equal(const fmc_rprice_t *lhs,
                                        const fmc_rprice_t *rhs);
FMMODFUNC bool fmc_rprice_greater(const fmc_rprice_t *lhs,
                                  const fmc_rprice_t *rhs);
FMMODFUNC bool fmc_rprice_greater_or_equal(const fmc_rprice_t *lhs,
                                           const fmc_rprice_t *rhs);
FMMODFUNC bool fmc_rprice_equal(const fmc_rprice_t *lhs,
                                const fmc_rprice_t *rhs);

FMMODFUNC void fmc_rprice_div(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                              const fmc_rprice_t *rhs);
FMMODFUNC void fmc_rprice_int_div(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                                  int64_t rhs);
FMMODFUNC void fmc_rprice_add(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                              const fmc_rprice_t *rhs);
FMMODFUNC void fmc_rprice_inc(fmc_rprice_t *lhs, const fmc_rprice_t *rhs);
FMMODFUNC void fmc_rprice_sub(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                              const fmc_rprice_t *rhs);
FMMODFUNC void fmc_rprice_dec(fmc_rprice_t *lhs, const fmc_rprice_t *rhs);
FMMODFUNC void fmc_rprice_mul(fmc_rprice_t *res, const fmc_rprice_t *lhs,
                              const fmc_rprice_t *rhs);

FMMODFUNC void fmc_rprice_max(fmc_rprice_t *res);
FMMODFUNC void fmc_rprice_min(fmc_rprice_t *res);

FMMODFUNC void fmc_rprice_abs(fmc_rprice_t *res, const fmc_rprice_t *val);
FMMODFUNC void fmc_rprice_negate(fmc_rprice_t *res, const fmc_rprice_t *val);
FMMODFUNC void fmc_rprice_round(int64_t *dest, const fmc_rprice_t *src);

#ifdef __cplusplus
}
#endif
