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
 * @file decimal.c
 * @date 19 Oct 2022
 * @brief Implementation of the fmc decimal API
 *
 * @see http://www.featuremine.com
 */

#include "fmc/decimal128.h"

#include "decQuad.h"

#define ZEROWORD	 0x22080000

extern const uint16_t BIN2DPD[1000];	/* 0-999 -> DPD 	      */

static decContext* get_context() {
  // __thread identifier supported by clang and gcc
  // https://www.ibm.com/docs/en/i/7.1?topic=specifiers-thread-storage-class-specifier
  static __thread bool init = false;
  static __thread decContext set;
  if (!init) {
    decContextDefault(&set, DEC_INIT_DECQUAD);
    init = true;
  }
  return &set;
}

void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char* src) {
  decQuadFromString((decQuad *)dest, src, get_context());
}

void fmc_decimal128_to_str(fmc_decimal128_t src, char* dest) {
  decQuadToString((decQuad *)&src, dest);
}

bool fmc_decimal128_less(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadCompare((decQuad*)&lhs, (decQuad*)&lhs, (decQuad*)&rhs, get_context());
  return !decQuadIsZero((decQuad*)&lhs) && decQuadIsSigned((decQuad*)&lhs);
}
bool fmc_decimal128_less_or_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadCompare((decQuad*)&lhs, (decQuad*)&lhs, (decQuad*)&rhs, get_context());
  return decQuadIsZero((decQuad*)&lhs) || decQuadIsSigned((decQuad*)&lhs);
}
bool fmc_decimal128_greater(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadCompare((decQuad*)&lhs, (decQuad*)&lhs, (decQuad*)&rhs, get_context());
  return !decQuadIsZero((decQuad*)&lhs) && !decQuadIsSigned((decQuad*)&lhs);
}
bool fmc_decimal128_greater_or_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadCompare((decQuad*)&lhs, (decQuad*)&lhs, (decQuad*)&rhs, get_context());
  return decQuadIsZero((decQuad*)&lhs) || !decQuadIsSigned((decQuad*)&lhs);
}
bool fmc_decimal128_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadCompare((decQuad*)&lhs, (decQuad*)&lhs, (decQuad*)&rhs, get_context());
  return decQuadIsZero((decQuad*)&lhs);
}

fmc_decimal128_t fmc_decimal128_div(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadDivide((decQuad *)&lhs, (decQuad *)&lhs, (decQuad *)&rhs, get_context());
  return lhs;
}

fmc_decimal128_t fmc_decimal128_from_int(int64_t n) {
  uint64_t u=(uint64_t)n;			/* copy as bits */
  uint64_t encode;				/* work */
  fmc_decimal128_t result;
  ((decQuad*)&result)->words[DECQUAD_Bytes/4 - 1 - 0]=ZEROWORD;		/* always */
  ((decQuad*)&result)->words[DECQUAD_Bytes/4 - 1 - 1]=0;
  ((decQuad*)&result)->words[DECQUAD_Bytes/4 - 1 - 2]=0;
  if (n<0) {				/* handle -n with care */
    /* [This can be done without the test, but is then slightly slower] */
    u=(~u)+1;
    ((decQuad*)&result)->words[DECQUAD_Bytes/4 - 1 - 0]|=DECFLOAT_Sign;
  }
  /* Since the maximum value of u now is 2**63, only the low word of */
  /* result is affected */
  encode=((uint64_t)BIN2DPD[u%1000]);
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<10;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<20;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<30;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<40;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<50;
  u/=1000;				/* now 0, or 1 */
  encode|=u<<60;
  ((decQuad*)&result)->longs[0] = encode;
  return result;
}

fmc_decimal128_t fmc_decimal128_from_uint(uint64_t u) {
  uint64_t encode;				/* work */
  fmc_decimal128_t result;
  ((decQuad*)&result)->words[DECQUAD_Bytes/4 - 1 - 0]=ZEROWORD;		/* always */
  ((decQuad*)&result)->words[DECQUAD_Bytes/4 - 1 - 1]=0;
  ((decQuad*)&result)->words[DECQUAD_Bytes/4 - 1 - 2]=0;
  encode=((uint64_t)BIN2DPD[u%1000]);
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<10;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<20;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<30;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<40;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<50;
  u/=1000;
  encode|=((uint64_t)BIN2DPD[u%1000])<<60;
  ((decQuad*)&result)->longs[0] = encode;
  ((decQuad*)&result)->longs[1] |= u>>4;
  return result;
}

fmc_decimal128_t fmc_decimal128_int_div(fmc_decimal128_t lhs, int64_t rhs) {
  fmc_decimal128_t drhs = fmc_decimal128_from_int(rhs);
  decQuadDivide((decQuad *)&lhs, (decQuad *)&lhs, (decQuad *)&drhs, get_context());
  return lhs;
}

fmc_decimal128_t fmc_decimal128_add(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadAdd((decQuad *)&lhs, (decQuad *)&lhs, (decQuad *)&rhs, get_context());
  return lhs;
}

void fmc_decimal128_inc(fmc_decimal128_t *lhs, fmc_decimal128_t rhs) {
  decQuadAdd((decQuad *)lhs, (decQuad *)lhs, (decQuad *)&rhs, get_context());
}

void fmc_decimal128_dec(fmc_decimal128_t *lhs, fmc_decimal128_t rhs) {
  decQuadSubtract((decQuad *)lhs, (decQuad *)lhs, (decQuad *)&rhs, get_context());
}

fmc_decimal128_t fmc_decimal128_sub(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadSubtract((decQuad *)&lhs, (decQuad *)&lhs, (decQuad *)&rhs, get_context());
  return lhs;
}

fmc_decimal128_t fmc_decimal128_mul(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  decQuadMultiply((decQuad *)&lhs, (decQuad *)&lhs, (decQuad *)&rhs, get_context());
  return lhs;
}

fmc_decimal128_t fmc_decimal128_round(fmc_decimal128_t val) {
  decQuadToIntegralValue((decQuad *)&val, (decQuad *)&val, get_context(), DEC_ROUND_HALF_UP);
  return val;
}

fmc_decimal128_t fmc_decimal128_qnan() {
  fmc_decimal128_t ret;
  ((decQuad*)&ret)->words[DECQUAD_Bytes/4 - 1 - 0]=DECFLOAT_qNaN;
  return ret;
}

fmc_decimal128_t fmc_decimal128_snan() {
  fmc_decimal128_t ret;
  ((decQuad*)&ret)->words[DECQUAD_Bytes/4 - 1 - 0]=DECFLOAT_sNaN;
  return ret;
}

bool fmc_decimal128_is_nan(fmc_decimal128_t val) {
  return decQuadIsNaN((decQuad*)&val);
}

bool fmc_decimal128_is_qnan(fmc_decimal128_t val) {
  return decQuadIsNaN((decQuad*)&val) && !decQuadIsSignaling((decQuad*)&val);
}

bool fmc_decimal128_is_snan(fmc_decimal128_t val) {
  return decQuadIsNaN((decQuad*)&val) && decQuadIsSignaling((decQuad*)&val);
}

fmc_decimal128_t fmc_decimal128_inf() {
  fmc_decimal128_t ret;
  ((decQuad*)&ret)->words[DECQUAD_Bytes/4 - 1 - 0]=DECFLOAT_Inf;
  return ret;
}

bool fmc_decimal128_is_inf(fmc_decimal128_t val) {
  return decQuadIsInfinite((decQuad*)&val);
}

fmc_decimal128_t fmc_decimal128_max() {
  fmc_decimal128_t result;
  uint64_t encode;				/* work */
  encode=((uint64_t)BIN2DPD[999]);
  encode|=((uint64_t)BIN2DPD[999])<<10;
  encode|=((uint64_t)BIN2DPD[999])<<20;
  encode|=((uint64_t)BIN2DPD[999])<<30;
  encode|=((uint64_t)BIN2DPD[999])<<40;
  encode|=((uint64_t)BIN2DPD[999])<<50;
  encode|=((uint64_t)BIN2DPD[999])<<60;
  ((decQuad*)&result)->longs[0] = encode;
  encode=((uint64_t)0x77FFC000)<<32;
  encode|=((uint64_t)BIN2DPD[999])>>4;
  encode|=((uint64_t)BIN2DPD[999])<<6;
  encode|=((uint64_t)BIN2DPD[999])<<16;
  encode|=((uint64_t)BIN2DPD[999])<<26;
  encode|=((uint64_t)BIN2DPD[999])<<36;
  ((decQuad*)&result)->longs[1] = encode;
  return result;
}

fmc_decimal128_t fmc_decimal128_min() {
  fmc_decimal128_t result;
  uint64_t encode;				/* work */
  encode=((uint64_t)BIN2DPD[999]);
  encode|=((uint64_t)BIN2DPD[999])<<10;
  encode|=((uint64_t)BIN2DPD[999])<<20;
  encode|=((uint64_t)BIN2DPD[999])<<30;
  encode|=((uint64_t)BIN2DPD[999])<<40;
  encode|=((uint64_t)BIN2DPD[999])<<50;
  encode|=((uint64_t)BIN2DPD[999])<<60;
  ((decQuad*)&result)->longs[0] = encode;
  encode=((uint64_t)0xF7FFC000)<<32;
  encode|=((uint64_t)BIN2DPD[999])>>4;
  encode|=((uint64_t)BIN2DPD[999])<<6;
  encode|=((uint64_t)BIN2DPD[999])<<16;
  encode|=((uint64_t)BIN2DPD[999])<<26;
  encode|=((uint64_t)BIN2DPD[999])<<36;
  ((decQuad*)&result)->longs[1] = encode;
  return result;
}
