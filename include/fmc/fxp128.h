/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file fxp128.h
 * @date 11 Dec 2023
 * @brief Fixed point 128(64.64) C API
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>
#include <fmc/platform.h>

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

typedef struct {
   FXPT128_U64 lo;
   FXPT128_U64 hi;
} fmc_fxpt128_t;

// Type conversion
FMMODFUNC void r128FromInt(struct fmc_fxpt128_t *dst, FXPT128_S64 v);
FMMODFUNC void r128FromFloat(struct fmc_fxpt128_t *dst, double v);
FMMODFUNC FXPT128_S64 r128ToInt(const struct fmc_fxpt128_t *v);
FMMODFUNC double r128ToFloat(const struct fmc_fxpt128_t *v);

// Copy
FMMODFUNC void r128Copy(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src);

// Sign manipulation
FMMODFUNC void r128Neg(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);   // -v
FMMODFUNC void r128Abs(struct fmc_fxpt128_t* dst, const struct fmc_fxpt128_t* v);   // abs(v)
FMMODFUNC void r128Nabs(struct fmc_fxpt128_t* dst, const struct fmc_fxpt128_t* v);  // -abs(v)

// Bitwise operations
FMMODFUNC void r128Not(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src);               // ~a
FMMODFUNC void r128Or(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);   // a | b
FMMODFUNC void r128And(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a & b
FMMODFUNC void r128Xor(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a ^ b
FMMODFUNC void r128Shl(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount);   // shift left by amount mod 128
FMMODFUNC void r128Shr(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount);   // shift right logical by amount mod 128
FMMODFUNC void r128Sar(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount);   // shift right arithmetic by amount mod 128

// Arithmetic
FMMODFUNC void r128Add(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a + b
FMMODFUNC void r128Sub(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a - b
FMMODFUNC void r128Mul(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a * b
FMMODFUNC void r128Div(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a / b
FMMODFUNC void r128Mod(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // a - toInt(a / b) * b

FMMODFUNC void r128Sqrt(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);  // sqrt(v)
FMMODFUNC void r128Rsqrt(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v); // 1 / sqrt(v)

// Comparison
FMMODFUNC int  r128Cmp(const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);  // sign of a-b
FMMODFUNC void r128Min(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);
FMMODFUNC void r128Max(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b);
FMMODFUNC void r128Floor(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);
FMMODFUNC void r128Ceil(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);
FMMODFUNC void r128Round(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v);    // round to nearest, rounding halfway values away from zero
FMMODFUNC int  r128IsNeg(const struct fmc_fxpt128_t *v); // quick check for < 0

// String conversion
//
typedef enum R128ToStringSign {
   R128ToStringSign_Default,  // no sign character for positive values
   R128ToStringSign_Space,    // leading space for positive values
   R128ToStringSign_Plus,     // leading '+' for positive values
} R128ToStringSign;

// Formatting options for use with r128ToStringOpt. The "defaults" correspond
// to a format string of "%f".
//
typedef struct R128ToStringFormat {
   // sign character for positive values. Default is R128ToStringSign_Default.
   R128ToStringSign sign;

   // minimum number of characters to write. Default is 0.
   int width;

   // place to the right of the decimal at which rounding is performed. If negative,
   // a maximum of 20 decimal places will be written, with no trailing zeroes.
   // (20 places is sufficient to ensure that r128FromString will convert back to the
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
} R128ToStringFormat;

// r128ToStringOpt: convert struct fmc_fxpt128_t to a decimal string, with formatting.
//
// dst and dstSize: specify the buffer to write into. At most dstSize bytes will be written
// (including null terminator). No additional rounding is performed if dstSize is not large
// enough to hold the entire string.
//
// opt: an R128ToStringFormat struct (q.v.) with formatting options.
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
extern int r128ToStringOpt(char *dst, size_t dstSize, const struct fmc_fxpt128_t *v, const R128ToStringFormat *opt);

// r128ToStringf: convert struct fmc_fxpt128_t to a decimal string, with formatting.
//
// dst and dstSize: specify the buffer to write into. At most dstSize bytes will be written
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
extern int r128ToStringf(char *dst, size_t dstSize, const char *format, const struct fmc_fxpt128_t *v);

// r128ToString: convert struct fmc_fxpt128_t to a decimal string, with default formatting.
// Equivalent to r128ToStringf(dst, dstSize, "%f", v).
//
// Uses the FXPT128_decimal global as the decimal point character.
// Always writes a null terminator, even if the destination buffer is not large enough.
//
// Will write at most 42 bytes (including NUL) to dst.
//
// Returns the number of bytes that would have been written if dst was sufficiently large,
// not including the final null terminator.
//
extern int r128ToString(char *dst, size_t dstSize, const struct fmc_fxpt128_t *v);

// r128FromString: Convert string to struct fmc_fxpt128_t.
//
// The string can be formatted either as a decimal number with optional sign
// or as hexadecimal with a prefix of 0x or 0X.
//
// endptr, if not NULL, is set to the character following the last character
//   used in the conversion.
//
extern void r128FromString(struct fmc_fxpt128_t *dst, const char *s, char **endptr);

// Constants
extern const struct fmc_fxpt128_t FXPT128_min;      // minimum (most negative) value
extern const struct fmc_fxpt128_t FXPT128_max;      // maximum (most positive) value
extern const struct fmc_fxpt128_t FXPT128_smallest; // smallest positive value
extern const struct fmc_fxpt128_t FXPT128_zero;     // zero
extern const struct fmc_fxpt128_t FXPT128_one;      // 1.0

extern char FXPT128_decimal;        // decimal point character used by r128From/ToString. defaults to '.'

#ifdef __cplusplus
}

#include <limits>
namespace std {
template<>
struct numeric_limits<struct fmc_fxpt128_t>
{
   static const bool is_specialized = true;

   static struct fmc_fxpt128_t min() throw() { return FXPT128_min; }
   static struct fmc_fxpt128_t max() throw() { return FXPT128_max; }

   static const int digits = 127;
   static const int digits10 = 38;
   static const bool is_signed = true;
   static const bool is_integer = false;
   static const bool is_exact = false;
   static const int radix = 2;
   static struct fmc_fxpt128_t epsilon() throw() { return FXPT128_smallest; }
   static struct fmc_fxpt128_t round_error() throw() { return FXPT128_one; }

   static const int min_exponent = 0;
   static const int min_exponent10 = 0;
   static const int max_exponent = 0;
   static const int max_exponent10 = 0;

   static const bool has_infinity = false;
   static const bool has_quiet_NaN = false;
   static const bool has_signaling_NaN = false;
   static const float_denorm_style has_denorm = denorm_absent;
   static const bool has_denorm_loss = false;

   static struct fmc_fxpt128_t infinity() throw() { return FXPT128_zero; }
   static struct fmc_fxpt128_t quiet_NaN() throw() { return FXPT128_zero; }
   static struct fmc_fxpt128_t signaling_NaN() throw() { return FXPT128_zero; }
   static struct fmc_fxpt128_t denorm_min() throw() { return FXPT128_zero; }

   static const bool is_iec559 = false;
   static const bool is_bounded = true;
   static const bool is_modulo = true;

   static const bool traps = numeric_limits<FXPT128_U64>::traps;
   static const bool tinyness_before = false;
   static const float_round_style round_style = round_toward_zero;
};
}  //namespace std

inline struct fmc_fxpt128_t::struct fmc_fxpt128_t() {}

inline struct fmc_fxpt128_t::struct fmc_fxpt128_t(double v)
{
   r128FromFloat(this, v);
}

inline struct fmc_fxpt128_t::struct fmc_fxpt128_t(int v)
{
   r128FromInt(this, v);
}

inline struct fmc_fxpt128_t::struct fmc_fxpt128_t(FXPT128_S64 v)
{
   r128FromInt(this, v);
}

inline struct fmc_fxpt128_t::struct fmc_fxpt128_t(FXPT128_U64 low, FXPT128_U64 high)
{
   lo = low;
   hi = high;
}

inline struct fmc_fxpt128_t::operator double() const
{
   return r128ToFloat(this);
}

inline struct fmc_fxpt128_t::operator FXPT128_S64() const
{
   return r128ToInt(this);
}

inline struct fmc_fxpt128_t::operator int() const
{
   return (int) r128ToInt(this);
}

inline struct fmc_fxpt128_t::operator bool() const
{
   return lo || hi;
}

inline bool struct fmc_fxpt128_t::operator!() const
{
   return !lo && !hi;
}

inline struct fmc_fxpt128_t struct fmc_fxpt128_t::operator~() const
{
   struct fmc_fxpt128_t r;
   r128Not(&r, this);
   return r;
}

inline struct fmc_fxpt128_t struct fmc_fxpt128_t::operator-() const
{
   struct fmc_fxpt128_t r;
   r128Neg(&r, this);
   return r;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator|=(const struct fmc_fxpt128_t &rhs)
{
   r128Or(this, this, &rhs);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator&=(const struct fmc_fxpt128_t &rhs)
{
   r128And(this, this, &rhs);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator^=(const struct fmc_fxpt128_t &rhs)
{
   r128Xor(this, this, &rhs);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator+=(const struct fmc_fxpt128_t &rhs)
{
   r128Add(this, this, &rhs);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator-=(const struct fmc_fxpt128_t &rhs)
{
   r128Sub(this, this, &rhs);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator*=(const struct fmc_fxpt128_t &rhs)
{
   r128Mul(this, this, &rhs);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator/=(const struct fmc_fxpt128_t &rhs)
{
   r128Div(this, this, &rhs);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator%=(const struct fmc_fxpt128_t &rhs)
{
   r128Mod(this, this, &rhs);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator<<=(int amount)
{
   r128Shl(this, this, amount);
   return *this;
}

inline struct fmc_fxpt128_t &struct fmc_fxpt128_t::operator>>=(int amount)
{
   r128Sar(this, this, amount);
   return *this;
}

static inline struct fmc_fxpt128_t operator|(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   struct fmc_fxpt128_t r(lhs);
   return r |= rhs;
}

static inline struct fmc_fxpt128_t operator&(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   struct fmc_fxpt128_t r(lhs);
   return r &= rhs;
}

static inline struct fmc_fxpt128_t operator^(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   struct fmc_fxpt128_t r(lhs);
   return r ^= rhs;
}

static inline struct fmc_fxpt128_t operator+(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   struct fmc_fxpt128_t r(lhs);
   return r += rhs;
}

static inline struct fmc_fxpt128_t operator-(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   struct fmc_fxpt128_t r(lhs);
   return r -= rhs;
}

static inline struct fmc_fxpt128_t operator*(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   struct fmc_fxpt128_t r(lhs);
   return r *= rhs;
}

static inline struct fmc_fxpt128_t operator/(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   struct fmc_fxpt128_t r(lhs);
   return r /= rhs;
}

static inline struct fmc_fxpt128_t operator%(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   struct fmc_fxpt128_t r(lhs);
   return r %= rhs;
}

static inline struct fmc_fxpt128_t operator<<(const struct fmc_fxpt128_t &lhs, int amount)
{
   struct fmc_fxpt128_t r(lhs);
   return r <<= amount;
}

static inline struct fmc_fxpt128_t operator>>(const struct fmc_fxpt128_t &lhs, int amount)
{
   struct fmc_fxpt128_t r(lhs);
   return r >>= amount;
}

static inline bool operator<(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   return r128Cmp(&lhs, &rhs) < 0;
}

static inline bool operator>(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   return r128Cmp(&lhs, &rhs) > 0;
}

static inline bool operator<=(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   return r128Cmp(&lhs, &rhs) <= 0;
}

static inline bool operator>=(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   return r128Cmp(&lhs, &rhs) >= 0;
}

static inline bool operator==(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}

static inline bool operator!=(const struct fmc_fxpt128_t &lhs, const struct fmc_fxpt128_t &rhs)
{
   return lhs.lo != rhs.lo || lhs.hi != rhs.hi;
}

#endif   //__cplusplus
#endif   //H_FXPT128_H

#ifdef FXPT128_IMPLEMENTATION

#ifdef FXPT128_DEBUG_VIS
#  define FXPT128_DEBUG_SET(x)   r128ToString(FXPT128_last, sizeof(FXPT128_last), x)
#else
#  define FXPT128_DEBUG_SET(x)
#endif

#define FXPT128_SET2(x, l, h) do { (x)->lo = (FXPT128_U64)(l); (x)->hi = (FXPT128_U64)(h); } while(0)
#define FXPT128_R0(x) ((FXPT128_U32)(x)->lo)
#define FXPT128_R2(x) ((FXPT128_U32)(x)->hi)
#if defined(_M_IX86)
// workaround: MSVC x86's handling of 64-bit values is not great
#  define FXPT128_SET4(x, r0, r1, r2, r3) do { \
      ((FXPT128_U32*)&(x)->lo)[0] = (FXPT128_U32)(r0); \
      ((FXPT128_U32*)&(x)->lo)[1] = (FXPT128_U32)(r1); \
      ((FXPT128_U32*)&(x)->hi)[0] = (FXPT128_U32)(r2); \
      ((FXPT128_U32*)&(x)->hi)[1] = (FXPT128_U32)(r3); \
      } while(0)
#  define FXPT128_R1(x) (((FXPT128_U32*)&(x)->lo)[1])
#  define FXPT128_R3(x) (((FXPT128_U32*)&(x)->hi)[1])
#else
#  define FXPT128_SET4(x, r0, r1, r2, r3) do { (x)->lo = (FXPT128_U64)(r0) | ((FXPT128_U64)(r1) << 32); \
      (x)->hi = (FXPT128_U64)(r2) | ((FXPT128_U64)(r3) << 32); } while(0)
#  define FXPT128_R1(x) ((FXPT128_U32)((x)->lo >> 32))
#  define FXPT128_R3(x) ((FXPT128_U32)((x)->hi >> 32))
#endif

#if defined(_M_X64)
#  define FXPT128_INTEL 1
#  define FXPT128_64BIT 1
#  ifndef FXPT128_STDC_ONLY
#     include <intrin.h>
#  endif
#elif defined(__x86_64__)
#  define FXPT128_INTEL 1
#  define FXPT128_64BIT 1
#  ifndef FXPT128_STDC_ONLY
#     include <x86intrin.h>
#  endif
#elif defined(_M_IX86)
#  define FXPT128_INTEL 1
#  ifndef FXPT128_STDC_ONLY
#     include <intrin.h>
#  endif
#elif defined(__i386__)
#  define FXPT128_INTEL 1
#  ifndef FXPT128_STDC_ONLY
#     include <x86intrin.h>
#  endif
#elif defined(_M_ARM)
#  ifndef FXPT128_STDC_ONLY
#     include <intrin.h>
#  endif
#elif defined(_M_ARM64)
#  define FXPT128_64BIT 1
#  ifndef FXPT128_STDC_ONLY
#     include <intrin.h>
#  endif
#elif defined(__aarch64__)
#  define FXPT128_64BIT 1
#endif

#ifndef FXPT128_INTEL
#  define FXPT128_INTEL 0
#endif

#ifndef FXPT128_64BIT
#  define FXPT128_64BIT 0
#endif

#ifndef FXPT128_ASSERT
#  include <assert.h>
#  define FXPT128_ASSERT(x) assert(x)
#endif

#include <stdlib.h>  // for NULL

static const R128ToStringFormat FXPT128__defaultFormat = {
   R128ToStringSign_Default,
   0,
   -1,
   0,
   0,
   0
};

const struct fmc_fxpt128_t FXPT128_min = { 0, FXPT128_LIT_U64(0x8000000000000000) };
const struct fmc_fxpt128_t FXPT128_max = { FXPT128_LIT_U64(0xffffffffffffffff), FXPT128_LIT_U64(0x7fffffffffffffff) };
const struct fmc_fxpt128_t FXPT128_smallest = { 1, 0 };
const struct fmc_fxpt128_t FXPT128_zero = { 0, 0 };
const struct fmc_fxpt128_t FXPT128_one = { 0, 1 };
char FXPT128_decimal = '.';
#ifdef FXPT128_DEBUG_VIS
char FXPT128_last[42];
#endif

static int r128__clz64(FXPT128_U64 x)
{
#if defined(FXPT128_STDC_ONLY)
   FXPT128_U64 n = 64, y;
   y = x >> 32; if (y) { n -= 32; x = y; }
   y = x >> 16; if (y) { n -= 16; x = y; }
   y = x >>  8; if (y) { n -=  8; x = y; }
   y = x >>  4; if (y) { n -=  4; x = y; }
   y = x >>  2; if (y) { n -=  2; x = y; }
   y = x >>  1; if (y) { n -=  1; x = y; }
   return (int)(n - x);
#elif defined(_M_X64) || defined(_M_ARM64)
   unsigned long idx;
   if (_BitScanReverse64(&idx, x)) {
      return 63 - (int)idx;
   } else {
      return 64;
   }
#elif defined(_MSC_VER)
   unsigned long idx;
   if (_BitScanReverse(&idx, (FXPT128_U32)(x >> 32))) {
      return 31 - (int)idx;
   } else if (_BitScanReverse(&idx, (FXPT128_U32)x)) {
      return 63 - (int)idx;
   } else {
      return 64;
   }
#else
   return x ? __builtin_clzll(x) : 64;
#endif
}

#if !FXPT128_64BIT
// 32*32->64
static FXPT128_U64 r128__umul64(FXPT128_U32 a, FXPT128_U32 b)
{
#  if defined(_M_IX86) && !defined(FXPT128_STDC_ONLY) && !defined(__MINGW32__)
   return __emulu(a, b);
#  elif defined(_M_ARM) && !defined(FXPT128_STDC_ONLY)
   return _arm_umull(a, b);
#  else
   return a * (FXPT128_U64)b;
#  endif
}

// 64/32->32
static FXPT128_U32 r128__udiv64(FXPT128_U32 nlo, FXPT128_U32 nhi, FXPT128_U32 d, FXPT128_U32 *rem)
{
#  if defined(_M_IX86) && (_MSC_VER >= 1920) && !defined(FXPT128_STDC_ONLY)
   unsigned __int64 n = ((unsigned __int64)nhi << 32) | nlo;
   return _udiv64(n, d, rem);
#  elif defined(_M_IX86) && !defined(FXPT128_STDC_ONLY) && !defined(__MINGW32__)
   __asm {
      mov eax, nlo
      mov edx, nhi
      div d
      mov ecx, rem
      mov dword ptr [ecx], edx
   }
#  elif defined(__i386__) && !defined(FXPT128_STDC_ONLY)
   FXPT128_U32 q, r;
   __asm("divl %4"
      : "=a"(q), "=d"(r)
      : "a"(nlo), "d"(nhi), "X"(d));
   *rem = r;
   return q;
#  else
   FXPT128_U64 n64 = ((FXPT128_U64)nhi << 32) | nlo;
   *rem = (FXPT128_U32)(n64 % d);
   return (FXPT128_U32)(n64 / d);
#  endif
}
#elif defined(FXPT128_STDC_ONLY) || !FXPT128_INTEL
#define r128__umul64(a, b) ((a) * (FXPT128_U64)(b))
static FXPT128_U32 r128__udiv64(FXPT128_U32 nlo, FXPT128_U32 nhi, FXPT128_U32 d, FXPT128_U32 *rem)
{
   FXPT128_U64 n64 = ((FXPT128_U64)nhi << 32) | nlo;
   *rem = (FXPT128_U32)(n64 % d);
   return (FXPT128_U32)(n64 / d);
}
#endif   //!FXPT128_64BIT

static void r128__neg(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(src != NULL);

#if FXPT128_INTEL && !defined(FXPT128_STDC_ONLY)
   {
      unsigned char carry = 0;
#  if FXPT128_64BIT
      carry = _addcarry_u64(carry, ~src->lo, 1, &dst->lo);
      carry = _addcarry_u64(carry, ~src->hi, 0, &dst->hi);
#  else
      FXPT128_U32 r0, r1, r2, r3;
      carry = _addcarry_u32(carry, ~FXPT128_R0(src), 1, &r0);
      carry = _addcarry_u32(carry, ~FXPT128_R1(src), 0, &r1);
      carry = _addcarry_u32(carry, ~FXPT128_R2(src), 0, &r2);
      carry = _addcarry_u32(carry, ~FXPT128_R3(src), 0, &r3);
      FXPT128_SET4(dst, r0, r1, r2, r3);
#  endif //FXPT128_64BIT
   }
#else
   if (src->lo) {
      dst->lo = ~src->lo + 1;
      dst->hi = ~src->hi;
   } else {
      dst->lo = 0;
      dst->hi = ~src->hi + 1;
   }
#endif   //FXPT128_INTEL
}

// 64*64->128
static void r128__umul128(struct fmc_fxpt128_t *dst, FXPT128_U64 a, FXPT128_U64 b)
{
#if defined(_M_X64) && !defined(FXPT128_STDC_ONLY)
   dst->lo = _umul128(a, b, &dst->hi);
#elif FXPT128_64BIT && !defined(_MSC_VER) && !defined(FXPT128_STDC_ONLY)
   unsigned __int128 p0 = a * (unsigned __int128)b;
   dst->hi = (FXPT128_U64)(p0 >> 64);
   dst->lo = (FXPT128_U64)p0;
#else
   FXPT128_U32 alo = (FXPT128_U32)a;
   FXPT128_U32 ahi = (FXPT128_U32)(a >> 32);
   FXPT128_U32 blo = (FXPT128_U32)b;
   FXPT128_U32 bhi = (FXPT128_U32)(b >> 32);
   FXPT128_U64 p0, p1, p2, p3;

   p0 = r128__umul64(alo, blo);
   p1 = r128__umul64(alo, bhi);
   p2 = r128__umul64(ahi, blo);
   p3 = r128__umul64(ahi, bhi);

   {
#if FXPT128_INTEL && !defined(FXPT128_STDC_ONLY)
      FXPT128_U32 r0, r1, r2, r3;
      unsigned char carry;

      r0 = (FXPT128_U32)(p0);
      r1 = (FXPT128_U32)(p0 >> 32);
      r2 = (FXPT128_U32)(p1 >> 32);
      r3 = (FXPT128_U32)(p3 >> 32);

      carry = _addcarry_u32(0, r1, (FXPT128_U32)p1, &r1);
      carry = _addcarry_u32(carry, r2, (FXPT128_U32)(p2 >> 32), &r2);
      _addcarry_u32(carry, r3, 0, &r3);
      carry = _addcarry_u32(0, r1, (FXPT128_U32)p2, &r1);
      carry = _addcarry_u32(carry, r2, (FXPT128_U32)p3, &r2);
      _addcarry_u32(carry, r3, 0, &r3);

      FXPT128_SET4(dst, r0, r1, r2, r3);
#else
      FXPT128_U64 carry, lo, hi;
      carry = ((FXPT128_U64)(FXPT128_U32)p1 + (FXPT128_U64)(FXPT128_U32)p2 + (p0 >> 32)) >> 32;

      lo = p0 + ((p1 + p2) << 32);
      hi = p3 + ((FXPT128_U32)(p1 >> 32) + (FXPT128_U32)(p2 >> 32)) + carry;

      FXPT128_SET2(dst, lo, hi);
#endif
   }
#endif
}

// 128/64->64
#if defined(_M_X64) && (_MSC_VER < 1920) && !defined(FXPT128_STDC_ONLY) && !defined(__MINGW32__)
// MSVC x64 provides neither inline assembly nor (pre-2019) a div intrinsic, so we do fake
// "inline assembly" to avoid long division or outline assembly.
#pragma code_seg(".text")
__declspec(allocate(".text") align(16)) static const unsigned char r128__udiv128Code[] = {
   0x48, 0x8B, 0xC1,       //mov  rax, rcx
   0x49, 0xF7, 0xF0,       //div  rax, r8
   0x49, 0x89, 0x11,       //mov  qword ptr [r9], rdx
   0xC3                    //ret
};
typedef FXPT128_U64 (*r128__udiv128Proc)(FXPT128_U64 nlo, FXPT128_U64 nhi, FXPT128_U64 d, FXPT128_U64 *rem);
static const r128__udiv128Proc r128__udiv128 = (r128__udiv128Proc)(void*)r128__udiv128Code;
#else
static FXPT128_U64 r128__udiv128(FXPT128_U64 nlo, FXPT128_U64 nhi, FXPT128_U64 d, FXPT128_U64 *rem)
{
#if defined(_M_X64) && !defined(FXPT128_STDC_ONLY) && !defined(__MINGW32__)
   return _udiv128(nhi, nlo, d, rem);
#elif defined(__x86_64__) && !defined(FXPT128_STDC_ONLY)
   FXPT128_U64 q, r;
   __asm("divq %4"
      : "=a"(q), "=d"(r)
      : "a"(nlo), "d"(nhi), "X"(d));
   *rem = r;
   return q;
#else
   FXPT128_U64 tmp;
   FXPT128_U32 d0, d1;
   FXPT128_U32 n3, n2, n1, n0;
   FXPT128_U32 q0, q1;
   FXPT128_U32 r;
   int shift;

   FXPT128_ASSERT(d != 0);    //division by zero
   FXPT128_ASSERT(nhi < d);   //overflow

   // normalize
   shift = r128__clz64(d);

   if (shift) {
      struct fmc_fxpt128_t tmp128;
      FXPT128_SET2(&tmp128, nlo, nhi);
      r128Shl(&tmp128, &tmp128, shift);
      n3 = FXPT128_R3(&tmp128);
      n2 = FXPT128_R2(&tmp128);
      n1 = FXPT128_R1(&tmp128);
      n0 = FXPT128_R0(&tmp128);
      d <<= shift;
   } else {
      n3 = (FXPT128_U32)(nhi >> 32);
      n2 = (FXPT128_U32)nhi;
      n1 = (FXPT128_U32)(nlo >> 32);
      n0 = (FXPT128_U32)nlo;
   }

   d1 = (FXPT128_U32)(d >> 32);
   d0 = (FXPT128_U32)d;

   // first digit
   FXPT128_ASSERT(n3 <= d1);
   if (n3 < d1) {
      q1 = r128__udiv64(n2, n3, d1, &r);
   } else {
      q1 = 0xffffffffu;
      r = n2 + d1;
   }
refine1:
   if (r128__umul64(q1, d0) > ((FXPT128_U64)r << 32) + n1) {
      --q1;
      if (r < ~d1 + 1) {
         r += d1;
         goto refine1;
      }
   }

   tmp = ((FXPT128_U64)n2 << 32) + n1 - (r128__umul64(q1, d0) + (r128__umul64(q1, d1) << 32));
   n2 = (FXPT128_U32)(tmp >> 32);
   n1 = (FXPT128_U32)tmp;

   // second digit
   FXPT128_ASSERT(n2 <= d1);
   if (n2 < d1) {
      q0 = r128__udiv64(n1, n2, d1, &r);
   } else {
      q0 = 0xffffffffu;
      r = n1 + d1;
   }
refine0:
   if (r128__umul64(q0, d0) > ((FXPT128_U64)r << 32) + n0) {
      --q0;
      if (r < ~d1 + 1) {
         r += d1;
         goto refine0;
      }
   }

   tmp = ((FXPT128_U64)n1 << 32) + n0 - (r128__umul64(q0, d0) + (r128__umul64(q0, d1) << 32));
   n1 = (FXPT128_U32)(tmp >> 32);
   n0 = (FXPT128_U32)tmp;

   *rem = (((FXPT128_U64)n1 << 32) + n0) >> shift;
   return ((FXPT128_U64)q1 << 32) + q0;
#endif
}
#endif

static int r128__ucmp(const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   if (a->hi != b->hi) {
      if (a->hi > b->hi) {
         return 1;
      } else {
         return -1;
      }
   } else {
      if (a->lo == b->lo) {
         return 0;
      } else if (a->lo > b->lo) {
         return 1;
      } else {
         return -1;
      }
   }
}

static void r128__umul(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
#if defined(_M_X64) && !defined(FXPT128_STDC_ONLY)
   FXPT128_U64 t0, t1;
   FXPT128_U64 lo, hi = 0;
   unsigned char carry;

   t0 = _umul128(a->lo, b->lo, &t1);
   carry = _addcarry_u64(0, t1, t0 >> 63, &lo);
   _addcarry_u64(carry, hi, hi, &hi);

   t0 = _umul128(a->lo, b->hi, &t1);
   carry = _addcarry_u64(0, lo, t0, &lo);
   _addcarry_u64(carry, hi, t1, &hi);

   t0 = _umul128(a->hi, b->lo, &t1);
   carry = _addcarry_u64(0, lo, t0, &lo);
   _addcarry_u64(carry, hi, t1, &hi);

   t0 = _umul128(a->hi, b->hi, &t1);
   hi += t0;

   FXPT128_SET2(dst, lo, hi);
#elif defined(__x86_64__) && !defined(FXPT128_STDC_ONLY)
   unsigned __int128 p0, p1, p2, p3;
   p0 = a->lo * (unsigned __int128)b->lo;
   p1 = a->lo * (unsigned __int128)b->hi;
   p2 = a->hi * (unsigned __int128)b->lo;
   p3 = a->hi * (unsigned __int128)b->hi;

   p0 = (p3 << 64) + p2 + p1 + (p0 >> 64) + ((FXPT128_U64)p0 >> 63);
   dst->lo = (FXPT128_U64)p0;
   dst->hi = (FXPT128_U64)(p0 >> 64);
#else
   struct fmc_fxpt128_t p0, p1, p2, p3, round;

   r128__umul128(&p0, a->lo, b->lo);
   round.hi = 0; round.lo = p0.lo >> 63;
   p0.lo = p0.hi; p0.hi = 0; //r128Shr(&p0, &p0, 64);
   r128Add(&p0, &p0, &round);

   r128__umul128(&p1, a->hi, b->lo);
   r128Add(&p0, &p0, &p1);

   r128__umul128(&p2, a->lo, b->hi);
   r128Add(&p0, &p0, &p2);

   r128__umul128(&p3, a->hi, b->hi);
   p3.hi = p3.lo; p3.lo = 0; //r128Shl(&p3, &p3, 64);
   r128Add(&p0, &p0, &p3);

   FXPT128_SET2(dst, p0.lo, p0.hi);
#endif
}

// Shift d left until the high bit is set, and shift n left by the same amount.
// returns non-zero on overflow.
static int r128__norm(struct fmc_fxpt128_t *n, struct fmc_fxpt128_t *d, FXPT128_U64 *n2)
{
   FXPT128_U64 d0, d1;
   FXPT128_U64 n0, n1;
   int shift;

   d1 = d->hi;
   d0 = d->lo;
   n1 = n->hi;
   n0 = n->lo;

   if (d1) {
      shift = r128__clz64(d1);
      if (shift) {
         d1 = (d1 << shift) | (d0 >> (64 - shift));
         d0 = d0 << shift;
         *n2 = n1 >> (64 - shift);
         n1 = (n1 << shift) | (n0 >> (64 - shift));
         n0 = n0 << shift;
      } else {
         *n2 = 0;
      }
   } else {
      shift = r128__clz64(d0);
      if (r128__clz64(n1) <= shift) {
         return 1; // overflow
      }

      if (shift) {
         d1 = d0 << shift;
         d0 = 0;
         *n2 = (n1 << shift) | (n0 >> (64 - shift));
         n1 = n0 << shift;
         n0 = 0;
      } else {
         d1 = d0;
         d0 = 0;
         *n2 = n1;
         n1 = n0;
         n0 = 0;
      }
   }

   FXPT128_SET2(n, n0, n1);
   FXPT128_SET2(d, d0, d1);
   return 0;
}

static void r128__udiv(struct fmc_fxpt128_t *quotient, const struct fmc_fxpt128_t *dividend, const struct fmc_fxpt128_t *divisor)
{
   struct fmc_fxpt128_t tmp;
   FXPT128_U64 d0, d1;
   FXPT128_U64 n1, n2, n3;
   struct fmc_fxpt128_t q;

   FXPT128_ASSERT(dividend != NULL);
   FXPT128_ASSERT(divisor != NULL);
   FXPT128_ASSERT(quotient != NULL);
   FXPT128_ASSERT(divisor->hi != 0 || divisor->lo != 0);  // divide by zero

   // scale dividend and normalize
   {
      struct fmc_fxpt128_t n, d;
      FXPT128_SET2(&n, dividend->lo, dividend->hi);
      FXPT128_SET2(&d, divisor->lo, divisor->hi);
      if (r128__norm(&n, &d, &n3)) {
         FXPT128_SET2(quotient, FXPT128_max.lo, FXPT128_max.hi);
         return;
      }

      d1 = d.hi;
      d0 = d.lo;
      n2 = n.hi;
      n1 = n.lo;
   }

   // first digit
   FXPT128_ASSERT(n3 <= d1);
   {
      struct fmc_fxpt128_t t0, t1;
      t0.lo = n1;
      if (n3 < d1) {
         q.hi = r128__udiv128(n2, n3, d1, &t0.hi);
      } else {
         q.hi = FXPT128_LIT_U64(0xffffffffffffffff);
         t0.hi = n2 + d1;
      }

refine1:
      r128__umul128(&t1, q.hi, d0);
      if (r128__ucmp(&t1, &t0) > 0) {
         --q.hi;
         if (t0.hi < ~d1 + 1) {
            t0.hi += d1;
            goto refine1;
         }
      }
   }

   {
      struct fmc_fxpt128_t t0, t1, t2;
      t0.hi = n2;
      t0.lo = n1;

      r128__umul128(&t1, q.hi, d0);
      r128__umul128(&t2, q.hi, d1);

      t2.hi = t2.lo; t2.lo = 0;  //r128Shl(&t2, &t2, 64);
      r128Add(&tmp, &t1, &t2);
      r128Sub(&tmp, &t0, &tmp);
   }
   n2 = tmp.hi;
   n1 = tmp.lo;

   // second digit
   FXPT128_ASSERT(n2 <= d1);
   {
      struct fmc_fxpt128_t t0, t1;
      t0.lo = 0;
      if (n2 < d1) {
         q.lo = r128__udiv128(n1, n2, d1, &t0.hi);
      } else {
         q.lo = FXPT128_LIT_U64(0xffffffffffffffff);
         t0.hi = n1 + d1;
      }

   refine0:
      r128__umul128(&t1, q.lo, d0);
      if (r128__ucmp(&t1, &t0) > 0) {
         --q.lo;
         if (t0.hi < ~d1 + 1) {
            t0.hi += d1;
            goto refine0;
         }
      }
   }

   FXPT128_SET2(quotient, q.lo, q.hi);
}

static FXPT128_U64 r128__umod(struct fmc_fxpt128_t *n, struct fmc_fxpt128_t *d)
{
   FXPT128_U64 d0, d1;
   FXPT128_U64 n3, n2, n1;
   FXPT128_U64 q;

   FXPT128_ASSERT(d != NULL);
   FXPT128_ASSERT(n != NULL);
   FXPT128_ASSERT(d->hi != 0 || d->lo != 0);  // divide by zero

   if (r128__norm(n, d, &n3)) {
      return FXPT128_LIT_U64(0xffffffffffffffff);
   }

   d1 = d->hi;
   d0 = d->lo;
   n2 = n->hi;
   n1 = n->lo;

   FXPT128_ASSERT(n3 < d1);
   {
      struct fmc_fxpt128_t t0, t1;
      t0.lo = n1;
      q = r128__udiv128(n2, n3, d1, &t0.hi);

   refine1:
      r128__umul128(&t1, q, d0);
      if (r128__ucmp(&t1, &t0) > 0) {
         --q;
         if (t0.hi < ~d1 + 1) {
            t0.hi += d1;
            goto refine1;
         }
      }
   }

   return q;
}

static int r128__format(char *dst, size_t dstSize, const struct fmc_fxpt128_t *v, const R128ToStringFormat *format)
{
   char buf[128];
   struct fmc_fxpt128_t tmp;
   FXPT128_U64 whole;
   char *cursor, *decimal, *dstp = dst;
   int sign = 0;
   int fullPrecision = 1;
   int width, precision;
   int padCnt, trail = 0;

   FXPT128_ASSERT(dst != NULL && dstSize > 0);
   FXPT128_ASSERT(v != NULL);
   FXPT128_ASSERT(format != NULL);

   --dstSize;

   FXPT128_SET2(&tmp, v->lo, v->hi);
   if (r128IsNeg(&tmp)) {
      r128__neg(&tmp, &tmp);
      sign = 1;
   }

   width = format->width;
   if (width < 0) {
      width = 0;
   }

   precision = format->precision;
   if (precision < 0) {
      // print a maximum of 20 digits
      fullPrecision = 0;
      precision = 20;
   } else if (precision > sizeof(buf) - 21) {
      trail = precision - (sizeof(buf) - 21);
      precision -= trail;
   }

   whole = tmp.hi;
   decimal = cursor = buf;

   // fractional part first in case a carry into the whole part is required
   if (tmp.lo || format->decimal) {
      while (tmp.lo || (fullPrecision && precision)) {
         if ((int)(cursor - buf) == precision) {
            if ((FXPT128_S64)tmp.lo < 0) {
               // round up, propagate carry backwards
               char *c;
               for (c = cursor - 1; c >= buf; --c) {
                  char d = ++*c;
                  if (d <= '9') {
                     goto endfrac;
                  } else {
                     *c = '0';
                  }
               }

               // carry out into the whole part
               whole++;
            }

            break;
         }

         r128__umul128(&tmp, tmp.lo, 10);
         *cursor++ = (char)tmp.hi + '0';
      }

   endfrac:
      if (format->decimal || precision) {
         decimal = cursor;
         *cursor++ = FXPT128_decimal;
      }
   }

   // whole part
   do {
      char digit = (char)(whole % 10);
      whole /= 10;
      *cursor++ = digit + '0';
   } while (whole);

#define FXPT128__WRITE(c) do { if (dstp < dst + dstSize) *dstp = c; ++dstp; } while(0)

   padCnt = width - (int)(cursor - buf) - 1;

   // left padding
   if (!format->leftAlign) {
      char padChar = format->zeroPad ? '0' : ' ';
      if (format->zeroPad) {
         if (sign) {
            FXPT128__WRITE('-');
         } else if (format->sign == R128ToStringSign_Plus) {
            FXPT128__WRITE('+');
         } else if (format->sign == R128ToStringSign_Space) {
            FXPT128__WRITE(' ');
         } else {
            ++padCnt;
         }
      }

      for (; padCnt > 0; --padCnt) {
         FXPT128__WRITE(padChar);
      }
   }

   if (format->leftAlign || !format->zeroPad) {
      if (sign) {
         FXPT128__WRITE('-');
      } else if (format->sign == R128ToStringSign_Plus) {
         FXPT128__WRITE('+');
      } else if (format->sign == R128ToStringSign_Space) {
         FXPT128__WRITE(' ');
      } else {
         ++padCnt;
      }
   }

   {
      char *i;

      // reverse the whole part
      for (i = cursor - 1; i >= decimal; --i) {
         FXPT128__WRITE(*i);
      }

      // copy the fractional part
      for (i = buf; i < decimal; ++i) {
         FXPT128__WRITE(*i);
      }
   }

   // right padding
   if (format->leftAlign) {
      char padChar = format->zeroPad ? '0' : ' ';
      for (; padCnt > 0; --padCnt) {
         FXPT128__WRITE(padChar);
      }
   }

   // trailing zeroes for very large precision
   while (trail--) {
      FXPT128__WRITE('0');
   }

#undef FXPT128__WRITE

   if (dstp <= dst + dstSize) {
      *dstp = '\0';
   } else {
      dst[dstSize] = '\0';
   }
   return (int)(dstp - dst);
}

void r128FromInt(struct fmc_fxpt128_t *dst, FXPT128_S64 v)
{
   FXPT128_ASSERT(dst != NULL);
   dst->lo = 0;
   dst->hi = (FXPT128_U64)v;
   FXPT128_DEBUG_SET(dst);
}

void r128FromFloat(struct fmc_fxpt128_t *dst, double v)
{
   FXPT128_ASSERT(dst != NULL);

   if (v < -9223372036854775808.0) {
      r128Copy(dst, &FXPT128_min);
   } else if (v >= 9223372036854775808.0) {
      r128Copy(dst, &FXPT128_max);
   } else {
      struct fmc_fxpt128_t r;
      int sign = 0;

      if (v < 0) {
         v = -v;
         sign = 1;
      }

      r.hi = (FXPT128_U64)(FXPT128_S64)v;
      v -= (FXPT128_S64)v;
      r.lo = (FXPT128_U64)(v * 18446744073709551616.0);

      if (sign) {
         r128__neg(&r, &r);
      }

      r128Copy(dst, &r);
   }
}

void r128FromString(struct fmc_fxpt128_t *dst, const char *s, char **endptr)
{
   FXPT128_U64 lo = 0, hi = 0;
   FXPT128_U64 base = 10;

   int sign = 0;

   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(s != NULL);

   FXPT128_SET2(dst, 0, 0);

   // consume whitespace
   for (;;) {
      if (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n' || *s == '\v') {
         ++s;
      } else {
         break;
      }
   }

   // sign
   if (*s == '-') {
      sign = 1;
      ++s;
   } else if (*s == '+') {
      ++s;
   }

   // parse base prefix
   if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
      base = 16;
      s += 2;
   }

   // whole part
   for (;; ++s) {
      FXPT128_U64 digit;

      if ('0' <= *s && *s <= '9') {
         digit = *s - '0';
      } else if (base == 16 && 'a' <= *s && *s <= 'f') {
         digit = *s - 'a' + 10;
      } else if (base == 16 && 'A' <= *s && *s <= 'F') {
         digit = *s - 'A' + 10;
      } else {
         break;
      }

      hi = hi * base + digit;
   }

   // fractional part
   if (*s == FXPT128_decimal) {
      const char *exp = ++s;

      // find the last digit and work backwards
      for (;; ++s) {
         if ('0' <= *s && *s <= '9') {
         } else if (base == 16 && ('a' <= *s && *s <= 'f')) {
         } else if (base == 16 && ('A' <= *s && *s <= 'F')) {
         } else {
            break;
         }
      }

      for (const char *c = s - 1; c >= exp; --c) {
         FXPT128_U64 digit, unused;

         if ('0' <= *c && *c <= '9') {
            digit = *c - '0';
         } else if ('a' <= *c && *c <= 'f') {
            digit = *c - 'a' + 10;
         } else {
            digit = *c - 'A' + 10;
         }

         lo = r128__udiv128(lo, digit, base, &unused);
      }
   }

   FXPT128_SET2(dst, lo, hi);
   if (sign) {
      r128__neg(dst, dst);
   }

   if (endptr) {
      *endptr = (char *) s;
   }
}

FXPT128_S64 r128ToInt(const struct fmc_fxpt128_t *v)
{
   FXPT128_ASSERT(v != NULL);
   if ((FXPT128_S64)v->hi < 0) {
      return (FXPT128_S64)v->hi + (v->lo != 0);
   } else {
      return (FXPT128_S64)v->hi;
   }
}

double r128ToFloat(const struct fmc_fxpt128_t *v)
{
   struct fmc_fxpt128_t tmp;
   int sign = 0;
   double d;

   FXPT128_ASSERT(v != NULL);

   FXPT128_SET2(&tmp, v->lo, v->hi);
   if (r128IsNeg(&tmp)) {
      r128__neg(&tmp, &tmp);
      sign = 1;
   }

   d = tmp.hi + tmp.lo * (1 / 18446744073709551616.0);
   if (sign) {
      d = -d;
   }

   return d;
}

int r128ToStringOpt(char *dst, size_t dstSize, const struct fmc_fxpt128_t *v, const R128ToStringFormat *opt)
{
   return r128__format(dst, dstSize, v, opt);
}

int r128ToStringf(char *dst, size_t dstSize, const char *format, const struct fmc_fxpt128_t *v)
{
   R128ToStringFormat opts;

   FXPT128_ASSERT(dst != NULL && dstSize);
   FXPT128_ASSERT(format != NULL);
   FXPT128_ASSERT(v != NULL);

   opts.sign = FXPT128__defaultFormat.sign;
   opts.precision = FXPT128__defaultFormat.precision;
   opts.zeroPad = FXPT128__defaultFormat.zeroPad;
   opts.decimal = FXPT128__defaultFormat.decimal;
   opts.leftAlign = FXPT128__defaultFormat.leftAlign;

   if (*format == '%') {
      ++format;
   }

   // flags field
   for (;; ++format) {
      if (*format == ' ' && opts.sign != R128ToStringSign_Plus) {
         opts.sign = R128ToStringSign_Space;
      } else if (*format == '+') {
         opts.sign = R128ToStringSign_Plus;
      } else if (*format == '0') {
         opts.zeroPad = 1;
      } else if (*format == '-') {
         opts.leftAlign = 1;
      } else if (*format == '#') {
         opts.decimal = 1;
      } else {
         break;
      }
   }

   // width field
   opts.width = 0;
   for (;;) {
      if ('0' <= *format && *format <= '9') {
         opts.width = opts.width * 10 + *format++ - '0';
      } else {
         break;
      }
   }

   // precision field
   if (*format == '.') {
      opts.precision = 0;
      ++format;
      for (;;) {
         if ('0' <= *format && *format <= '9') {
            opts.precision = opts.precision * 10 + *format++ - '0';
         } else {
            break;
         }
      }
   }

   return r128__format(dst, dstSize, v, &opts);
}

int r128ToString(char *dst, size_t dstSize, const struct fmc_fxpt128_t *v)
{
   return r128__format(dst, dstSize, v, &FXPT128__defaultFormat);
}

void r128Copy(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(src != NULL);
   dst->lo = src->lo;
   dst->hi = src->hi;
   FXPT128_DEBUG_SET(dst);
}

void r128Neg(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v)
{
   r128__neg(dst, v);
   FXPT128_DEBUG_SET(dst);
}

void r128Abs(struct fmc_fxpt128_t* dst, const struct fmc_fxpt128_t* v)
{
    struct fmc_fxpt128_t sign, inv;

    FXPT128_ASSERT(dst != NULL);
    FXPT128_ASSERT(v != NULL);

    sign.lo = sign.hi = (FXPT128_U64)(((FXPT128_S64)v->hi) >> 63);
    inv.lo = v->lo ^ sign.lo;
    inv.hi = v->hi ^ sign.hi;

    r128Sub(dst, &inv, &sign);
}

void r128Nabs(struct fmc_fxpt128_t* dst, const struct fmc_fxpt128_t* v)
{
    struct fmc_fxpt128_t sign, inv;

    FXPT128_ASSERT(dst != NULL);
    FXPT128_ASSERT(v != NULL);

    sign.lo = sign.hi = (FXPT128_U64)(((FXPT128_S64)v->hi) >> 63);
    inv.lo = v->lo ^ sign.lo;
    inv.hi = v->hi ^ sign.hi;

    r128Sub(dst, &sign, &inv);
}

void r128Not(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(src != NULL);

   dst->lo = ~src->lo;
   dst->hi = ~src->hi;
   FXPT128_DEBUG_SET(dst);
}

void r128Or(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   dst->lo = a->lo | b->lo;
   dst->hi = a->hi | b->hi;
   FXPT128_DEBUG_SET(dst);
}

void r128And(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   dst->lo = a->lo & b->lo;
   dst->hi = a->hi & b->hi;
   FXPT128_DEBUG_SET(dst);
}

void r128Xor(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   dst->lo = a->lo ^ b->lo;
   dst->hi = a->hi ^ b->hi;
   FXPT128_DEBUG_SET(dst);
}

void r128Shl(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount)
{
   FXPT128_U64 r[4];

   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(src != NULL);

#if defined(_M_IX86) && !defined(FXPT128_STDC_ONLY) && !defined(__MINGW32__)
   __asm {
      // load src
      mov edx, dword ptr[src]
      mov ecx, amount

      mov edi, dword ptr[edx]
      mov esi, dword ptr[edx + 4]
      mov ebx, dword ptr[edx + 8]
      mov eax, dword ptr[edx + 12]

      // shift mod 32
      shld eax, ebx, cl
      shld ebx, esi, cl
      shld esi, edi, cl
      shl edi, cl

      // clear out low 12 bytes of stack
      xor edx, edx
      mov dword ptr[r], edx
      mov dword ptr[r + 4], edx
      mov dword ptr[r + 8], edx

      // store shifted amount offset by count/32 bits
      shr ecx, 5
      and ecx, 3
      mov dword ptr[r + ecx * 4 + 0], edi
      mov dword ptr[r + ecx * 4 + 4], esi
      mov dword ptr[r + ecx * 4 + 8], ebx
      mov dword ptr[r + ecx * 4 + 12], eax
   }
#else

   r[0] = src->lo;
   r[1] = src->hi;

   amount &= 127;
   if (amount >= 64) {
      r[1] = r[0] << (amount - 64);
      r[0] = 0;
   } else if (amount) {
#  if defined(_M_X64) && !defined(FXPT128_STDC_ONLY)
      r[1] = __shiftleft128(r[0], r[1], (char) amount);
#  else
      r[1] = (r[1] << amount) | (r[0] >> (64 - amount));
#  endif
      r[0] = r[0] << amount;
   }
#endif   //_M_IX86

   dst->lo = r[0];
   dst->hi = r[1];
   FXPT128_DEBUG_SET(dst);
}

void r128Shr(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount)
{
   FXPT128_U64 r[4];

   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(src != NULL);

#if defined(_M_IX86) && !defined(FXPT128_STDC_ONLY) && !defined(__MINGW32__)
   __asm {
      // load src
      mov edx, dword ptr[src]
      mov ecx, amount

      mov edi, dword ptr[edx]
      mov esi, dword ptr[edx + 4]
      mov ebx, dword ptr[edx + 8]
      mov eax, dword ptr[edx + 12]

      // shift mod 32
      shrd edi, esi, cl
      shrd esi, ebx, cl
      shrd ebx, eax, cl
      shr eax, cl

      // clear out high 12 bytes of stack
      xor edx, edx
      mov dword ptr[r + 20], edx
      mov dword ptr[r + 24], edx
      mov dword ptr[r + 28], edx

      // store shifted amount offset by -count/32 bits
      shr ecx, 5
      and ecx, 3
      neg ecx
      mov dword ptr[r + ecx * 4 + 16], edi
      mov dword ptr[r + ecx * 4 + 20], esi
      mov dword ptr[r + ecx * 4 + 24], ebx
      mov dword ptr[r + ecx * 4 + 28], eax
   }
#else
   r[2] = src->lo;
   r[3] = src->hi;

   amount &= 127;
   if (amount >= 64) {
      r[2] = r[3] >> (amount - 64);
      r[3] = 0;
   } else if (amount) {
#if defined(_M_X64) && !defined(FXPT128_STDC_ONLY)
      r[2] = __shiftright128(r[2], r[3], (char) amount);
#else
      r[2] = (r[2] >> amount) | (r[3] << (64 - amount));
#endif
      r[3] = r[3] >> amount;
   }
#endif

   dst->lo = r[2];
   dst->hi = r[3];
   FXPT128_DEBUG_SET(dst);
}

void r128Sar(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *src, int amount)
{
   FXPT128_U64 r[4];

   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(src != NULL);

#if defined(_M_IX86) && !defined(FXPT128_STDC_ONLY) && !defined(__MINGW32__)
   __asm {
      // load src
      mov edx, dword ptr[src]
      mov ecx, amount

      mov edi, dword ptr[edx]
      mov esi, dword ptr[edx + 4]
      mov ebx, dword ptr[edx + 8]
      mov eax, dword ptr[edx + 12]

      // shift mod 32
      shrd edi, esi, cl
      shrd esi, ebx, cl
      shrd ebx, eax, cl
      sar eax, cl

      // copy sign to high 12 bytes of stack
      cdq
      mov dword ptr[r + 20], edx
      mov dword ptr[r + 24], edx
      mov dword ptr[r + 28], edx

      // store shifted amount offset by -count/32 bits
      shr ecx, 5
      and ecx, 3
      neg ecx
      mov dword ptr[r + ecx * 4 + 16], edi
      mov dword ptr[r + ecx * 4 + 20], esi
      mov dword ptr[r + ecx * 4 + 24], ebx
      mov dword ptr[r + ecx * 4 + 28], eax
   }
#else
   r[2] = src->lo;
   r[3] = src->hi;

   amount &= 127;
   if (amount >= 64) {
      r[2] = (FXPT128_U64)((FXPT128_S64)r[3] >> (amount - 64));
      r[3] = (FXPT128_U64)((FXPT128_S64)r[3] >> 63);
   } else if (amount) {
      r[2] = (r[2] >> amount) | (FXPT128_U64)((FXPT128_S64)r[3] << (64 - amount));
      r[3] = (FXPT128_U64)((FXPT128_S64)r[3] >> amount);
   }
#endif

   dst->lo = r[2];
   dst->hi = r[3];
   FXPT128_DEBUG_SET(dst);
}

void r128Add(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   unsigned char carry = 0;
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

#if FXPT128_INTEL && !defined(FXPT128_STDC_ONLY)
#  if FXPT128_64BIT
   carry = _addcarry_u64(carry, a->lo, b->lo, &dst->lo);
   carry = _addcarry_u64(carry, a->hi, b->hi, &dst->hi);
#  else
   FXPT128_U32 r0, r1, r2, r3;
   carry = _addcarry_u32(carry, FXPT128_R0(a), FXPT128_R0(b), &r0);
   carry = _addcarry_u32(carry, FXPT128_R1(a), FXPT128_R1(b), &r1);
   carry = _addcarry_u32(carry, FXPT128_R2(a), FXPT128_R2(b), &r2);
   carry = _addcarry_u32(carry, FXPT128_R3(a), FXPT128_R3(b), &r3);
   FXPT128_SET4(dst, r0, r1, r2, r3);
#  endif //FXPT128_64BIT
#else
   {
      FXPT128_U64 r = a->lo + b->lo;
      carry = r < a->lo;
      dst->lo = r;
      dst->hi = a->hi + b->hi + carry;
   }
#endif   //FXPT128_INTEL

   FXPT128_DEBUG_SET(dst);
}

void r128Sub(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   unsigned char borrow = 0;
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

#if FXPT128_INTEL && !defined(FXPT128_STDC_ONLY)
#  if FXPT128_64BIT
   borrow = _subborrow_u64(borrow, a->lo, b->lo, &dst->lo);
   borrow = _subborrow_u64(borrow, a->hi, b->hi, &dst->hi);
#  else
   FXPT128_U32 r0, r1, r2, r3;
   borrow = _subborrow_u32(borrow, FXPT128_R0(a), FXPT128_R0(b), &r0);
   borrow = _subborrow_u32(borrow, FXPT128_R1(a), FXPT128_R1(b), &r1);
   borrow = _subborrow_u32(borrow, FXPT128_R2(a), FXPT128_R2(b), &r2);
   borrow = _subborrow_u32(borrow, FXPT128_R3(a), FXPT128_R3(b), &r3);
   FXPT128_SET4(dst, r0, r1, r2, r3);
#  endif //FXPT128_64BIT
#else
   {
      FXPT128_U64 r = a->lo - b->lo;
      borrow = r > a->lo;
      dst->lo = r;
      dst->hi = a->hi - b->hi - borrow;
   }
#endif   //FXPT128_INTEL

   FXPT128_DEBUG_SET(dst);
}

void r128Mul(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   int sign = 0;
   struct fmc_fxpt128_t ta, tb, tc;

   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   FXPT128_SET2(&ta, a->lo, a->hi);
   FXPT128_SET2(&tb, b->lo, b->hi);

   if (r128IsNeg(&ta)) {
      r128__neg(&ta, &ta);
      sign = !sign;
   }
   if (r128IsNeg(&tb)) {
      r128__neg(&tb, &tb);
      sign = !sign;
   }

   r128__umul(&tc, &ta, &tb);
   if (sign) {
      r128__neg(&tc, &tc);
   }

   r128Copy(dst, &tc);
}

void r128Div(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   int sign = 0;
   struct fmc_fxpt128_t tn, td, tq;

   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   FXPT128_SET2(&tn, a->lo, a->hi);
   FXPT128_SET2(&td, b->lo, b->hi);

   if (r128IsNeg(&tn)) {
      r128__neg(&tn, &tn);
      sign = !sign;
   }

   if (td.lo == 0 && td.hi == 0) {
      // divide by zero
      if (sign) {
         r128Copy(dst, &FXPT128_min);
      } else {
         r128Copy(dst, &FXPT128_max);
      }
      return;
   } else if (r128IsNeg(&td)) {
      r128__neg(&td, &td);
      sign = !sign;
   }

   r128__udiv(&tq, &tn, &td);

   if (sign) {
      r128__neg(&tq, &tq);
   }

   r128Copy(dst, &tq);
}

void r128Mod(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   int sign = 0;
   struct fmc_fxpt128_t tn, td, tq;

   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   FXPT128_SET2(&tn, a->lo, a->hi);
   FXPT128_SET2(&td, b->lo, b->hi);

   if (r128IsNeg(&tn)) {
      r128__neg(&tn, &tn);
      sign = !sign;
   }

   if (td.lo == 0 && td.hi == 0) {
      // divide by zero
      if (sign) {
         r128Copy(dst, &FXPT128_min);
      } else {
         r128Copy(dst, &FXPT128_max);
      }
      return;
   } else if (r128IsNeg(&td)) {
      r128__neg(&td, &td);
      sign = !sign;
   }

   tq.hi = r128__umod(&tn, &td);
   tq.lo = 0;

   if (sign) {
      tq.hi = ~tq.hi + 1;
   }

   r128Mul(&tq, &tq, b);
   r128Sub(dst, a, &tq);
}

void r128Rsqrt(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v)
{
   static const struct fmc_fxpt128_t threeHalves = { FXPT128_LIT_U64(0x8000000000000000), 1 };
   struct fmc_fxpt128_t x, est;
   int i;

   if ((FXPT128_S64)v->hi < 0) {
      r128Copy(dst, &FXPT128_min);
      return;
   }

   FXPT128_SET2(&x, v->lo, v->hi);

   // get initial estimate
   if (x.hi) {
      int shift = (64 + r128__clz64(x.hi)) >> 1;
      est.lo = FXPT128_LIT_U64(1) << shift;
      est.hi = 0;
   } else if (x.lo) {
      int shift = r128__clz64(x.lo) >> 1;
      est.hi = FXPT128_LIT_U64(1) << shift;
      est.lo = 0;
   } else {
      FXPT128_SET2(dst, 0, 0);
      return;
   }

   // x /= 2
   r128Shr(&x, &x, 1);

   // Newton-Raphson iterate
   for (i = 0; i < 7; ++i) {
      struct fmc_fxpt128_t newEst;

      // newEst = est * (threeHalves - (x / 2) * est * est);
      r128__umul(&newEst, &est, &est);
      r128__umul(&newEst, &newEst, &x);
      r128Sub(&newEst, &threeHalves, &newEst);
      r128__umul(&newEst, &est, &newEst);

      if (newEst.lo == est.lo && newEst.hi == est.hi) {
         break;
      }
      FXPT128_SET2(&est, newEst.lo, newEst.hi);
   }

   r128Copy(dst, &est);
}

void r128Sqrt(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v)
{
   struct fmc_fxpt128_t x, est;
   int i;

   if ((FXPT128_S64)v->hi < 0) {
      r128Copy(dst, &FXPT128_min);
      return;
   }

   FXPT128_SET2(&x, v->lo, v->hi);

   // get initial estimate
   if (x.hi) {
      int shift = (63 - r128__clz64(x.hi)) >> 1;
      r128Shr(&est, &x, shift);
   } else if (x.lo) {
      int shift = (1 + r128__clz64(x.lo)) >> 1;
      r128Shl(&est, &x, shift);
   } else {
      FXPT128_SET2(dst, 0, 0);
      return;
   }

   // Newton-Raphson iterate
   for (i = 0; i < 7; ++i) {
      struct fmc_fxpt128_t newEst;

      // newEst = (est + x / est) / 2
      r128__udiv(&newEst, &x, &est);
      r128Add(&newEst, &newEst, &est);
      r128Shr(&newEst, &newEst, 1);

      if (newEst.lo == est.lo && newEst.hi == est.hi) {
         break;
      }
      FXPT128_SET2(&est, newEst.lo, newEst.hi);
   }

   r128Copy(dst, &est);
}

int r128Cmp(const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   if (a->hi == b->hi) {
      if (a->lo == b->lo) {
         return 0;
      } else if (a->lo > b->lo) {
         return 1;
      } else {
         return -1;
      }
   } else if ((FXPT128_S64)a->hi > (FXPT128_S64)b->hi) {
      return 1;
   } else {
      return -1;
   }
}

int r128IsNeg(const struct fmc_fxpt128_t *v)
{
   FXPT128_ASSERT(v != NULL);

   return (FXPT128_S64)v->hi < 0;
}

void r128Min(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   if (r128Cmp(a, b) < 0) {
      r128Copy(dst, a);
   } else {
      r128Copy(dst, b);
   }
}

void r128Max(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *a, const struct fmc_fxpt128_t *b)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(a != NULL);
   FXPT128_ASSERT(b != NULL);

   if (r128Cmp(a, b) > 0) {
      r128Copy(dst, a);
   } else {
      r128Copy(dst, b);
   }
}

void r128Floor(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(v != NULL);

   dst->hi = v->hi;
   dst->lo = 0;
   FXPT128_DEBUG_SET(dst);
}

void r128Ceil(struct fmc_fxpt128_t *dst, const struct fmc_fxpt128_t *v)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(v != NULL);

   dst->hi = v->hi + (v->lo != 0);
   dst->lo = 0;
   FXPT128_DEBUG_SET(dst);
}

void r128Round(struct fmc_fxpt128_t* dst, const struct fmc_fxpt128_t* v)
{
   FXPT128_ASSERT(dst != NULL);
   FXPT128_ASSERT(v != NULL);

   dst->hi = v->hi + (v->lo >= FXPT128_LIT_U64(0x8000000000000000) + (FXPT128_U64)((FXPT128_S64)v->hi < 0));
   dst->lo = 0;
   FXPT128_DEBUG_SET(dst);
}

#endif   //FXPT128_IMPLEMENTATION
