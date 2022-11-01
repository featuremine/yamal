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
#include "decQuad.h"

#include "decNumberLocal.h"

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

const fmc_decimal128_t *get_exp63table() {
  static bool init = false;
  static fmc_decimal128_t table[18];
  if (!init) {
    decContext *set = get_context();
    decQuadFromString((decQuad *)&table[0], "1", set);
    decQuadFromString((decQuad *)&table[1], "9223372036854775808", set);
    decQuadFromString((decQuad *)&table[2],
                      "85070591730234615865843651857942052864", set);
    decQuadFromString(
        (decQuad *)&table[3],
        "784637716923335095479473677900958302012794430558004314112", set);
    decQuadFromString((decQuad *)&table[4],
                      "72370055773322622139731865630429942408293740416025352524"
                      "66099000494570602496",
                      set);
    decQuadFromString((decQuad *)&table[5],
                      "66749594872528440074844428317798503581334516323645399060"
                      "845050244444366430645017188217565216768",
                      set);
    decQuadFromString(
        (decQuad *)&table[6],
        "6156563468186637376918600015647439657043709261010226041866920844413394"
        "02679643915803347910232576806887603562348544",
        set);
    decQuadFromString(
        (decQuad *)&table[7],
        "5678427533559428832416592249125035424637823130369672345949142181098744"
        "438385921275985867583701277855943457200048954515105739075223552",
        set);
    decQuadFromString((decQuad *)&table[8],
                      "52374249726338269920211035149241586435466272736689036631"
                      "73266188953814074247479287813232147721446651441418694604"
                      "0961136147476104734166288853256441430016",
                      set);
    decQuadFromString(
        (decQuad *)&table[9],
        "4830671903771572930869189863664984180373659162133043748321544064314398"
        "9278619505306702422082274032224530795200393777214717063483263037345696"
        "7863584183385093587122601852928",
        set);
    decQuadFromString(
        (decQuad *)&table[10],
        "4455508415646675018204269146191690746966043464109921807206242693261010"
        "9054772240102596804798021205075963303804429632883893444382044682011701"
        "68614570041224793214838549179946240315306828365824",
        set);
    decQuadFromString(
        (decQuad *)&table[11],
        "4109481173084666802532023346000100519961202970955604577733031955522446"
        "9955445943922763019814668659775210804444188892325882964314454560967680"
        "686052895717819140275184930690973423372373108471271228681978529185792",
        set);
    decQuadFromString(
        (decQuad *)&table[12],
        "3790327373781027673703563204254156629045131877726310085788701264712038"
        "4587069748201437461153043126903088079362722926591947548340920771835728"
        "6202948008100864063587640630090308972232735749901964068667724412528434"
        "753635948938919936",
        set);
    decQuadFromString(
        (decQuad *)&table[13],
        "3495959950985713037648777335653666807949431051290362857597833215614143"
        "5553409306835138286457305454559850292369652099267668941480416349336792"
        "5354321796442622320713797704824366482749038836413315139709961037985171"
        "4779776678907072458937421726941708288",
        set);
    decQuadFromString(
        (decQuad *)&table[14],
        "3224453925388581825880980132547098428459761511450937024706791436930382"
        "7060346976286280350090799388909587060241287666545341940158661052584060"
        "7018419472009019109122731932986501567829295456803247713027485905890617"
        "92245363054977512313161523248215761503691988438775496704",
        set);
    decQuadFromString(
        (decQuad *)&table[15],
        "2974033816955566125596124996299801120262520403318788918111543718631881"
        "3143208087470903366289923127011795974475803859461009091704910898114155"
        "8166116220478925156594168089491974788537281966859547374047839156470287"
        "4412135497413755760176314197880697316166024090210908287825647530697629"
        "36832",
        set);
    decQuadFromString(
        (decQuad *)&table[16],
        "2743062034396844341627968125593604635037196317966166035056000994228098"
        "6908798364735825878497681813968066423626689360558724790919313723239516"
        "1205185912283514980724935035500313226779509889596701232075627063117989"
        "7595796976964454084495146379250195728106130226298287754794921070036903"
        "071843030324651025760256",
        set);
    decQuadFromString(
        (decQuad *)&table[17],
        "2530028166341382729406191833986466338119458122051776479461266975342879"
        "2445999418361495047962679640561898384733039601488923726092173224184608"
        "3766749925923137401896780345707951705583634677616520426549709598090931"
        "3357025093542808658732726291945614494454260125706404484619404167682690"
        "3812816523290938580750782913463467636686848",
        set);
    init = true;
  }
  return table;
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
  int64_t mantissa = (*(int64_t *)(&n) & ((1ll << 52ll) - 1ll));
  int64_t exp = (*(int64_t *)(&n) >> 52ll) & ((1ll << 11ll) - 1ll);
  bool is_negative = (*(int64_t *)(&n) >> 63ll);
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
  fmc_decimal128_from_int(&decmantissa, mantissa);
  fmc_decimal128_from_int(&decexp, (1ll << (absexp % 63ll)));

  const fmc_decimal128_t *exptable = get_exp63table();

  if (exp >= 0ll) {
    fmc_decimal128_mul(res, &decmantissa, &decexp);
    fmc_decimal128_mul(res, res, &exptable[absexp / 63ll]);
  } else {
    fmc_decimal128_div(res, &decmantissa, &decexp);
    fmc_decimal128_div(res, res, &exptable[absexp / 63ll]);
  }
  if (is_negative) {
    fmc_decimal128_negate(res, res);
  }
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
  int32_t exp = decQuadGetExponent((decQuad *)res);
  exp += pow;
  decQuadSetExponent((decQuad *)res, get_context(), exp);
}
