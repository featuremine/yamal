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
 * @file decimal128.h
 * @date 19 Oct 2022
 * @brief Decimal 128 C API
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <stdint.h>

#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FMC_DECIMAL128_SIZE 16
#define FMC_DECIMAL128_STR_SIZE 43

typedef struct {
  uint8_t bytes[FMC_DECIMAL128_SIZE];
} fmc_decimal128_t;

FMMODFUNC void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char *src);
FMMODFUNC void fmc_decimal128_to_str(char *dest, const fmc_decimal128_t *src);
FMMODFUNC void fmc_decimal128_from_uint(fmc_decimal128_t *dest, uint64_t src);
FMMODFUNC void fmc_decimal128_to_uint(uint64_t *dest,
                                      const fmc_decimal128_t *src);
FMMODFUNC void fmc_decimal128_from_int(fmc_decimal128_t *dest, int64_t src);
FMMODFUNC void fmc_decimal128_to_int(int64_t *dest,
                                     const fmc_decimal128_t *src);

FMMODFUNC bool fmc_decimal128_less(const fmc_decimal128_t *lhs,
                                   const fmc_decimal128_t *rhs);
FMMODFUNC bool fmc_decimal128_less_or_equal(const fmc_decimal128_t *lhs,
                                            const fmc_decimal128_t *rhs);
FMMODFUNC bool fmc_decimal128_greater(const fmc_decimal128_t *lhs,
                                      const fmc_decimal128_t *rhs);
FMMODFUNC bool fmc_decimal128_greater_or_equal(const fmc_decimal128_t *lhs,
                                               const fmc_decimal128_t *rhs);
FMMODFUNC bool fmc_decimal128_equal(const fmc_decimal128_t *lhs,
                                    const fmc_decimal128_t *rhs);

FMMODFUNC void fmc_decimal128_div(fmc_decimal128_t *res,
                                  const fmc_decimal128_t *lhs,
                                  const fmc_decimal128_t *rhs);
FMMODFUNC void fmc_decimal128_int_div(fmc_decimal128_t *res,
                                      const fmc_decimal128_t *lhs, int64_t rhs);
FMMODFUNC void fmc_decimal128_add(fmc_decimal128_t *res,
                                  const fmc_decimal128_t *lhs,
                                  const fmc_decimal128_t *rhs);
FMMODFUNC void fmc_decimal128_inc(fmc_decimal128_t *lhs,
                                  const fmc_decimal128_t *rhs);
FMMODFUNC void fmc_decimal128_sub(fmc_decimal128_t *res,
                                  const fmc_decimal128_t *lhs,
                                  const fmc_decimal128_t *rhs);
FMMODFUNC void fmc_decimal128_dec(fmc_decimal128_t *lhs,
                                  const fmc_decimal128_t *rhs);
FMMODFUNC void fmc_decimal128_mul(fmc_decimal128_t *res,
                                  const fmc_decimal128_t *lhs,
                                  const fmc_decimal128_t *rhs);

FMMODFUNC void fmc_decimal128_round(fmc_decimal128_t *dest,
                                    const fmc_decimal128_t *src);

FMMODFUNC bool fmc_decimal128_is_nan(const fmc_decimal128_t *val);
FMMODFUNC bool fmc_decimal128_is_qnan(const fmc_decimal128_t *val);
FMMODFUNC bool fmc_decimal128_is_snan(const fmc_decimal128_t *val);

FMMODFUNC bool fmc_decimal128_is_inf(const fmc_decimal128_t *val);
FMMODFUNC bool fmc_decimal128_is_finite(const fmc_decimal128_t *val);

FMMODFUNC void fmc_decimal128_qnan(fmc_decimal128_t *res);
FMMODFUNC void fmc_decimal128_snan(fmc_decimal128_t *res);
FMMODFUNC void fmc_decimal128_inf(fmc_decimal128_t *res);
FMMODFUNC void fmc_decimal128_max(fmc_decimal128_t *res);
FMMODFUNC void fmc_decimal128_min(fmc_decimal128_t *res);

FMMODFUNC void fmc_decimal128_abs(fmc_decimal128_t *res,
                                  const fmc_decimal128_t *val);
FMMODFUNC void fmc_decimal128_negate(fmc_decimal128_t *res,
                                     const fmc_decimal128_t *val);

#ifdef __cplusplus
}
#endif
