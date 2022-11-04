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

#include "fmc/rprice.h"
#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int32_t num;
  int32_t den;
} fmc_rational64_t;

FMMODFUNC void fmc_rational64_zero(fmc_rational64_t *dest);
FMMODFUNC void fmc_rational64_new(fmc_rational64_t *dest, int32_t num, int32_t den);
FMMODFUNC void fmc_rational64_new2(fmc_rational64_t *dest, int64_t num, int64_t den);
FMMODFUNC void fmc_rational64_from_double(fmc_rational64_t *dest, double value, int32_t base);
FMMODFUNC void fmc_rational64_from_int(fmc_rational64_t *dest, int value);
FMMODFUNC void fmc_rational64_from_rprice(fmc_rational64_t *dest, fmc_rprice_t *src);
FMMODFUNC void fmc_rational64_to_double(double *dest, const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_to_rprice(fmc_rprice_t *dest, const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_div(fmc_rational64_t *res, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs);
FMMODFUNC void fmc_rational64_add(fmc_rational64_t *res, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs);
FMMODFUNC bool fmc_rational64_less(const fmc_rational64_t *lhs, const fmc_rational64_t *rhs);
FMMODFUNC bool fmc_rational64_greater(const fmc_rational64_t *lhs, const fmc_rational64_t *rhs);
FMMODFUNC bool fmc_rational64_equal(const fmc_rational64_t *lhs, const fmc_rational64_t *rhs);
FMMODFUNC bool fmc_rational64_notequal(const fmc_rational64_t *lhs, const fmc_rational64_t *rhs);
FMMODFUNC void fmc_rational64_sub(fmc_rational64_t *res, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs);
FMMODFUNC void fmc_rational64_mul(fmc_rational64_t *res, const fmc_rational64_t *lhs, const fmc_rational64_t *rhs);
FMMODFUNC void fmc_rational64_inf(fmc_rational64_t *dest);
FMMODFUNC void fmc_rational64_nan(fmc_rational64_t *dest);
FMMODFUNC bool fmc_rational64_is_nan(const fmc_rational64_t *src);
FMMODFUNC bool fmc_rational64_is_inf(const fmc_rational64_t *src);
FMMODFUNC void fmc_rational64_abs(fmc_rational64_t *dest, const fmc_rational64_t *src);

#ifdef __cplusplus
}
#endif
