
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

void fmc_decimal128_to_str(fmc_decimal128_t *src, char* dest) {
  decQuadToString((decQuad *)src, dest);
}

void fmc_decimal128_add(fmc_decimal128_t *res, fmc_decimal128_t *lhs, fmc_decimal128_t *rhs) {
  decQuadAdd((decQuad *)res, (decQuad *)lhs, (decQuad *)rhs, &wrap.set);
}
