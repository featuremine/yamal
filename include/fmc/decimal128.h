#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <fmc/platform.h>

#define FMC_DECIMAL128_SIZE    16

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

FMMODFUNC bool fmc_decimal128_less(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC bool fmc_decimal128_greater(fmc_decimal128_t lhs, fmc_decimal128_t rhs);
FMMODFUNC bool fmc_decimal128_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs);

//extern decQuad * decQuadDivide(decQuad *, const decQuad *, const decQuad *, decContext *);
FMMODFUNC fmc_decimal128_t fmc_decimal128_divide(fmc_decimal128_t lhs, int64_t rhs);

//extern decQuad * decQuadDivideInteger(decQuad *, const decQuad *, const decQuad *, decContext *);
FMMODFUNC fmc_decimal128_t fmc_decimal64_intdiv(fmc_decimal128_t lhs, int64_t rhs);

FMMODFUNC fmc_decimal128_t fmc_decimal128_add(fmc_decimal128_t lhs, fmc_decimal128_t rhs);

//extern decQuad * decQuadSubtract(decQuad *, const decQuad *, const decQuad *, decContext *);
FMMODFUNC fmc_decimal128_t fmc_decimal64_sub(fmc_decimal128_t lhs, fmc_decimal128_t rhs);

//extern decQuad * decQuadMultiply(decQuad *, const decQuad *, const decQuad *, decContext *);
FMMODFUNC fmc_decimal128_t fmc_decimal128_mul(fmc_decimal128_t lhs, fmc_decimal128_t rhs);

//extern decQuad * decQuadToIntegralValue(decQuad *, const decQuad *, decContext *, enum rounding);
FMMODFUNC fmc_decimal128_t fmc_decimal128_round(fmc_decimal128_t *val);

#ifdef __cplusplus
}
#endif
