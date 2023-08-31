/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file math.c
 * @author Andres Rangel
 * @date 3 May 2021
 * @brief File contains C implementation of math API
 *
 * This file contains C implementation of math API.
 * @see http://www.featuremine.com
 */

#include <fmc/math.h>

#include <float.h>
#include <limits.h>

long long fmc_llround(double x) {
  union {
    double d;
    uint64_t u;
  } u = {x};
  uint64_t absx = u.u & 0x7fffffffffffffffULL;
  long long result = (long long)x;

  if (absx >= 0x4330000000000000ULL) {
    if (x <= (double)LLONG_MIN)
      return LLONG_MIN;
    if (x >= -((double)LLONG_MIN))
      return LLONG_MAX;
    return x;
  }

  if ((float)result != x) {
    union {
      uint64_t u;
      double d;
    } v = {(u.u & 0x8000000000000000ULL) | 0x3fe0000000000000ULL};
    if (absx == 0x3fdfffffffffffffULL)
      return result;
    x += v.d;
    result = (long long)x;
  }

  return result;
}

double fmc_remainder(double x) { return x - (double)fmc_llround(x); }
