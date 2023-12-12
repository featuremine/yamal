/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file fxpt128.h
 * @date 11 Dec 2023
 * @brief Fixed point 128(64.64) C API
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <fmc/platform.h>
#include <fmc/error.h>

#define FMC_FXPT128_STR_SIZE 43

// 64-bit integer support
// If your compiler does not have stdint.h, add appropriate defines for these macros.
#if defined(_MSC_VER) && (_MSC_VER < 1600)
#  define FXPT128_S32 __int32
#  define FXPT128_U32 unsigned __int32
#  define FXPT128_S64 __int64
#  define FXPT128_U64 unsigned __int64
#  define FXPT128_LIT_S64(x) x##i64
#  define FXPT128_LIT_U64(x) x##ui64
#else
#  include <stdint.h>
#  define FXPT128_S32 int32_t
#  define FXPT128_U32 uint32_t
#  define FXPT128_S64 long long
#  define FXPT128_U64 unsigned long long
#  define FXPT128_LIT_S64(x) x##ll
#  define FXPT128_LIT_U64(x) x##ull
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct fmc_fxpt128_t{
   FXPT128_U64 lo;
   FXPT128_U64 hi;
};

// Type conversion
FMMODFUNC void fmc_fxpt128_from_int(struct fmc_fxpt128_t *dst, FXPT128_S64 v);
FMMODFUNC void fmc_fxpt128_from_double(struct fmc_fxpt128_t *dst, double v);
FMMODFUNC FXPT128_S64 fmc_fxpt128_to_int(const struct fmc_fxpt128_t *v);
FMMODFUNC double fmc_fxpt128_to_double(const struct fmc_fxpt128_t *v);

// Copy
FMMODFUNC void fmc_fxpt128_copy(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src);

// Sign manipulation
FMMODFUNC void fmc_fxpt128_neg(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);   // -v
FMMODFUNC void fmc_fxpt128_abs(struct fmc_fxpt128_t* dst, const struct fmc_fxpt128_t* v);   // abs(v)
FMMODFUNC void fmc_fxpt128_nabs(struct fmc_fxpt128_t* dst, const struct fmc_fxpt128_t* v);  // -abs(v)

// Bitwise operations
FMMODFUNC void fmc_fxpt128_not(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src);               // ~a
FMMODFUNC void fmc_fxpt128_or(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);   // a | b
FMMODFUNC void fmc_fxpt128_and(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a & b
FMMODFUNC void fmc_fxpt128_xor(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a ^ b
FMMODFUNC void fmc_fxpt128_shl(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount);   // shift left by amount mod 128
FMMODFUNC void fmc_fxpt128_shr(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount);   // shift right logical by amount mod 128
FMMODFUNC void fmc_fxpt128_sar(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount);   // shift right arithmetic by amount mod 128

// Arithmetic
FMMODFUNC void fmc_fxpt128_add(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a + b
FMMODFUNC void fmc_fxpt128_sub(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a - b
FMMODFUNC void fmc_fxpt128_mul(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a * b
FMMODFUNC void fmc_fxpt128_div(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a / b
FMMODFUNC void fmc_fxpt128_mod(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a - toInt(a / b) * b

FMMODFUNC void fmc_fxpt128_sqrt(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);  // sqrt(v)
FMMODFUNC void fmc_fxpt128_rsqrt(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v); // 1 / sqrt(v)

// Comparison
FMMODFUNC int  fmc_fxpt128_cmp(const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // sign of a-b
FMMODFUNC void fmc_fxpt128_min(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);
FMMODFUNC void fmc_fxpt128_max(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);
FMMODFUNC void fmc_fxpt128_floor(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);
FMMODFUNC void fmc_fxpt128_ceil(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);
FMMODFUNC void fmc_fxpt128_round(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);    // round to nearest, rounding halfway values away from zero
FMMODFUNC int  fmc_fxpt128_isneg(const struct fmc_fxpt128_t *v); // quick check for < 0

// String conversion
//
typedef enum fmc_fxpt128_to_string_sign {
   fmc_fxpt128_to_string_sign_default,  // no sign character for positive values
   fmc_fxpt128_to_string_sign_space,    // leading space for positive values
   fmc_fxpt128_to_string_sign_plus,     // leading '+' for positive values
} fmc_fxpt128_to_string_sign;

// Formatting options for use with fmc_fxpt128_to_string_opt. The "defaults" correspond
// to a format string of "%f".
//
typedef struct fmc_fxpt128_to_string_format {
   // sign character for positive values. Default is fmc_fxpt128_to_string_sign_default.
   fmc_fxpt128_to_string_sign sign;

   // minimum number of characters to write. Default is 0.
   int width;

   // place to the right of the decimal at which rounding is performed. If negative,
   // a maximum of 20 decimal places will be written, with no trailing zeroes.
   // (20 places is sufficient to ensure that fmc_fxpt128_from_string will convert back to the
   // original value.) Default is -1. NOTE: This is not the same default that the C
   // standard library uses for %f.
   int precision;

   // If non-zero, pads the output string with leading zeroes if the final result is
   // fewer than width characters. Otherwise, leading spaces are used. Default is 0.
   int zeroPad;

   // Always print a decimal point, even if the value is an integer. Default is 0.
   int decimal;

   // Left-align output if width specifier requires padding.
   // Default is 0 (right align).
   int leftAlign;
} fmc_fxpt128_to_string_format;

// fmc_fxpt128_to_string_opt: convert struct fmc_fxpt128_t to a decimal string, with formatting.
//
// dst and dstsize: specify the buffer to write into. At most dstsize bytes will be written
// (including null terminator). No additional rounding is performed if dstsize is not large
// enough to hold the entire string.
//
// opt: an fmc_fxpt128_to_string_format struct (q.v.) with formatting options.
//
// Uses the FXPT128_decimal global as the decimal point character.
// Always writes a null terminator, even if the destination buffer is not large enough.
//
// Number of bytes that will be written (i.e. how big does dst need to be?):
// If width is specified: width + 1 bytes.
// If precision is specified: at most precision + 22 bytes.
// If neither is specified: at most 42 bytes.
//
// Returns the number of bytes that would have been written if dst was sufficiently large,
// not including the final null terminator.
//
FMMODFUNC int fmc_fxpt128_to_string_opt(char *dst, size_t dstsize, const struct fmc_fxpt128_t *v, const fmc_fxpt128_to_string_format *opt);

// fmc_fxpt128_to_stringf: convert struct fmc_fxpt128_t to a decimal string, with formatting.
//
// dst and dstsize: specify the buffer to write into. At most dstsize bytes will be written
// (including null terminator).
//
// format: a printf-style format specifier, as one would use with floating point types.
//    e.g. "%+5.2f". (The leading % and trailing f are optional.)
//    NOTE: This is NOT a full replacement for sprintf. Any characters in the format string
//       that do not correspond to a format placeholder are ignored.
//
// Uses the FXPT128_decimal global as the decimal point character.
// Always writes a null terminator, even if the destination buffer is not large enough.
//
// Number of bytes that will be written (i.e. how big does dst need to be?):
// If the precision field is specified: at most max(width, precision + 21) + 1 bytes
// Otherwise: at most max(width, 41) + 1 bytes.
//
// Returns the number of bytes that would have been written if dst was sufficiently large,
// not including the final null terminator.
//
FMMODFUNC int fmc_fxpt128_to_stringf(char *dst, size_t dstsize, const char *format, const struct fmc_fxpt128_t *v);

// fmc_fxpt128_to_string: convert struct fmc_fxpt128_t to a decimal string, with default formatting.
// Equivalent to fmc_fxpt128_to_stringf(dst, dstsize, "%f", v).
//
// Uses the FXPT128_decimal global as the decimal point character.
// Always writes a null terminator, even if the destination buffer is not large enough.
//
// Will write at most 42 bytes (including NUL) to dst.
//
// Returns the number of bytes that would have been written if dst was sufficiently large,
// not including the final null terminator.
//
FMMODFUNC int fmc_fxpt128_to_string(char *dst, size_t dstsize, const struct fmc_fxpt128_t *v);

// fmc_fxpt128_to_str: convert struct fmc_fxpt128_t to a decimal string, with default formatting.
// Equivalent to fmc_fxpt128_to_stringf(dst, dstsize, "%f", v).
//
// Uses the FXPT128_decimal global as the decimal point character.
// Always writes a null terminator, even if the destination buffer is not large enough.
//
// Will write at most 42 bytes (including NUL) to dst.
//
// Returns the number of bytes that would have been written if dst was sufficiently large,
// not including the final null terminator.
//
FMMODFUNC int fmc_fxpt128_to_str(char *dst, const struct fmc_fxpt128_t *v);

// fmc_fxpt128_from_string: Convert string to struct fmc_fxpt128_t.
//
// The string can be formatted either as a decimal number with optional sign
// or as hexadecimal with a prefix of 0x or 0X.
//
// endptr, if not NULL, is set to the character following the last character
//   used in the conversion.
//
FMMODFUNC void fmc_fxpt128_from_string(struct fmc_fxpt128_t *dst, const char *s, char **endptr);

// fmc_fxpt128_from_str: Convert string to struct fmc_fxpt128_t.
//
// The string can be formatted either as a decimal number with optional sign
// or as hexadecimal with a prefix of 0x or 0X.
//
// err, is set to fmc_error_t upon error.
//
FMMODFUNC void fmc_fxpt128_from_str(struct fmc_fxpt128_t *dst, const char *s, fmc_error_t **err);

// Constants
extern const struct fmc_fxpt128_t FXPT128_min;      // minimum (most negative) value
extern const struct fmc_fxpt128_t FXPT128_max;      // maximum (most positive) value
extern const struct fmc_fxpt128_t FXPT128_smallest; // smallest positive value
extern const struct fmc_fxpt128_t FXPT128_zero;     // zero
extern const struct fmc_fxpt128_t FXPT128_one;      // 1.0

extern char FXPT128_decimal;        // decimal point character used by fmc_fxpt128_from/to_string. defaults to '.'

#ifdef __cplusplus
}
#endif   //__cplusplus
