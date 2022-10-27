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

#include "decContext.h"
#include "decNumberLocal.h"
#include "decQuad.h"

#include <stdlib.h>

#define DECBYTES DECQUAD_Bytes
#define DECBIAS DECQUAD_Bias
#define DECECONL DECQUAD_EconL

#define GETECON(df)                                                            \
  ((Int)((DFWORD((df), 0) & 0x03ffffff) >> (32 - 6 - DECECONL)))
/* Get the biased exponent similarly			      */
#define GETEXP(df) ((Int)(DECCOMBEXP[DFWORD((df), 0) >> 26] + GETECON(df)))
/* Get the unbiased exponent similarly			      */
#define GETEXPUN(df) ((Int)GETEXP(df) - DECBIAS)

static decContext *get_context() {
  static __thread bool init = false;
  static __thread decContext set;
  if (!init) {
    decContextDefault(&set, DEC_INIT_DECQUAD);
    init = true;
  }
  return &set;
}

void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char *src) {
  decQuadFromString((decQuad *)dest, src, get_context());
}

void fmc_decimal128_to_str(char *dest, const fmc_decimal128_t *src) {
  decQuadToString((const decQuad *)src, dest);
}

bool fmc_decimal128_less(const fmc_decimal128_t *lhs,
                         const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompareTotal(&res, (decQuad *)lhs, (decQuad *)rhs);
  return !decQuadIsZero(&res) && decQuadIsSigned(&res);
}
bool fmc_decimal128_less_or_equal(const fmc_decimal128_t *lhs,
                                  const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompareTotal(&res, (decQuad *)lhs, (decQuad *)rhs);
  return decQuadIsZero(&res) || decQuadIsSigned(&res);
}
bool fmc_decimal128_greater(const fmc_decimal128_t *lhs,
                            const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompareTotal(&res, (decQuad *)lhs, (decQuad *)rhs);
  return !decQuadIsZero(&res) && !decQuadIsSigned(&res);
}
bool fmc_decimal128_greater_or_equal(const fmc_decimal128_t *lhs,
                                     const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompareTotal(&res, (decQuad *)lhs, (decQuad *)rhs);
  return decQuadIsZero(&res) || !decQuadIsSigned(&res);
}
bool fmc_decimal128_equal(const fmc_decimal128_t *lhs,
                          const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompareTotal(&res, (decQuad *)lhs, (decQuad *)rhs);
  return decQuadIsZero(&res);
}

void fmc_decimal128_div(fmc_decimal128_t *res, const fmc_decimal128_t *lhs,
                        const fmc_decimal128_t *rhs) {
  decQuadDivide((decQuad *)res, (decQuad *)lhs, (decQuad *)rhs, get_context());
}

void fmc_decimal128_int_div(fmc_decimal128_t *res, const fmc_decimal128_t *lhs,
                            int64_t rhs) {
  fmc_decimal128_t drhs;
  fmc_decimal128_from_int(&drhs, rhs);
  decQuadDivideInteger((decQuad *)res, (decQuad *)lhs, (decQuad *)&drhs,
                       get_context());
}

void fmc_decimal128_from_int(fmc_decimal128_t *res, int64_t n) {
  uint64_t u = (uint64_t)n; /* copy as bits */
  uint64_t encode;          /* work */
  DFWORD((decQuad *)res, 0) = 0x22080000; /* always */
  DFWORD((decQuad *)res, 1) = 0;
  DFWORD((decQuad *)res, 2) = 0;
  if (n < 0) { /* handle -n with care */
    /* [This can be done without the test, but is then slightly slower] */
    u = (~u) + 1;
    DFWORD((decQuad *)res, 0) |= DECFLOAT_Sign;
  }
  /* Since the maximum value of u now is 2**63, only the low word of */
  /* result is affected */
  encode = ((uint64_t)BIN2DPD[u % 1000]);
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 10;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 20;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 30;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 40;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 50;
  u /= 1000; /* now 0, or 1 */
  encode |= u << 60;
  DFLONG((decQuad *)res, 1) = encode;
}

static uint64_t decToInt64(const decQuad *df, decContext *set,
                           enum rounding rmode, Flag exact, Flag unsign) {
  int64_t exp;                      /* exponent */
  uint64_t sourhi, sourpen, sourlo; /* top word from source decQuad .. */
  uint64_t hi, lo;                  /* .. penultimate, least, etc. */
  decQuad zero, result;             /* work */
  int64_t i;                        /* .. */

  /* Start decoding the argument */
  sourhi = DFWORD(df, 0);         /* top word */
  exp = DECCOMBEXP[sourhi >> 26]; /* get exponent high bits (in place) */
  if (EXPISSPECIAL(exp)) {        /* is special? */
    set->status |= DEC_Invalid_operation; /* signal */
    return 0;
  }

  /* Here when the argument is finite */
  if (GETEXPUN(df) == 0)
    result = *df;                             /* already a true integer */
  else {                                      /* need to round to integer */
    enum rounding saveround;                  /* saver */
    uint64_t savestatus;                      /* .. */
    saveround = set->round;                   /* save rounding mode .. */
    savestatus = set->status;                 /* .. and status */
    set->round = rmode;                       /* set mode */
    decQuadZero(&zero);                       /* make 0E+0 */
    set->status = 0;                          /* clear */
    decQuadQuantize(&result, df, &zero, set); /* [this may fail] */
    set->round = saveround;                   /* restore rounding mode .. */
    if (exact)
      set->status |= savestatus; /* include Inexact */
    else
      set->status = savestatus; /* .. or just original status */
  }

  /* only the last seven declets of the coefficient can contain */
  /* non-zero; check for others (and also NaN or Infinity from the */
  /* Quantize) first (see DFISZERO for explanation): */
  /* decQuadShow(&result, "sofar"); */
  if ((DFWORD(&result, 1) & 0xffffffc0) != 0 ||
      (DFWORD(&result, 0) & 0x1c003fff) != 0 ||
      (DFWORD(&result, 0) & 0x60000000) == 0x60000000) {
    set->status |= DEC_Invalid_operation; /* Invalid or out of range */
    return 0;
  }
  /* get last twelve digits of the coefficent into hi & ho, base */
  /* 10**9 (see GETCOEFFBILL): */
  sourlo = DFLONG(&result, DECLONGS - 1);
  lo = DPD2BIN[sourlo & 0x3ff] +
       DPD2BINK[(sourlo >> 10) & 0x3ff] +
       DPD2BINM[(sourlo >> 20) & 0x3ff] +
       ((uint64_t)DPD2BINM[(sourlo >> 30) & 0x3ff]) * 1000 +
       ((uint64_t)DPD2BINM[(sourlo >> 40) & 0x3ff]) * 1000000  +
       ((uint64_t)DPD2BINM[(sourlo >> 50) & 0x3ff]) * 1000000000;
  sourpen = DFLONG(&result, DECLONGS - 2);
  hi = DPD2BIN[((sourpen << 4) | (sourlo >> 60)) & 0x3ff] +
       DPD2BINK[(sourpen >> 6) & 0x3ff];

  /* according to request, check range carefully */
  if (unsign) {
    if (hi > 18 || (hi == 18 && lo > 446744073709551615) ||
        (hi + lo != 0 && DFISSIGNED(&result))) {
      set->status |= DEC_Invalid_operation; /* out of range */
      return 0;
    }
    return hi * 1000000000000000000 + lo;
  }
  /* signed */
  if (hi > 9 || (hi == 2 && lo > 223372036854775807)) {
    /* handle the usual edge case */
    if (lo == 223372036854775808 && hi == 9 && DFISSIGNED(&result))
      return 0x80000000;
    set->status |= DEC_Invalid_operation; /* truly out of range */
    return 0;
  }
  i = hi * 1000000000000000000 + lo;
  if (DFISSIGNED(&result))
    i = -i;
  return (uint64_t)i;
}

void fmc_decimal128_to_int(int64_t *dest, const fmc_decimal128_t *src) {
  *dest = decToInt64((decQuad *)src, get_context(), DEC_ROUND_HALF_UP, 1, 0);
}

void fmc_decimal128_from_uint(fmc_decimal128_t *res, uint64_t u) {
  uint64_t encode;                                                 /* work */
  DFWORD((decQuad *)res, 0) = 0x22080000; /* always */
  DFWORD((decQuad *)res, 1) = 0;
  DFWORD((decQuad *)res, 2) = 0;
  encode = ((uint64_t)BIN2DPD[u % 1000]);
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 10;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 20;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 30;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 40;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 50;
  u /= 1000;
  encode |= ((uint64_t)BIN2DPD[u % 1000]) << 60;
  DFLONG((decQuad *)res, 1) = encode;
  DFLONG((decQuad *)res, 0) |= u >> 4;
}

void fmc_decimal128_to_uint(uint64_t *dest, const fmc_decimal128_t *src) {
  *dest = decToInt64((decQuad *)src, get_context(), DEC_ROUND_HALF_UP, 1, 1);
}

void fmc_decimal128_add(fmc_decimal128_t *res, const fmc_decimal128_t *lhs,
                        const fmc_decimal128_t *rhs) {
  decQuadAdd((decQuad *)res, (decQuad *)lhs, (decQuad *)rhs, get_context());
}

void fmc_decimal128_inc(fmc_decimal128_t *lhs, const fmc_decimal128_t *rhs) {
  decQuadAdd((decQuad *)lhs, (decQuad *)lhs, (decQuad *)rhs, get_context());
}

void fmc_decimal128_dec(fmc_decimal128_t *lhs, const fmc_decimal128_t *rhs) {
  decQuadSubtract((decQuad *)lhs, (decQuad *)lhs, (decQuad *)rhs,
                  get_context());
}

void fmc_decimal128_sub(fmc_decimal128_t *res, const fmc_decimal128_t *lhs,
                        const fmc_decimal128_t *rhs) {
  decQuadSubtract((decQuad *)res, (decQuad *)lhs, (decQuad *)rhs,
                  get_context());
}

void fmc_decimal128_mul(fmc_decimal128_t *res, const fmc_decimal128_t *lhs,
                        const fmc_decimal128_t *rhs) {
  decQuadMultiply((decQuad *)res, (decQuad *)lhs, (decQuad *)rhs,
                  get_context());
}

void fmc_decimal128_round(fmc_decimal128_t *res, const fmc_decimal128_t *val) {
  decQuadToIntegralValue((decQuad *)res, (decQuad *)val, get_context(),
                         DEC_ROUND_HALF_UP);
}

void fmc_decimal128_qnan(fmc_decimal128_t *res) {
  DFWORD((decQuad *)res, 0) = DECFLOAT_qNaN;
}

void fmc_decimal128_snan(fmc_decimal128_t *res) {
  DFWORD((decQuad *)res, 0) = DECFLOAT_sNaN;
}

bool fmc_decimal128_is_nan(const fmc_decimal128_t *val) {
  return decQuadIsNaN((decQuad *)val);
}

bool fmc_decimal128_is_qnan(const fmc_decimal128_t *val) {
  return decQuadIsNaN((decQuad *)val) && !decQuadIsSignaling((decQuad *)val);
}

bool fmc_decimal128_is_snan(const fmc_decimal128_t *val) {
  return decQuadIsNaN((decQuad *)val) && decQuadIsSignaling((decQuad *)val);
}

void fmc_decimal128_inf(fmc_decimal128_t *res) {
  DFWORD((decQuad *)res, 0) = DECFLOAT_Inf;
}

bool fmc_decimal128_is_inf(const fmc_decimal128_t *val) {
  return decQuadIsInfinite((decQuad *)val);
}

void fmc_decimal128_max(fmc_decimal128_t *res) {
  uint64_t encode; /* work */
  encode = ((uint64_t)BIN2DPD[999]);
  encode |= ((uint64_t)BIN2DPD[999]) << 10;
  encode |= ((uint64_t)BIN2DPD[999]) << 20;
  encode |= ((uint64_t)BIN2DPD[999]) << 30;
  encode |= ((uint64_t)BIN2DPD[999]) << 40;
  encode |= ((uint64_t)BIN2DPD[999]) << 50;
  encode |= ((uint64_t)BIN2DPD[999]) << 60;
  DFLONG((decQuad *)res, 1) = encode;
  encode = ((uint64_t)0x77FFC000) << 32;
  encode |= ((uint64_t)BIN2DPD[999]) >> 4;
  encode |= ((uint64_t)BIN2DPD[999]) << 6;
  encode |= ((uint64_t)BIN2DPD[999]) << 16;
  encode |= ((uint64_t)BIN2DPD[999]) << 26;
  encode |= ((uint64_t)BIN2DPD[999]) << 36;
  DFLONG((decQuad *)res, 0) = encode;
}

void fmc_decimal128_min(fmc_decimal128_t *res) {
  uint64_t encode; /* work */
  encode = ((uint64_t)BIN2DPD[999]);
  encode |= ((uint64_t)BIN2DPD[999]) << 10;
  encode |= ((uint64_t)BIN2DPD[999]) << 20;
  encode |= ((uint64_t)BIN2DPD[999]) << 30;
  encode |= ((uint64_t)BIN2DPD[999]) << 40;
  encode |= ((uint64_t)BIN2DPD[999]) << 50;
  encode |= ((uint64_t)BIN2DPD[999]) << 60;
  DFLONG((decQuad *)res, 1) = encode;
  encode = ((uint64_t)0xF7FFC000) << 32;
  encode |= ((uint64_t)BIN2DPD[999]) >> 4;
  encode |= ((uint64_t)BIN2DPD[999]) << 6;
  encode |= ((uint64_t)BIN2DPD[999]) << 16;
  encode |= ((uint64_t)BIN2DPD[999]) << 26;
  encode |= ((uint64_t)BIN2DPD[999]) << 36;
  DFLONG((decQuad *)res, 0) = encode;
}

bool fmc_decimal128_is_finite(const fmc_decimal128_t *val) {
  return decQuadIsFinite((decQuad *)val);
}

void fmc_decimal128_abs(fmc_decimal128_t *res, const fmc_decimal128_t *val) {
  decQuadAbs((decQuad *)res, (const decQuad *)val, get_context());
}

void fmc_decimal128_negate(fmc_decimal128_t *res, const fmc_decimal128_t *val) {
  decQuadCopyNegate((decQuad *)res, (const decQuad *)val);
}

void fmc_decimal128_pow10(fmc_decimal128_t *res, int pow) {
  int32_t exp = GETEXPUN((decQuad*)res);
  exp+=pow;
  decQuadSetExponent((decQuad *)res, get_context(), exp);
}
