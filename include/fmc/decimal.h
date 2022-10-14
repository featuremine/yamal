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
 * @file decimal.h
 * @date 14 Oct 2022
 * @brief Decimal definitions for compatibility
 *
 * @see http://www.featuremine.com
 */

#pragma once

#ifdef __cplusplus
typedef float _Decimal128 __attribute__((mode(TD)));
extern "C" {
#endif

long double fmc_decimal_bid_to_ld (_Decimal128 a);
double fmc_decimal_bid_to_d (_Decimal128 a);
_Decimal128 fmc_decimal_bid_from_ld (long double a);
_Decimal128 fmc_decimal_bid_from_d (double a);

#ifdef __cplusplus
}
#endif
