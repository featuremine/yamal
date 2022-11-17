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
#include "fmc/string.h"

#include "decContext.h"
#include "decQuad.h"

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

const char *fmc_decimal128_parse(fmc_decimal128_t *dest, const char *string) {
  Int digits;                         /* count of digits in coefficient */
  const char *dotchar = NULL;         /* where dot was found [NULL if none] */
  const char *cfirst = string;        /* -> first character of decimal part */
  const char *c;                      /* work */
  uByte *ub;                          /* .. */
  uInt uiwork;                        /* for macros */
  bcdnum num;                         /* collects data for finishing */
  uByte buffer[ROUNDUP(DECSTRING + 11, 8)]; /* room for most coefficents, */
  /* some common rounding, +3, & pad */
#if DECTRACE
/* printf("FromString %s ...\n", string); */
#endif

  num.sign = 0;     /* assume non-negative */
  num.msd = buffer; /* MSD is here always */

  /* detect and validate the coefficient, including any leading, */
  /* trailing, or embedded '.' */
  /* [could test four-at-a-time here (saving 10% for decQuads), */
  /* but that risks storage violation because the position of the */
  /* terminator is unknown] */
  for (c = string;; c++) { /* -> input character */
    if (((unsigned)(*c - '0')) <= 9)
      continue; /* '0' through '9' is good */
    if (*c == '\0')
      break; /* most common non-digit */
    if (*c == '.') {
      if (dotchar != NULL)
        break;     /* not first '.' */
      dotchar = c; /* record offset into decimal part */
      continue;
    }
    if (c == string) { /* first in string... */
      if (*c == '-') { /* valid - sign */
        cfirst++;
        num.sign = DECFLOAT_Sign;
        continue;
      }
      if (*c == '+') { /* valid + sign */
        cfirst++;
        continue;
      }
    }
    /* *c is not a digit, terminator, or a valid +, -, or '.' */
    break;
  } /* c loop */

  digits = (uInt)(c - cfirst); /* digits (+1 if a dot) */

  if (digits > 0) {            /* had digits and/or dot */
    const char *clast = c - 1; /* note last coefficient char position */
    const char *parsed = c;
    Int exp = 0;               /* exponent accumulator */
    if (*c == 'E' || *c == 'e') {          /* something follows the coefficient */
      uInt edig;               /* unsigned work */
      /* had some digits and more to come; expect E[+|-]nnn now */
      const char *firstexp; /* exponent first non-zero */
      ++c; /* to (optional) sign */
      if (*c == '-' || *c == '+')
        ++c; /* step over sign (c=clast+2) */
      for(;*c == '0'; ++c, parsed = c);
      firstexp = c; /* remember start [maybe not digit] */
      /* gather exponent digits */
      do {
        edig = (uInt)*c - (uInt)'0';
        if (edig > 9) break;
        exp = exp * 10 + edig;
        parsed = ++c;
      } while(1);
      /* (this next test must be after the syntax checks) */
      /* if definitely more than the possible digits for format then */
      /* the exponent may have wrapped, so simply set it to a certain */
      /* over/underflow value */
      if (c > firstexp + DECEMAXD)
        exp = DECEMAX * 2;
      if (*(clast + 2) == '-')
        exp = -exp; /* was negative */
    }               /* digits>0 */

    if (dotchar != NULL) { /* had a '.' */
      digits--;            /* remove from digits count */
      if (digits == 0)
        return string;     /* was dot alone: bad syntax */
      exp -= (Int)(clast - dotchar); /* adjust exponent */
      /* [the '.' can now be ignored] */
    }
    num.exponent = exp; /* exponent is good; store it */

    /* Here when whole string has been inspected and syntax is good */
    /* cfirst->first digit or dot, clast->last digit or dot */

    /* if the number of digits in the coefficient will fit in buffer */
    /* then it can simply be converted to bcd8 and copied -- decFinalize */
    /* will take care of leading zeros and rounding; the buffer is big */
    /* enough for all canonical coefficients, including 0.00000nn... */
    ub = buffer;
    if (digits <= (Int)(sizeof(buffer) - 3)) { /* [-3 allows by-4s copy] */
      c = cfirst;
      if (dotchar != NULL) {         /* a dot to worry about */
        if (*(c + 1) == '.') {       /* common canonical case */
          *ub++ = (uByte)(*c - '0'); /* copy leading digit */
          c += 2;                    /* prepare to handle rest */
        } else
          for (; c <= clast;) { /* '.' could be anywhere */
            /* as usual, go by fours when safe; NB it has been asserted */
            /* that a '.' does not have the same mask as a digit */
            if (c <= clast - 3                             /* safe for four */
                && (UBTOUI(c) & 0xf0f0f0f0) == CHARMASK) { /* test four */
              UBFROMUI(ub, UBTOUI(c) & 0x0f0f0f0f);        /* to BCD8 */
              ub += 4;
              c += 4;
              continue;
            }
            if (*c == '.') { /* found the dot */
              c++;           /* step over it .. */
              break;         /* .. and handle the rest */
            }
            *ub++ = (uByte)(*c++ - '0');
          }
      } /* had dot */
      /* Now no dot; do this by fours (where safe) */
      for (; c <= clast - 3; c += 4, ub += 4)
        UBFROMUI(ub, UBTOUI(c) & 0x0f0f0f0f);
      for (; c <= clast; c++, ub++)
        *ub = (uByte)(*c - '0');
      num.lsd = buffer + digits - 1; /* record new LSD */
    }                                /* fits */

    else { /* too long for buffer */
      /* [This is a rare and unusual case; arbitrary-length input] */
      /* strip leading zeros [but leave final 0 if all 0's] */
      if (*cfirst == '.')
        cfirst++;           /* step past dot at start */
      if (*cfirst == '0') { /* [cfirst always -> digit] */
        for (; cfirst < clast; cfirst++) {
          if (*cfirst != '0') { /* non-zero found */
            if (*cfirst == '.')
              continue; /* [ignore] */
            break;      /* done */
          }
          digits--; /* 0 stripped */
        }           /* cfirst */
      }             /* at least one leading 0 */

      /* the coefficient is now as short as possible, but may still */
      /* be too long; copy up to Pmax+1 digits to the buffer, then */
      /* just record any non-zeros (set round-for-reround digit) */
      for (c = cfirst; c <= clast && ub <= buffer + DECPMAX; c++) {
        /* (see commentary just above) */
        if (c <= clast - 3                             /* safe for four */
            && (UBTOUI(c) & 0xf0f0f0f0) == CHARMASK) { /* four digits */
          UBFROMUI(ub, UBTOUI(c) & 0x0f0f0f0f);        /* to BCD8 */
          ub += 4;
          c += 3; /* [will become 4] */
          continue;
        }
        if (*c == '.')
          continue; /* [ignore] */
        *ub++ = (uByte)(*c - '0');
      }
      ub--;                     /* -> LSD */
      for (; c <= clast; c++) { /* inspect remaining chars */
        if (*c != '0') {        /* sticky bit needed */
          if (*c == '.')
            continue;              /* [ignore] */
          *ub = DECSTICKYTAB[*ub]; /* update round-for-reround */
          break;                   /* no need to look at more */
        }
      }
      num.lsd = ub; /* record LSD */
      /* adjust exponent for dropped digits */
      num.exponent += digits - (Int)(ub - buffer + 1);
    } /* too long for buffer */

    decQuadFinalize((decQuad *) dest, &num, get_context()); /* round, check, and lay out */
    return parsed;
  }   /* digits or dot */

  /* no digits or dot were found */
  /* only Infinities and NaNs are allowed, here */
  size_t len = 0;
  if ( (len = fmc_cstr_biparse(c, "infinity", "INFINITY")) || (len = fmc_cstr_biparse(c, "inf", "INF")) ) {
    fmc_decimal128_inf(dest);
    fmc_decimal128_sign_set(dest, num.sign);
    return cfirst + len;
  } else if ((len = fmc_cstr_biparse(c, "nan", "NAN"))) {
    fmc_decimal128_qnan(dest);
    fmc_decimal128_sign_set(dest, num.sign);
    return cfirst + len;
  } else if ((len = fmc_cstr_biparse(c, "snan", "SNAN"))) {
    fmc_decimal128_snan(dest);
    fmc_decimal128_sign_set(dest, num.sign);
    return cfirst + len;
  }          /* NaN or sNaN */
  return string;
}

void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char *src, fmc_error_t **err) {
  fmc_error_clear(err);

  if (*src == '\0') {
    fmc_error_set(err, "empty string in conversion");
    return;
  }

  const char *res = fmc_decimal128_parse(dest, src);
  if (*res != '\0') {
    fmc_error_set(err, "only %llu characters parsed", res - src);
  }
}

void fmc_decimal128_to_str(char *dest, const fmc_decimal128_t *src) {
  decQuadToString((const decQuad *)src, dest);
}

static const char longzero_str[] = "0."
                                   "0000000000000000000000000000000000000000000"
                                   "00000000000000000000000000000000000000000";
static const char *zeros_str = longzero_str + 2;

void fmc_decimal128_to_std_str(char *dest, const fmc_decimal128_t *src,
                               size_t intdigits, size_t decdigits,
                               fmc_error_t **error) {
  fmc_error_clear(error);
  const decQuad *df = (const decQuad *)src;

  uInt msd;       /* coefficient MSD */
  Int exp;        /* exponent top two bits or full */
  uInt comb;      /* combination field */
  char *cstart;   /* coefficient start */
  char *c;        /* output pointer in dest */
  char *s, *t;    /* .. (source, target) */
  Int pre;        /* work */
  const uByte *u; /* .. */
  uInt uiwork;    /* for macros [one compiler needs */
  /* volatile here to avoid bug, but */
  /* that doubles execution time] */

  /* Source words; macro handles endianness */
  uInt sourhi = DFWORD(df, 0); /* word with sign */
#if DECPMAX == 16
  uInt sourlo = DFWORD(df, 1);
#elif DECPMAX == 34
  uInt sourmh = DFWORD(df, 1);
  uInt sourml = DFWORD(df, 2);
  uInt sourlo = DFWORD(df, 3);
#endif

  c = dest; /* where result will go */
  if (((Int)sourhi) < 0)
    *c++ = '-';           /* handle sign */
  comb = sourhi >> 26;    /* sign+combination field */
  msd = DECCOMBMSD[comb]; /* decode the combination field */
  exp = DECCOMBEXP[comb]; /* .. */

  if (!EXPISSPECIAL(exp)) { /* finite */
    /* complete exponent; top two bits are in place */
    exp += GETECON(df) - DECBIAS; /* .. + continuation and unbias */
  } else {                        /* IS special */
    fmc_error_set(error, "not a finite number");
    return;
  }

  /* convert the digits of the significand to characters */
  cstart = c; /* save start of coefficient */
  if (msd)
    *c++ = (char)('0' + (char)msd); /* non-zero most significant digit */

/* Decode the declets.  After extracting each declet, it is */
/* decoded to a 4-uByte sequence by table lookup; the four uBytes */
/* are the three encoded BCD8 digits followed by a 1-byte length */
/* (significant digits, except that 000 has length 0).  This allows */
/* us to left-align the first declet with non-zero content, then */
/* the remaining ones are full 3-char length.  Fixed-length copies */
/* are used because variable-length memcpy causes a subroutine call */
/* in at least two compilers.  (The copies are length 4 for speed */
/* and are safe because the last item in the array is of length */
/* three and has the length byte following.) */
#define dpd2char(dpdin)                                                        \
  u = &DPD2BCD8[((dpdin)&0x3ff) * 4];                                          \
  if (c != cstart) {                                                           \
    UBFROMUI(c, UBTOUI(u) | CHARMASK);                                         \
    c += 3;                                                                    \
  } else if (*(u + 3)) {                                                       \
    UBFROMUI(c, UBTOUI(u + 3 - *(u + 3)) | CHARMASK);                          \
    c += *(u + 3);                                                             \
  }

#if DECPMAX == 7
  dpd2char(sourhi >> 10); /* declet 1 */
  dpd2char(sourhi);       /* declet 2 */

#elif DECPMAX == 16
  dpd2char(sourhi >> 8);                    /* declet 1 */
  dpd2char((sourhi << 2) | (sourlo >> 30)); /* declet 2 */
  dpd2char(sourlo >> 20);                   /* declet 3 */
  dpd2char(sourlo >> 10);                   /* declet 4 */
  dpd2char(sourlo);                         /* declet 5 */

#elif DECPMAX == 34
  dpd2char(sourhi >> 4);                    /* declet 1 */
  dpd2char((sourhi << 6) | (sourmh >> 26)); /* declet 2 */
  dpd2char(sourmh >> 16);                   /* declet 3 */
  dpd2char(sourmh >> 6);                    /* declet 4 */
  dpd2char((sourmh << 4) | (sourml >> 28)); /* declet 5 */
  dpd2char(sourml >> 18);                   /* declet 6 */
  dpd2char(sourml >> 8);                    /* declet 7 */
  dpd2char((sourml << 2) | (sourlo >> 30)); /* declet 8 */
  dpd2char(sourlo >> 20);                   /* declet 9 */
  dpd2char(sourlo >> 10);                   /* declet 10 */
  dpd2char(sourlo);                         /* declet 11 */
#endif

  if (c == cstart) {
    memcpy(c, "0", 2);
    return;
  }

  Int right_zeros = 1;
  while (*(c - right_zeros) == '0') {
    ++right_zeros;
  }
  --right_zeros;
  c -= right_zeros;
  exp += right_zeros;

  /*[This fast path is valid but adds 3-5 cycles to worst case length] */
  /*if (exp==0) {		   // integer or NaN case -- easy */
  /*  *c='\0';			   // terminate */
  /*  return string; */
  /*  } */

  Int length = (Int)(c - cstart);

  pre = length + exp; /* length+exp  [c->LSD+1] */
  /* [here, pre-exp is the digits count (==1 for zero)] */

  if (pre > (Int)intdigits || exp < -(Int)decdigits) {
    fmc_error_set(error, "digits limit reached");
    return;
  }
  if (exp > 0) {
    memcpy(c, zeros_str, exp);
    c += exp;
    pre += exp;
  }

  /* modify the coefficient, adding 0s, '.' */
  if (pre > 0) { /* ddd.ddd (plain) */
    char *dotat = cstart + pre;
    if (dotat < c) { /* if embedded dot needed... */
      /* [memmove is a disaster, here] */
      /* move by fours; there must be space for junk at the end */
      /* because exponent is still possible */
      s = dotat + ROUNDDOWN4(c - dotat); /* source */
      t = s + 1;                         /* target */
      /* open the gap [cannot use memcpy] */
      for (; s >= dotat; s -= 4, t -= 4)
        UBFROMUI(t, UBTOUI(s));
      *dotat = '.';
      while (*c == '0') {
        --c;
      }
      if (*c != '.') {
        c++; /* length increased by one */
      }
    }          /* need dot? */
    *c = '\0'; /* add terminator */
    return;
  } /* pre>0 */

  /* here for plain 0.ddd or 0.000ddd forms (can never have E) */
  /* Surprisingly, this is close to being the worst-case path, so the */
  /* shift is done by fours; this is a little tricky because the */
  /* rightmost character to be written must not be beyond where the */
  /* rightmost terminator could be -- so backoff to not touch */
  /* terminator position if need be (this can make exact alignments */
  /* for full Doubles, but in some cases needs care not to access too */
  /* far to the left) */

  pre = -pre + 2;                            /* gap width, including "0." */
  t = cstart + ROUNDDOWN4(c - cstart) + pre; /* preferred first target point */
  /* backoff if too far to the right */
  if (t > dest + DECSTRING - 5)
    t = dest + DECSTRING - 5; /* adjust to fit */
  /* now shift the entire coefficient to the right, being careful not */
  /* to access to the left of string [cannot use memcpy] */
  for (s = t - pre; s >= dest; s -= 4, t -= 4)
    UBFROMUI(t, UBTOUI(s));
  /* for Quads and Singles there may be a character or two left... */
  s += 3; /* where next would come from */
  for (; s >= cstart; s--, t--)
    *(t + 3) = *(s);
  /* now have fill 0. */
  memcpy(cstart, longzero_str, pre);
  c += pre;
  *c = '\0'; /* terminate */
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

void fmc_decimal128_to_int(int64_t *dest, const fmc_decimal128_t *src) {
  *dest = decToInt64((decQuad *)src, get_context(), DEC_ROUND_HALF_UP, 1, 0);
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

void fmc_decimal128_to_uint(uint64_t *dest, const fmc_decimal128_t *src) {
  *dest = decToInt64((decQuad *)src, get_context(), DEC_ROUND_HALF_UP, 1, 1);
}

void fmc_decimal128_from_double(fmc_decimal128_t *res, double n) {
  int64_t mantissa = fmc_double_mantissa(n);
  int64_t exp = fmc_double_exp(n);
  bool is_negative = fmc_double_sign(n);
  if (exp == 0x000ll) {
    if (mantissa == 0) {
      fmc_decimal128_from_uint(res, 0);
      fmc_decimal128_sign_set(res, is_negative);
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
    fmc_decimal128_sign_set(res, is_negative);
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
  fmc_decimal128_sign_set(res, is_negative);
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

  int64_t mantissa;
  fmc_decimal128_to_int(&mantissa, &d);
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

void fmc_decimal128_sign_set(fmc_decimal128_t *res, bool sign) {
  DFBYTE((decQuad *)res, 0) &= ~(1 << 7);
  DFBYTE((decQuad *)res, 0) |= ((!!sign) << 7);
}

void fmc_decimal128_pow10(fmc_decimal128_t *res, int pow) {
  int32_t exp = decQuadGetExponent((decQuad *)res);
  exp += pow;
  decQuadSetExponent((decQuad *)res, get_context(), exp);
}

#define dpd2bcd8addr(dpdin) &DPD2BCD8[((dpdin)&0x3ff) * 4]

int fmc_decimal128_lead_zeros(const fmc_decimal128_t *val) {
  const decQuad *df = (const decQuad *)val;
  uInt msd = GETMSD(df); /* coefficient MSD */
  bool stop = msd != 0;
  int left_zeros = !stop;
  const uByte *u; /* .. */

#define dpd2deccount(dpdin)                                                    \
  u = dpd2bcd8addr(dpdin);                                                     \
  left_zeros += (!stop) * (3 - *(u + 3));                                      \
  stop |= *(u + 3);

  /* Source words; macro handles endianness */
  uInt sourhi = DFWORD(df, 0); /* word with sign */
#if DECPMAX == 7
  dpd2deccount(sourhi >> 10); /* declet 1 */
  dpd2deccount(sourhi);       /* declet 2 */

#elif DECPMAX == 16
  uInt sourlo = DFWORD(df, 1);

  dpd2deccount(sourhi >> 8);                    /* declet 1 */
  dpd2deccount((sourhi << 2) | (sourlo >> 30)); /* declet 2 */
  dpd2deccount(sourlo >> 20);                   /* declet 3 */
  dpd2deccount(sourlo >> 10);                   /* declet 4 */
  dpd2deccount(sourlo);                         /* declet 5 */

#elif DECPMAX == 34
  uInt sourmh = DFWORD(df, 1);
  uInt sourml = DFWORD(df, 2);
  uInt sourlo = DFWORD(df, 3);

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
  Int exp;   /* exponent top two bits or full */
  uInt comb; /* combination field */

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

void fmc_uint64_bebits(uint64_t u, char bits[64]) {
  char *p = bits;
  for (size_t x = 64; x;) {
    --x;
    *(p++) = '0' + ((u & (1ULL << x)) >> x);
  }
}

void fmc_decimal128_bebits(const fmc_decimal128_t *dest, char bits[128]) {
  fmc_uint64_bebits(DFLONG((decQuad *)(dest), 1), bits + 64);
  fmc_uint64_bebits(DFLONG((decQuad *)(dest), 0), bits);
}

void fmc_decimal128_pretty(const fmc_decimal128_t *src) {
  char bits[129] = {0};
  fmc_decimal128_bebits(src, bits);
  printf("%.1s %.5s %.12s %.10s %.10s %.10s %.10s %.10s %.10s %.10s %.10s "
         "%.10s %.10s %.10s\n",
         bits, bits + 1, bits + 6, bits + 18, bits + 28, bits + 38, bits + 48,
         bits + 58, bits + 68, bits + 78, bits + 88, bits + 98, bits + 108,
         bits + 118);
}

void fmc_decimal128_stdrep(fmc_decimal128_t *dest,
                           const fmc_decimal128_t *src) {
  uint32_t exp = DECCOMBEXP[(DFWORD((const decQuad *)src, 0)) >> 26];
  if (EXPISSPECIAL(exp)) {
    // if INF we need to clear 6th bit from the front
    uint16_t sft = 58 + EXPISINF(exp);
    DFLONG((decQuad *)(dest), 0) = (DFLONG((decQuad *)(src), 0) >> sft) << sft;
    DFLONG((decQuad *)(dest), 1) = 0;
    return;
  }

  uint32_t zeros = fmc_decimal128_lead_zeros(src);
  if (!zeros) {
    dest->longs[0] = src->longs[0];
    dest->longs[1] = src->longs[1];
    return;
  } else if (zeros == 34) {
    dest->longs[0] = 0ULL;
    dest->longs[1] = 0ULL;
    return;
  }

#define shiftdec(source, destination, offset)                                  \
  ({                                                                           \
    uint16_t decoffset = (offset)*10;                                          \
    uint64_t sourhi = DFLONG((decQuad *)(source), 0);                          \
    uint64_t sourlo = DFLONG((decQuad *)(source), 1);                          \
    uint64_t mask = (sourhi >> 46) << 46;                                      \
    DFLONG((decQuad *)(destination), 0) = mask | (sourhi & ~mask)              \
                                                     << decoffset;             \
    if (decoffset < 64) {                                                      \
      DFLONG((decQuad *)(destination), 0) |= sourlo >> (64 - decoffset);       \
      DFLONG((decQuad *)(destination), 1) = sourlo << decoffset;               \
    } else {                                                                   \
      DFLONG((decQuad *)(destination), 0) |= sourlo << (decoffset - 64);       \
      DFLONG((decQuad *)(destination), 1) = 0ULL;                              \
    }                                                                          \
  })

  // move everything to the first declet onwards
  uint32_t decsft = (zeros - 1) / 3;
  if (decsft) {
    shiftdec(src, dest, decsft);
  } else {
    dest->longs[0] = src->longs[0];
    dest->longs[1] = src->longs[1];
  }

  exp = GETEXP((decQuad *)src) - zeros;
  const uint8_t *u = dpd2bcd8addr(DFWORD((decQuad *)dest, 0) >> 4);
  uint16_t sigdig = *(u + 3 - *(u + 3));
  exp *= !!sigdig;
  uint32_t top18 =
      DECCOMBFROM[((exp >> DECECONL) << 4) + sigdig] | ((exp & 0xfff) << 14);
  DFWORD((decQuad *)(dest), 0) &= 0x3FFF;
  DFWORD((decQuad *)(dest), 0) |= top18;

  DFBYTE((decQuad *)dest, 0) |= DFBYTE((decQuad *)src, 0) & 0x80;

  sigdig = *(u + 3);
  if (sigdig == 1) {
    shiftdec(dest, dest, 1);
    return;
  }

  uint16_t carry = 0;
  uint16_t sft = 100 * (sigdig == 2) + 10 * (sigdig == 3) +
                 ((sigdig == 1) | (sigdig == 0));
  uint16_t rmd = 1000 / sft;
  uint16_t mult = 0;
  uint64_t dpdout = 0;
  uint16_t n = 0;

#define dpd2sft(dpdin)                                                         \
  n = DPD2BIN[(dpdin)&0x3ff];                                                  \
  dpdout |= ((uint64_t)BIN2DPD[(n % rmd) * sft + carry]) << mult;              \
  carry = n / rmd;                                                             \
  mult += 10;

  /* Source words; macro handles endianness */
  uint32_t sourhi = DFWORD((decQuad *)dest, 0); /* word with sign */
  uint32_t sourmh = DFWORD((decQuad *)dest, 1);
  uint32_t sourml = DFWORD((decQuad *)dest, 2);
  uint32_t sourlo = DFWORD((decQuad *)dest, 3);

  dpd2sft(sourlo);                         /* declet 11 */
  dpd2sft(sourlo >> 10);                   /* declet 10 */
  dpd2sft(sourlo >> 20);                   /* declet 9 */
  dpd2sft((sourml << 2) | (sourlo >> 30)); /* declet 8 */
  dpd2sft(sourml >> 8);                    /* declet 7 */
  dpd2sft(sourml >> 18);                   /* declet 6 */

  DFLONG((decQuad *)(dest), 1) = dpdout;

  mult = 0;
  dpdout = 0;
  dpd2sft((sourmh << 4) | (sourml >> 28)); /* declet 5 */
  dpd2sft(sourmh >> 6);                    /* declet 4 */
  dpd2sft(sourmh >> 16);                   /* declet 3 */
  dpd2sft((sourhi << 6) | (sourmh >> 26)); /* declet 2 */
  dpd2sft(sourhi >> 4);                    /* declet 1 */

  DFLONG((decQuad *)(dest), 1) |= dpdout << 60;
  DFLONG((decQuad *)(dest), 0) &= 0xFFFFC00000000000ULL;
  DFLONG((decQuad *)(dest), 0) |= dpdout >> 4;
}
