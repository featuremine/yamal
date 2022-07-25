/******************************************************************************

       COPYRIGHT (c) 2020 by Featuremine Corporation.
       This software has been provided pursuant to a License Agreement
       containing restrictions on its use.  This software contains
       valuable trade secrets and proprietary information of
       FeatureMine Corporation and is protected by law.  It may not be
       copied or distributed in any form or medium, disclosed to third
       parties, reverse engineered or used in any manner not provided
       for in said License Agreement except with the prior written
       authorization from Featuremine Corporation

*****************************************************************************/

/**
 * @file time.hpp
 * @author Federico Ravchina
 * @date 10 May 2021
 * @brief File contains C declaration of time API
 *
 * This file contains C declaration of time API.
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fmc_time64 {
  int64_t value;
} fmc_time64_t;

FMMODFUNC fmc_time64_t fmc_time64_from_raw(int64_t value);
FMMODFUNC fmc_time64_t fmc_time64_from_nanos(int64_t value);
FMMODFUNC fmc_time64_t fmc_time64_from_seconds(int32_t value);
FMMODFUNC int64_t fmc_time64_to_nanos(fmc_time64_t t);
FMMODFUNC double fmc_time64_to_fseconds(fmc_time64_t t);
FMMODFUNC int64_t fmc_time64_raw(fmc_time64_t time);
FMMODFUNC bool fmc_time64_less(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC bool fmc_time64_greater(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC bool fmc_time64_equal(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC bool fmc_time64_less_or_equal(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC bool fmc_time64_greater_or_equal(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC fmc_time64_t fmc_time64_min(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC fmc_time64_t fmc_time64_max(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC int64_t fmc_time64_div(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC fmc_time64_t fmc_time64_add(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC void fmc_time64_inc(fmc_time64_t *a, fmc_time64_t b);
FMMODFUNC fmc_time64_t fmc_time64_sub(fmc_time64_t a, fmc_time64_t b);
FMMODFUNC fmc_time64_t fmc_time64_mul(fmc_time64_t a, int64_t b);
FMMODFUNC fmc_time64_t fmc_time64_int_div(fmc_time64_t a, int64_t b);
FMMODFUNC fmc_time64_t fmc_time64_start();
FMMODFUNC fmc_time64_t fmc_time64_end();
FMMODFUNC bool fmc_time64_is_end(fmc_time64_t time);

/**
 * @brief Converts given time since epoch into calendar time, expressed in UTC.
 *
 * @param t time_t object to convert
 * @param tm_ tm object to store the result
 */
FMMODFUNC void fmc_gmtime(time_t *t, struct tm *tm_);

/**
 * @brief Converts a string representation of time to a time tm structure
 *
 * Format table:
 * - %% - The % character.
 * - %a or %A - The name of the day of the week according to the current locale,
 * in abbreviated form or the full name.
 * - %b or %B or %h - The month name according to the current locale, in
 * abbreviated form or the full name.
 * - %c - The date and time representation for the current locale.
 * - %C - The century number (0–99).
 * - %d or %e - The day of month (1–31).
 * - %D - Equivalent to %m/%d/%y.  (This is the American style date, very
 * confusing to non-Americans, especially since %d/%m/%y is widely used in
 * Europe.  The ISO 8601 standard format is %Y-%m-%d.)
 * - %H - The hour (0–23).
 * - %I - The hour on a 12-hour clock (1–12).
 * - %j - The day number in the year (1–366).
 * - %m - The month number (1–12).
 * - %M - The minute (0–59).
 * - %n - Arbitrary whitespace.
 * - %p - The locale's equivalent of AM or PM.  (Note: there may be none.)
 * - %r - The 12-hour clock time (using the locale's AM or PM).  In the POSIX
 * locale equivalent to %I:%M:%S %p.  If t_fmt_ampm is empty in the LC_TIME part
 * of the current locale, then the behavior is undefined.
 * - %R - Equivalent to %H:%M.
 * - %S - The second (0–60; 60 may occur for leap seconds; earlier also 61 was
 * allowed).
 * - %t - Arbitrary whitespace.
 * - %T - Equivalent to %H:%M:%S.
 * - %U - The week number with Sunday the first day of the week (0–53). The
 * first Sunday of January is the first day of week 1.
 * - %w - The ordinal number of the day of the week (0–6), with Sunday = 0.
 * - %W - The week number with Monday the first day of the week (0–53).  The
 * first Monday of January is the first day of week 1.
 * - %x - The date, using the locale's date format.
 * - %X - The time, using the locale's time format.
 * - %y - The year within century (0–99).  When a century is not otherwise
 * specified, values in the range 69–99 refer to years in the twentieth century
 * (1969–1999); values in the range 00–68 refer to years in the twenty-first
 * century (2000–2068).
 * - %Y - The year, including century (for example, 1991).
 *
 * @param s string representation of time
 * @param format a character string that consists of field descriptors and text
 * characters
 * @param tm tm object to store the result
 * @return a pointer to the first character not processed in this function call
 */
FMMODFUNC char *fmc_strptime(const char *s, const char *format, struct tm *tm);

/**
 * Returns the time in nanoseconds since the Epoch
 *
 * @return the time in nanoseconds since the Epoch
 */
FMMODFUNC int64_t fmc_cur_time_ns();

#ifdef __cplusplus
}
#endif
