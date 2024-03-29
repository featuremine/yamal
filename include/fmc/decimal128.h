/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

#include <fmc/error.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FMC_DECIMAL128
#define FMC_DECIMAL128_SIZE 16
#define FMC_DECIMAL128_STR_SIZE 43

typedef enum {
  FMC_DECIMAL128_NEG = 1,
  FMC_DECIMAL128_INF = 2,
  FMC_DECIMAL128_NAN = 4,
  FMC_DECIMAL128_SIG = 8,
  FMC_DECIMAL128_SNAN = 12
} FMC_DECIMAL128_FLAG;

typedef struct {
  uint64_t longs[2];
} fmc_decimal128_t;

FMMODFUNC extern const fmc_decimal128_t fmc_decimal128_exp63[18];

FMMODFUNC const char *fmc_decimal128_parse(fmc_decimal128_t *dest,
                                           const char *src);

FMMODFUNC void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char *src,
                                       fmc_error_t **err);

FMMODFUNC void fmc_decimal128_to_str(char *dest, const fmc_decimal128_t *src);
FMMODFUNC void fmc_decimal128_to_std_str(char *dest,
                                         const fmc_decimal128_t *src,
                                         size_t intdigits, size_t decdigits,
                                         fmc_error_t **error);
FMMODFUNC void fmc_decimal128_from_uint(fmc_decimal128_t *dest, uint64_t src);
FMMODFUNC void fmc_decimal128_to_uint(uint64_t *dest,
                                      const fmc_decimal128_t *src);
FMMODFUNC void fmc_decimal128_from_int(fmc_decimal128_t *dest, int64_t src);
FMMODFUNC void fmc_decimal128_to_int(int64_t *dest,
                                     const fmc_decimal128_t *src);
FMMODFUNC void fmc_decimal128_from_double(fmc_decimal128_t *res, double n);
FMMODFUNC void fmc_decimal128_to_double(double *dest,
                                        const fmc_decimal128_t *res);

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
                                    const fmc_decimal128_t *src, int digits);

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
FMMODFUNC void fmc_decimal128_sign_set(fmc_decimal128_t *res, bool sign);
FMMODFUNC void fmc_decimal128_pow10(fmc_decimal128_t *res, int pow);
FMMODFUNC int fmc_decimal128_lead_zeros(const fmc_decimal128_t *res);
FMMODFUNC int fmc_decimal128_flog10abs(const fmc_decimal128_t *res);
FMMODFUNC void fmc_decimal128_stdrep(fmc_decimal128_t *dest,
                                     const fmc_decimal128_t *src);
FMMODFUNC void fmc_decimal128_pretty(const fmc_decimal128_t *src);
FMMODFUNC void fmc_decimal128_set_triple(fmc_decimal128_t *dest, uint64_t *data,
                                         int64_t len, int64_t exp,
                                         uint16_t flag);
FMMODFUNC void fmc_decimal128_triple(uint64_t *data, int64_t *len, int64_t *exp,
                                     uint16_t *flag,
                                     const fmc_decimal128_t *src);
FMMODFUNC uint32_t fmc_decimal128_digits(const fmc_decimal128_t *src);
FMMODFUNC void fmc_decimal128_powu(fmc_decimal128_t *res,
                                   const fmc_decimal128_t *src, uint64_t n);

#ifdef __cplusplus
}
#endif
