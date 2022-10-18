
extern "C" {
#include "decQuad.h"
#include "fmc/decimal128.h"
}


struct fmc_decimal128_context_wrap {
  fmc_decimal128_context_wrap() {
    decContextDefault(&set, DEC_INIT_DECQUAD);
  }
  decContext set;
};

static thread_local fmc_decimal128_context_wrap wrap;

void fmc_decimal128_from_str(fmc_decimal128_t *dest, const char* src) {
  decQuadFromString((decQuad *)dest, src, &wrap.set);
}

void fmc_decimal128_to_str(fmc_decimal128_t src, char* dest) {
  decQuadToString((decQuad *)&src, dest);
}

//TODO: Confirm if it is possible to avoid int32 conversion and find out result from res
bool fmc_decimal128_less(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadCompare((decQuad*)&res, (decQuad*)&lhs, (decQuad*)&rhs, &wrap.set);
  return decQuadToInt32((decQuad*)&res, &wrap.set, DEC_ROUND_HALF_UP) == -1;
}
bool fmc_decimal128_greater(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadCompare((decQuad*)&res, (decQuad*)&lhs, (decQuad*)&rhs, &wrap.set);
  return decQuadToInt32((decQuad*)&res, &wrap.set, DEC_ROUND_HALF_UP) == 1;
}
bool fmc_decimal128_equal(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadCompare((decQuad*)&res, (decQuad*)&lhs, (decQuad*)&rhs, &wrap.set);
  return decQuadToInt32((decQuad*)&res, &wrap.set, DEC_ROUND_HALF_UP) == 0;
}

fmc_decimal128_t fmc_decimal128_divide(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadDivide((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&rhs, &wrap.set);
  return res;
}

fmc_decimal128_t fmc_decimal128_intdiv(fmc_decimal128_t lhs, int32_t rhs) {
  fmc_decimal128_t drhs, res;
  decQuadFromInt32((decQuad*)&drhs, rhs);
  decQuadDivide((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&drhs, &wrap.set);
  return res;
}

fmc_decimal128_t fmc_decimal128_add(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadAdd((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&rhs, &wrap.set);
  return res;
}

fmc_decimal128_t fmc_decimal128_sub(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadSubtract((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&rhs, &wrap.set);
  return res;
}

fmc_decimal128_t fmc_decimal128_mul(fmc_decimal128_t lhs, fmc_decimal128_t rhs) {
  fmc_decimal128_t res;
  decQuadMultiply((decQuad *)&res, (decQuad *)&lhs, (decQuad *)&rhs, &wrap.set);
  return res;
}

fmc_decimal128_t fmc_decimal128_round(fmc_decimal128_t *val) {
  fmc_decimal128_t res;
  decQuadToIntegralValue((decQuad *)&res, (decQuad *)&val, &wrap.set, DEC_ROUND_HALF_UP);
  return res;
}
