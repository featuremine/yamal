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
#include "fmc/alignment.h"
#include "fmc/math.h"

#include "decContext.h"
#include "decQuad.h"

#include "decNumberLocal.h"

#include <fenv.h>
#include <math.h>
#include <stdlib.h>

static decContext *get_context() {
  static __thread bool init = false;
  static __thread decContext set;
  if (!init) {
    decContextDefault(&set, DEC_INIT_DECQUAD);
    init = true;
  }
  return &set;
}

const fmc_decimal128_t fmc_decimal128_exp63[18] = {
    {{0x0000000000000001ull, 0x2208000000000000ull}},
    {{0x948df20da5cfd42eull, 0x2208000000000000ull}},
    {{0x1baaca794cbf6905ull, 0x6a09287164f308e6ull}},
    {{0x0b73daedbf9026abull, 0x3e0df4c7dce94cddull}},
    {{0x244dfd3aae310a9eull, 0x3e12937016f76c96ull}},
    {{0xc01e4a9083dfe45cull, 0x3a17774afe6b54a4ull}},
    {{0x77d8bec0015d1e40ull, 0x3a1c0d6b8e683ab3ull}},
    {{0x84f216ae94929435ull, 0x3620f7889eb3b662ull}},
    {{0xc6782120e94ab0d9ull, 0x3625937891fcccf2ull}},
    {{0x22f32f33d6dd928eull, 0x322a43cdc58dfc6full}},
    {{0x5061045a4c626f1bull, 0x322f255a2215d1b7ull}},
    {{0x6432b208dc600081ull, 0x3233c8982cf312b6ull}},
    {{0x7dcf83b8d04550d7ull, 0x2e38b9a69df3e2c2ull}},
    {{0x30df48fddb5d4f67ull, 0x2e3d65bafcdca3f9ull}},
    {{0xb8b40e238b2b1c5eull, 0x2e4212494ead73a8ull}},
    {{0x6296fa2939e57c0dull, 0x2a46efc0cf1cb76eull}},
    {{0xc70727eb8a5aef05ull, 0x2a4bbc3188347ea4ull}},
    {{0xaea60626d3de3a66ull, 0x2a506b00a0e6705aull}},
};

void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char *src,
                             fmc_error_t **err) {
  fmc_error_clear(err);
  decQuadFromString((decQuad *)dest, src, get_context());
  if (fetestexcept(FE_ALL_EXCEPT)) {
    fmc_error_set(err, "unable to process string");
  }
}

void fmc_decimal128_to_str(char *dest, const fmc_decimal128_t *src) {
  decQuadToString((const decQuad *)src, dest);
}

bool fmc_decimal128_less(const fmc_decimal128_t *lhs,
                         const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompare(&res, (decQuad *)lhs, (decQuad *)rhs, get_context());
  return !decQuadIsZero(&res) && decQuadIsSigned(&res);
}
bool fmc_decimal128_less_or_equal(const fmc_decimal128_t *lhs,
                                  const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompare(&res, (decQuad *)lhs, (decQuad *)rhs, get_context());
  return decQuadIsZero(&res) || decQuadIsSigned(&res);
}
bool fmc_decimal128_greater(const fmc_decimal128_t *lhs,
                            const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompare(&res, (decQuad *)lhs, (decQuad *)rhs, get_context());
  return !decQuadIsZero(&res) && !decQuadIsSigned(&res);
}
bool fmc_decimal128_greater_or_equal(const fmc_decimal128_t *lhs,
                                     const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompare(&res, (decQuad *)lhs, (decQuad *)rhs, get_context());
  return decQuadIsZero(&res) || !decQuadIsSigned(&res);
}
bool fmc_decimal128_equal(const fmc_decimal128_t *lhs,
                          const fmc_decimal128_t *rhs) {
  decQuad res;
  decQuadCompare(&res, (decQuad *)lhs, (decQuad *)rhs, get_context());
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
  uint64_t u = (uint64_t)n;               /* copy as bits */
  uint64_t encode;                        /* work */
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
    feraiseexcept(FE_INVALID);
    return 0;
  }

  /* Here when the argument is finite */
  if (GETEXPUN(df) == 0)
    result = *df;            /* already a true integer */
  else {                     /* need to round to integer */
    enum rounding saveround; /* saver */
    saveround = set->round;  /* save rounding mode .. */
    set->round = rmode;      /* set mode */
    decQuadZero(&zero);      /* make 0E+0 */
    fexcept_t excepts;
    fegetexceptflag(&excepts, FE_ALL_EXCEPT);
    decQuadQuantize(&result, df, &zero, set); /* [this may fail] */
    fesetexceptflag(&excepts, FE_ALL_EXCEPT);
    set->round = saveround; /* restore rounding mode .. */
  }

  /* only the last seven declets of the coefficient can contain */
  /* non-zero; check for others (and also NaN or Infinity from the */
  /* Quantize) first (see DFISZERO for explanation): */
  /* decQuadShow(&result, "sofar"); */
  if ((DFWORD(&result, 1) & 0xffffffc0) != 0 ||
      (DFWORD(&result, 0) & 0x1c003fff) != 0 ||
      (DFWORD(&result, 0) & 0x60000000) == 0x60000000) {
    feraiseexcept(FE_INVALID);
    return 0;
  }
  /* get last twelve digits of the coefficent into hi & ho, base */
  /* 10**9 (see GETCOEFFBILL): */
  sourlo = DFLONG(&result, DECLONGS - 1);
  lo = DPD2BIN[sourlo & 0x3ff] + DPD2BINK[(sourlo >> 10) & 0x3ff] +
       DPD2BINM[(sourlo >> 20) & 0x3ff] +
       ((uint64_t)DPD2BINM[(sourlo >> 30) & 0x3ff]) * 1000 +
       ((uint64_t)DPD2BINM[(sourlo >> 40) & 0x3ff]) * 1000000 +
       ((uint64_t)DPD2BINM[(sourlo >> 50) & 0x3ff]) * 1000000000;
  sourpen = DFLONG(&result, DECLONGS - 2);
  hi = DPD2BIN[((sourpen << 4) | (sourlo >> 60)) & 0x3ff] +
       DPD2BINK[(sourpen >> 6) & 0x3ff];

  /* according to request, check range carefully */
  if (unsign) {
    if (hi > 18 || (hi == 18 && lo > 446744073709551615) ||
        (hi + lo != 0 && DFISSIGNED(&result))) {
      feraiseexcept(FE_INVALID);
      return 0;
    }
    return hi * 1000000000000000000 + lo;
  }
  /* signed */
  if (hi > 9 || (hi == 2 && lo > 223372036854775807)) {
    /* handle the usual edge case */
    if (lo == 223372036854775808 && hi == 9 && DFISSIGNED(&result))
      return 0x80000000;
    feraiseexcept(FE_INVALID);
    return 0;
  }
  i = hi * 1000000000000000000 + lo;
  if (DFISSIGNED(&result))
    i = -i;
  return (uint64_t)i;
}

void fmc_decimal128_to_int(int64_t *dest, const fmc_decimal128_t *src,
                           fmc_error_t **err) {
  fmc_error_clear(err);
  *dest = decToInt64((decQuad *)src, get_context(), DEC_ROUND_HALF_UP, 1, 0);
  if (fetestexcept(FE_ALL_EXCEPT)) {
    fmc_error_set(err, "unable to convert to int");
  }
}

void fmc_decimal128_from_uint(fmc_decimal128_t *res, uint64_t u) {
  uint64_t encode;                        /* work */
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

void fmc_decimal128_to_uint(uint64_t *dest, const fmc_decimal128_t *src,
                            fmc_error_t **err) {
  fmc_error_clear(err);
  *dest = decToInt64((decQuad *)src, get_context(), DEC_ROUND_HALF_UP, 1, 1);
  if (fetestexcept(FE_ALL_EXCEPT)) {
    fmc_error_set(err, "unable to convert to uint");
  }
}

void fmc_decimal128_from_double(fmc_decimal128_t *res, double n) {
  int64_t mantissa = fmc_double_mantissa(n);
  int64_t exp = fmc_double_exp(n);
  bool is_negative = fmc_double_sign(n);
  if (exp == 0x000ll) {
    if (mantissa == 0) {
      fmc_decimal128_from_uint(res, 0);
      if (is_negative) {
        fmc_decimal128_negate(res, res);
      }
      return;
    } else {
      exp = 1ll - 1023ll - 52ll;
    }
  } else if (exp == 0x7ffll) {
    if (mantissa == 0ll) {
      fmc_decimal128_inf(res);
    } else {
      fmc_decimal128_qnan(res);
    }
    if (is_negative) {
      fmc_decimal128_negate(res, res);
    }
    return;
  } else {
    mantissa += (1ll << 52ll);
    exp -= 1023ll + 52ll;
  }
  int64_t absexp = labs(exp);

  fmc_decimal128_t decmantissa, decexp;
  fmc_decimal128_from_uint(&decmantissa, mantissa);
  fmc_decimal128_from_uint(&decexp, (1ll << (absexp % 63ll)));

  if (exp >= 0ll) {
    fmc_decimal128_mul(res, &decmantissa, &decexp);
    if (absexp >= 63ll) {
      fmc_decimal128_mul(res, res, &fmc_decimal128_exp63[absexp / 63ll]);
    }
  } else {
    fmc_decimal128_div(res, &decmantissa, &decexp);
    if (absexp >= 63ll) {
      fmc_decimal128_div(res, res, &fmc_decimal128_exp63[absexp / 63ll]);
    }
  }
  if (is_negative) {
    fmc_decimal128_negate(res, res);
  }
}

void fmc_decimal128_to_double(double *res, const fmc_decimal128_t *src) {
  bool sign = decQuadIsSigned((const decQuad *)src);
  int digits10 = fmc_decimal128_flog10abs(src);
  if (digits10 == INT32_MIN) {
    if (decQuadIsZero((const decQuad *)src)) {
      *res = 0.0;
    } else {
      *res = fmc_decimal128_is_inf(src) ? INFINITY : NAN;
    }
    *res = fmc_double_setsign(*res, sign);
    return;
  }

  int digits2 = digits10 * 33219 / 10000;
  int exp = 53 - digits2;
  int absexp = labs(exp);
  fmc_decimal128_t d;
  fmc_decimal128_from_int(&d, (1ll << (absexp % 63ll)));
  if (exp >= 0ll) {
    fmc_decimal128_mul(&d, src, &d);
    if (absexp >= 63ll) {
      fmc_decimal128_mul(&d, &d, &fmc_decimal128_exp63[absexp / 63ll]);
    }
  } else {
    fmc_decimal128_div(&d, src, &d);
    if (absexp >= 63ll) {
      fmc_decimal128_div(&d, &d, &fmc_decimal128_exp63[absexp / 63ll]);
    }
  }

  fmc_error_t *error;
  int64_t mantissa;
  fmc_decimal128_to_int(&mantissa, &d, &error);
  mantissa = labs(mantissa);

  uint64_t actual_digit2 = FMC_FLOORLOG2(mantissa);
  uint64_t correction = actual_digit2 - 52ull;
  mantissa >>= correction;

  int64_t dexp = digits2 + correction + 1022ull;
  if (dexp < 0) {
    mantissa >>= (-dexp + 1ull);
    dexp = 0;
  }

  *res = fmc_double_make(mantissa, dexp, sign);
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

void fmc_decimal128_round(fmc_decimal128_t *res, const fmc_decimal128_t *val,
                          int decimals) {
  Int exp;     /* exponent */
  uInt sourhi; /* top word from source decFloat */

  /* Start decoding the argument */
  sourhi = DFWORD((const decQuad *)val, 0); /* top word */
  exp = DECCOMBEXP[sourhi >> 26]; /* get exponent high bits (in place) */

  if (EXPISSPECIAL(exp)) { /* is special? */
    *res = *val;
    return;
  }

  fmc_decimal128_t decdigits = fmc_decimal128_exp63[0];
  decContext *ctx = get_context();
  decQuadSetExponent((decQuad *)&decdigits, ctx, -decimals);

  enum rounding saveround = ctx->round;
  ctx->round = DEC_ROUND_HALF_UP;
  decQuadQuantize((decQuad *)res, (const decQuad *)val,
                  (const decQuad *)&decdigits, ctx);
  ctx->round = saveround;
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
  int32_t exp = decQuadGetExponent((decQuad *)res);
  exp += pow;
  decQuadSetExponent((decQuad *)res, get_context(), exp);
}

int fmc_decimal128_lead_zeros(const fmc_decimal128_t *val) {
  const decQuad *df = (const decQuad *)val;
  uInt msd;       /* coefficient MSD */
  uInt comb;      /* combination field */

  /* Source words; macro handles endianness */
  uInt sourhi = DFWORD(df, 0); /* word with sign */
#if DECPMAX == 16
  uInt sourlo = DFWORD(df, 1);
#elif DECPMAX == 34
  uInt sourmh = DFWORD(df, 1);
  uInt sourml = DFWORD(df, 2);
  uInt sourlo = DFWORD(df, 3);
#endif

  comb = sourhi >> 26;    /* sign+combination field */
  msd = DECCOMBMSD[comb]; /* decode the combination field */

  bool stop = msd != 0;
  int left_zeros = !stop;

  const uByte *u; /* .. */

#define dpd2deccount(dpdin)                                                    \
  u = &DPD2BCD8[((dpdin)&0x3ff) * 4];                                          \
  left_zeros += (!stop) * (3 - *(u + 3));                                      \
  stop |= *(u + 3);

#if DECPMAX == 7
  dpd2deccount(sourhi >> 10); /* declet 1 */
  dpd2deccount(sourhi);       /* declet 2 */

#elif DECPMAX == 16
  dpd2deccount(sourhi >> 8);                    /* declet 1 */
  dpd2deccount((sourhi << 2) | (sourlo >> 30)); /* declet 2 */
  dpd2deccount(sourlo >> 20);                   /* declet 3 */
  dpd2deccount(sourlo >> 10);                   /* declet 4 */
  dpd2deccount(sourlo);                         /* declet 5 */

#elif DECPMAX == 34
  dpd2deccount(sourhi >> 4);                    /* declet 1 */
  dpd2deccount((sourhi << 6) | (sourmh >> 26)); /* declet 2 */
  dpd2deccount(sourmh >> 16);                   /* declet 3 */
  dpd2deccount(sourmh >> 6);                    /* declet 4 */
  dpd2deccount((sourmh << 4) | (sourml >> 28)); /* declet 5 */
  dpd2deccount(sourml >> 18);                   /* declet 6 */
  dpd2deccount(sourml >> 8);                    /* declet 7 */
  dpd2deccount((sourml << 2) | (sourlo >> 30)); /* declet 8 */
  dpd2deccount(sourlo >> 20);                   /* declet 9 */
  dpd2deccount(sourlo >> 10);                   /* declet 10 */
  dpd2deccount(sourlo);                         /* declet 11 */
#endif

  return left_zeros;
}

int fmc_decimal128_flog10abs(const fmc_decimal128_t *val) {
  const decQuad *df = (const decQuad *)val;
  Int exp;        /* exponent top two bits or full */
  uInt comb;      /* combination field */

  /* Source words; macro handles endianness */
  uInt sourhi = DFWORD(df, 0); /* word with sign */

  comb = sourhi >> 26;    /* sign+combination field */
  exp = DECCOMBEXP[comb]; /* .. */

  if (!EXPISSPECIAL(exp)) { /* finite */
    /* complete exponent; top two bits are in place */
    exp += GETECON(df) - DECBIAS; /* .. + continuation and unbias */
  } else {                        /* IS special */
    return INT32_MIN;
  }

  int left_zeros = fmc_decimal128_lead_zeros(val);

#if DECPMAX == 7
  const int max_digits = 2 * 3 + 1;
#elif DECPMAX == 16
  const int max_digits = 5 * 3 + 1;
#elif DECPMAX == 34
  const int max_digits = 11 * 3 + 1;
#endif

  if (max_digits == left_zeros) {
    return INT32_MIN;
  }

  return max_digits - left_zeros + exp - 1;
}
