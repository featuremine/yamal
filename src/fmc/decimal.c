#include "fmc/decimal.h"

long double fmc_decimal_bid_to_ld (_Decimal128 a) {
    return __bid_trunctdtf(a);
}

_Decimal128 fmc_decimal_bid_from_ld (long double a) {
    return __bid_extendtftd(a);
}
