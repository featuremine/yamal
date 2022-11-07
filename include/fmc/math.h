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
 * @file math.h
 * @author Andres Rangel
 * @date 3 May 2021
 * @brief File contains C declaration of math API
 *
 * This file contains C declaration of math API.
 * @see http://www.featuremine.com
 */

#pragma once

#include <float.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FMC_MAX(a, b)                                                          \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })

#define FMC_MIN(a, b)                                                          \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a < _b ? _a : _b;                                                         \
  })

#define FMC_LESS(a, b)                                                         \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a < _b;                                                                   \
  })

#define FMC_TYPED_PTR_LESS(type, a, b)                                         \
  ({                                                                           \
    type _a = *((type *)(a));                                                  \
    type _b = *((type *)(b));                                                  \
    _a < _b;                                                                   \
  })

#define FMC_SIZE_T_PTR_LESS(a, b) FMC_TYPED_PTR_LESS(size_t, a, b)

#define FMC_INT64_T_PTR_LESS(a, b) FMC_TYPED_PTR_LESS(int64_t, a, b)

#define fmc_sign(x) (((x) > 0) - ((x) < 0))

/**
 * @brief Round to nearest
 *
 * @param x
 * @return the value x rounded to the nearest integral
 */
FMMODFUNC long long fmc_llround(double x);

/**
 * @brief Returns the difference between x and x rounded to the nearest integral
 *
 * @param x
 * @return x - (double)fmc_llround(x)
 */
FMMODFUNC double fmc_remainder(double x);

/**
 * @brief Returns true if -epsilon < x < epsilon
 *
 * @param x
 * @return true if -epsilon < x < epsilon
 */
#define FMC_ALMOST_ZERO(x) !((x) > DBL_EPSILON || (x) < -DBL_EPSILON)

/**
 * @brief Returns true if -epsilon < x - y < epsilon
 *
 * @param x
 * @param y
 * @return true if -epsilon < x - y < epsilon
 */
#define FMC_ALMOST_EQUAL(x, y) FMC_ALMOST_ZERO((x) - (y))

/**
 * @brief Returns true if x - y < epsilon
 *
 * @param x
 * @param y
 * @return true if  x - y < epsilon
 */
#define FMC_ALMOST_LESS(x, y) ((x) - (y) < DBL_EPSILON)

/**
 * @brief Creates a double value by providing the raw mantissa, exponent and
 * sign bit
 *
 * @param mantissa
 * @param exp
 * @param sign
 * @return the IEEE 754 double value
 */
#define fmc_double_make(mantissa, exp, sign)                                   \
  ({                                                                           \
    double _res;                                                               \
    *(uint64_t *)&_res = ((uint64_t)(mantissa) & ((1ull << 52ull) - 1)) |      \
                         ((uint64_t)(exp) << 52ull) |                          \
                         ((uint64_t)(!!(sign)) << 63ull);                      \
    _res;                                                                      \
  })

/**
 * @brief Extract the mantissa bits from a IEEE 754 double value
 *
 * @param value
 * @return the mantissa
 */
#define fmc_double_mantissa(value)                                             \
  ({                                                                           \
    double _val = (value);                                                     \
    (*(uint64_t *)(&_val)) & ((1ull << 52ull) - 1ull);                         \
  })

/**
 * @brief Extract the exponent bits from a IEEE 754 double value
 *
 * @param value
 * @return the exponent
 */
#define fmc_double_exp(value)                                                  \
  ({                                                                           \
    double _val = (value);                                                     \
    (*(uint64_t *)(&_val) >> 52ll) & ((1ll << 11ll) - 1ll);                    \
  })

/**
 * @brief Extract the sign bit from a IEEE 754 double value
 *
 * @param value
 * @return the sign bit
 */
#define fmc_double_sign(value)                                                 \
  ({                                                                           \
    double _val = (value);                                                     \
    (*(int64_t *)(&_val) >> 63ll);                                             \
  })

/**
 * @brief Set the sign bit to a IEEE 754 double value
 *
 * @param value
 * @return the sign bit
 */
#define fmc_double_setsign(value, sign)                                        \
  ({                                                                           \
    double *_val = &(value);                                                   \
    (*(uint64_t *)_val & ~(1ull << 63ull)) |                                   \
        ((uint64_t)((!!(sign))) << 63ull);                                     \
  })

#ifdef __cplusplus
}
#endif
