#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <fmc/platform.h>

#define FMC_DECIMAL128_SIZE       16
#define FMC_DECIMAL128_STR_SIZE   43

typedef struct {
    uint8_t bytes[FMC_DECIMAL128_SIZE];
} fmc_decimal128_t;

// Pending:
// To x and from x
// MAX
// MIN
// Inplace operations

FMMODFUNC void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char* src);
FMMODFUNC void fmc_decimal128_to_str(fmc_decimal128_t src, char* dest);
FMMODFUNC fmc_decimal128_t fmc_decimal128_from_uint(uint64_t src);
FMMODFUNC fmc_decimal128_t fmc_decimal128_from_int(int64_t src);
FMMODFUNC int64_t fmc_decimal128_to_int(fmc_decimal128_t src);

FMMODFUNC bool fmc_decimal128_less(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC bool fmc_decimal128_less_or_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC bool fmc_decimal128_greater(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC bool fmc_decimal128_greater_or_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC bool fmc_decimal128_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs);

FMMODFUNC fmc_decimal128_t fmc_decimal128_div(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC fmc_decimal128_t fmc_decimal128_int_div(fmc_decimal128_t lhs, int64_t rhs);
FMMODFUNC fmc_decimal128_t fmc_decimal128_add(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC void fmc_decimal128_inc(fmc_decimal128_t *lhs, fmc_decimal128_t rhs);
FMMODFUNC fmc_decimal128_t fmc_decimal128_sub(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC void fmc_decimal128_dec(fmc_decimal128_t *lhs, fmc_decimal128_t rhs);
FMMODFUNC fmc_decimal128_t fmc_decimal128_mul(fmc_decimal128_t lhs, fmc_decimal128_t rhs);

FMMODFUNC fmc_decimal128_t fmc_decimal128_round(fmc_decimal128_t val);

FMMODFUNC fmc_decimal128_t fmc_decimal128_qnan();
FMMODFUNC fmc_decimal128_t fmc_decimal128_snan();
FMMODFUNC bool fmc_decimal128_is_nan(fmc_decimal128_t val);
FMMODFUNC bool fmc_decimal128_is_qnan(fmc_decimal128_t val);
FMMODFUNC bool fmc_decimal128_is_snan(fmc_decimal128_t val);

FMMODFUNC fmc_decimal128_t fmc_decimal128_inf();
FMMODFUNC bool fmc_decimal128_is_inf(fmc_decimal128_t val);

FMMODFUNC fmc_decimal128_t fmc_decimal128_max();
FMMODFUNC fmc_decimal128_t fmc_decimal128_min();

#ifdef __cplusplus
}
#endif
