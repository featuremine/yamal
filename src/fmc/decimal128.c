

#include "decQuad.h"

// Definitions are present in decQuad.c, however, they
// need to be available for decNumberLocal.h
#define DECBYTES    DECQUAD_Bytes
#define DECPMAX     DECQUAD_Pmax

#include "decDPD.h"
#include "decNumberLocal.h"
#include "fmc/decimal128.h"

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

//TODO: Confirm if it is possible to avoid int32 conversion and find out result from res
bool fmc_decimal128_less(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadCompare((decQuad*)&res, (decQuad*)&lhs, (decQuad*)&rhs, get_context());
  return decQuadToInt32((decQuad*)&res, get_context(), DEC_ROUND_HALF_UP) == -1;
}
bool fmc_decimal128_greater(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadCompare((decQuad*)&res, (decQuad*)&lhs, (decQuad*)&rhs, get_context());
  return decQuadToInt32((decQuad*)&res, get_context(), DEC_ROUND_HALF_UP) == 1;
}
bool fmc_decimal128_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadCompare((decQuad*)&res, (decQuad*)&lhs, (decQuad*)&rhs, get_context());
  return decQuadToInt32((decQuad*)&res, get_context(), DEC_ROUND_HALF_UP) == 0;
}

fmc_decimal128_t fmc_decimal128_divide(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadDivide((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&rhs, get_context());
  return res;
}

fmc_decimal128_t fmc_decimal128_from_int(int64_t n) {
  uint64_t u=(uint64_t)n;			/* copy as bits */
  uint64_t encode;				/* work */
  fmc_decimal128_t result;
  DFWORD((decQuad*)&result, 0)=ZEROWORD;		/* always */
  DFWORD((decQuad*)&result, 1)=0;
  DFWORD((decQuad*)&result, 2)=0;
  if (n<0) {				/* handle -n with care */
    /* [This can be done without the test, but is then slightly slower] */
    u=(~u)+1;
    DFWORD((decQuad*)&result, 0)|=DECFLOAT_Sign;
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

fmc_decimal128_t fmc_decimal128_intdiv(fmc_decimal128_t lhs, int64_t rhs) {
  fmc_decimal128_t drhs = fmc_decimal128_from_int(rhs);
  fmc_decimal128_t res;
  decQuadDivide((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&drhs, get_context());
  return res;
}

fmc_decimal128_t fmc_decimal128_add(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadAdd((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&rhs, get_context());
  return res;
}

fmc_decimal128_t fmc_decimal128_sub(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadSubtract((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&rhs, get_context());
  return res;
}

fmc_decimal128_t fmc_decimal128_mul(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadMultiply((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&rhs, get_context());
  return res;
}

fmc_decimal128_t fmc_decimal128_round(fmc_decimal128_t *val) {
  fmc_decimal128_t res;
  decQuadToIntegralValue((decQuad *)&res, (decQuad *)&val, get_context(), DEC_ROUND_HALF_UP);
  return res;
}
