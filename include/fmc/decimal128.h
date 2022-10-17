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

FMMODFUNC void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char* src);
FMMODFUNC void fmc_decimal128_to_str(fmc_decimal128_t *src, char* dest);
FMMODFUNC void fmc_decimal128_add(fmc_decimal128_t *res, fmc_decimal128_t *lhs, fmc_decimal128_t *rhs);

#ifdef __cplusplus
}
#endif
