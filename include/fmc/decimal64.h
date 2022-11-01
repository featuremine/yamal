/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the decimal object
 *
 * This file contains declarations of the decimal object
 * @see http://www.featuremine.com
 */

#ifndef __FM_DECIMAL64_H__
#define __FM_DECIMAL64_H__

#include "fmc/math.h"
#include "fmc/platform.h"
#include <stddef.h>
#include <stdint.h>

#define NEW2OLD_DECIMAL64_FRACTION 100LL
#define DECIMAL64_FRACTION 1000000000LL

typedef struct {
  int64_t value;
} fm_decimal64_t;

inline fm_decimal64_t fm_decimal64_from_raw(int64_t value) {
  fm_decimal64_t res = {value};
  return res;
}

inline fm_decimal64_t fm_decimal64_from_old(fm_decimal64_t a) {
  fm_decimal64_t res = {a.value * NEW2OLD_DECIMAL64_FRACTION};
  return res;
}

inline fm_decimal64_t fm_decimal64_from_double(double value) {
  fm_decimal64_t res = {fmc_llround(value * DECIMAL64_FRACTION)};
  return res;
}

inline fm_decimal64_t fm_decimal64_from_ratio(int64_t num, int64_t denum) {
  fm_decimal64_t res = {(DECIMAL64_FRACTION / denum) * num};
  return res;
}

inline double fm_decimal64_to_double(fm_decimal64_t t) {
  return (double)t.value / (double)DECIMAL64_FRACTION;
}

inline bool fm_decimal64_less(fm_decimal64_t a, fm_decimal64_t b) {
  return a.value < b.value;
}

inline bool fm_decimal64_greater(fm_decimal64_t a, fm_decimal64_t b) {
  return a.value > b.value;
}

inline bool fm_decimal64_equal(fm_decimal64_t a, fm_decimal64_t b) {
  return a.value == b.value;
}

inline double fm_decimal64_div(fm_decimal64_t a, fm_decimal64_t b) {
  return (double)a.value / (double)b.value;
}

inline fm_decimal64_t fm_decimal64_intdiv(fm_decimal64_t a, int b) {
  fm_decimal64_t res = {a.value / b};
  return res;
}

inline fm_decimal64_t fm_decimal64_add(fm_decimal64_t a, fm_decimal64_t b) {
  fm_decimal64_t res = {a.value + b.value};
  return res;
}

inline void fm_decimal64_inplace_add(fm_decimal64_t *a, fm_decimal64_t b) {
  a->value += b.value;
}

inline fm_decimal64_t fm_decimal64_sub(fm_decimal64_t a, fm_decimal64_t b) {
  fm_decimal64_t res = {a.value - b.value};
  return res;
}

inline void fm_decimal64_inplace_sub(fm_decimal64_t *a, fm_decimal64_t b) {
  a->value -= b.value;
}

inline fm_decimal64_t fm_decimal64_mul(fm_decimal64_t a, int64_t b) {
  fm_decimal64_t res = {a.value * b};
  return res;
}

inline fm_decimal64_t fm_decimal64_max() {
  fm_decimal64_t res = {INT64_MAX};
  return res;
}

inline int64_t fm_decimal64_round(fm_decimal64_t num) {
  return (num.value + fmc_sign(num.value) * (DECIMAL64_FRACTION / 2)) /
         DECIMAL64_FRACTION;
}

#define FM_DECIMAL64_MIN (fm_decimal64_t{INT64_MIN})
#define FM_DECIMAL64_MAX (fm_decimal64_t{INT64_MAX})

#endif // __FM_DECIMAL64_H__
