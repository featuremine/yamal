/*
The MIT License (MIT)

Copyright (c) 2017 Charles Gunyon

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef CMP_H__
#define CMP_H__

#include <fmc/platform.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct cmp_ctx_s;

typedef bool (*cmp_reader)(struct cmp_ctx_s *ctx, void *data, size_t limit);
typedef bool (*cmp_skipper)(struct cmp_ctx_s *ctx, size_t count);
typedef size_t (*cmp_writer)(struct cmp_ctx_s *ctx, const void *data,
                             size_t count);

enum {
  CMP_TYPE_POSITIVE_FIXNUM, /*  0 */
  CMP_TYPE_FIXMAP,          /*  1 */
  CMP_TYPE_FIXARRAY,        /*  2 */
  CMP_TYPE_FIXSTR,          /*  3 */
  CMP_TYPE_NIL,             /*  4 */
  CMP_TYPE_BOOLEAN,         /*  5 */
  CMP_TYPE_BIN8,            /*  6 */
  CMP_TYPE_BIN16,           /*  7 */
  CMP_TYPE_BIN32,           /*  8 */
  CMP_TYPE_EXT8,            /*  9 */
  CMP_TYPE_EXT16,           /* 10 */
  CMP_TYPE_EXT32,           /* 11 */
  CMP_TYPE_FLOAT,           /* 12 */
  CMP_TYPE_DOUBLE,          /* 13 */
  CMP_TYPE_UINT8,           /* 14 */
  CMP_TYPE_UINT16,          /* 15 */
  CMP_TYPE_UINT32,          /* 16 */
  CMP_TYPE_UINT64,          /* 17 */
  CMP_TYPE_SINT8,           /* 18 */
  CMP_TYPE_SINT16,          /* 19 */
  CMP_TYPE_SINT32,          /* 20 */
  CMP_TYPE_SINT64,          /* 21 */
  CMP_TYPE_FIXEXT1,         /* 22 */
  CMP_TYPE_FIXEXT2,         /* 23 */
  CMP_TYPE_FIXEXT4,         /* 24 */
  CMP_TYPE_FIXEXT8,         /* 25 */
  CMP_TYPE_FIXEXT16,        /* 26 */
  CMP_TYPE_STR8,            /* 27 */
  CMP_TYPE_STR16,           /* 28 */
  CMP_TYPE_STR32,           /* 29 */
  CMP_TYPE_ARRAY16,         /* 30 */
  CMP_TYPE_ARRAY32,         /* 31 */
  CMP_TYPE_MAP16,           /* 32 */
  CMP_TYPE_MAP32,           /* 33 */
  CMP_TYPE_NEGATIVE_FIXNUM  /* 34 */
};

typedef struct cmp_ext_s {
  int8_t type;
  uint32_t size;
} cmp_ext_t;

union cmp_object_data_u {
  bool boolean;
  uint8_t u8;
  uint16_t u16;
  uint32_t u32;
  uint64_t u64;
  int8_t s8;
  int16_t s16;
  int32_t s32;
  int64_t s64;
  float flt;
  double dbl;
  uint32_t array_size;
  uint32_t map_size;
  uint32_t str_size;
  uint32_t bin_size;
  cmp_ext_t ext;
};

enum {
  ERROR_NONE,
  STR_DATA_LENGTH_TOO_LONG_ERROR,
  BIN_DATA_LENGTH_TOO_LONG_ERROR,
  ARRAY_LENGTH_TOO_LONG_ERROR,
  MAP_LENGTH_TOO_LONG_ERROR,
  INPUT_VALUE_TOO_LARGE_ERROR,
  FIXED_VALUE_WRITING_ERROR,
  TYPE_MARKER_READING_ERROR,
  TYPE_MARKER_WRITING_ERROR,
  DATA_READING_ERROR,
  DATA_WRITING_ERROR,
  EXT_TYPE_READING_ERROR,
  EXT_TYPE_WRITING_ERROR,
  INVALID_TYPE_ERROR,
  LENGTH_READING_ERROR,
  LENGTH_WRITING_ERROR,
  SKIP_DEPTH_LIMIT_EXCEEDED_ERROR,
  INTERNAL_ERROR,
  ERROR_MAX
};

typedef struct cmp_ctx_s {
  uint8_t error;
  void *buf;
  cmp_reader read;
  cmp_skipper skip;
  cmp_writer write;
} cmp_ctx_t;

typedef struct cmp_object_s {
  uint8_t type;
  union cmp_object_data_u as;
} cmp_object_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 * === Main API
 * ============================================================================
 */

/*
 * Initializes a CMP context
 *
 * If you don't intend to read, `read` may be NULL, but calling `*read*`
 * functions will crash; there is no check.
 *
 * `skip` may be NULL, in which case skipping functions will use `read`.
 *
 * If you don't intend to write, `write` may be NULL, but calling `*write*`
 * functions will crash; there is no check.
 */
FMMODFUNC void cmp_init(cmp_ctx_t *ctx, void *buf, cmp_reader read,
                        cmp_skipper skip, cmp_writer write);

/* Returns CMP's version */
FMMODFUNC uint32_t cmp_version(void);

/* Returns the MessagePack version employed by CMP */
FMMODFUNC uint32_t cmp_mp_version(void);

/* Returns a string description of a CMP context's error */
FMMODFUNC const char *cmp_strerror(cmp_ctx_t *ctx);

/* Writes a signed integer to the backend */
FMMODFUNC bool cmp_write_integer(cmp_ctx_t *ctx, int64_t d);

/* Writes an unsigned integer to the backend */
FMMODFUNC bool cmp_write_uinteger(cmp_ctx_t *ctx, uint64_t u);

/*
 * Writes a floating-point value (either single or double-precision) to the
 * backend
 */
FMMODFUNC bool cmp_write_decimal(cmp_ctx_t *ctx, double d);

/* Writes NULL to the backend */
FMMODFUNC bool cmp_write_nil(cmp_ctx_t *ctx);

/* Writes true to the backend */
FMMODFUNC bool cmp_write_true(cmp_ctx_t *ctx);

/* Writes false to the backend */
FMMODFUNC bool cmp_write_false(cmp_ctx_t *ctx);

/* Writes a boolean value to the backend */
FMMODFUNC bool cmp_write_bool(cmp_ctx_t *ctx, bool b);

/*
 * Writes an unsigned char's value to the backend as a boolean.  This is useful
 * if you are using a different boolean type in your application.
 */
FMMODFUNC bool cmp_write_u8_as_bool(cmp_ctx_t *ctx, uint8_t b);

/*
 * Writes a string to the backend; according to the MessagePack spec, this must
 * be encoded using UTF-8, but CMP leaves that job up to the programmer.
 */
FMMODFUNC bool cmp_write_str(cmp_ctx_t *ctx, const char *data, uint32_t size);

/*
 * Writes a string to the backend.  This avoids using the STR8 marker, which
 * is unsupported by MessagePack v4, the version implemented by many other
 * MessagePack libraries.  No encoding is assumed in this case, not that it
 * matters.
 */
FMMODFUNC bool cmp_write_str_v4(cmp_ctx_t *ctx, const char *data,
                                uint32_t size);

/*
 * Writes the string marker to the backend.  This is useful if you are writing
 * data in chunks instead of a single shot.
 */
FMMODFUNC bool cmp_write_str_marker(cmp_ctx_t *ctx, uint32_t size);

/*
 * Writes the string marker to the backend.  This is useful if you are writing
 * data in chunks instead of a single shot.  This avoids using the STR8
 * marker, which is unsupported by MessagePack v4, the version implemented by
 * many other MessagePack libraries.  No encoding is assumed in this case, not
 * that it matters.
 */
FMMODFUNC bool cmp_write_str_marker_v4(cmp_ctx_t *ctx, uint32_t size);

/* Writes binary data to the backend */
FMMODFUNC bool cmp_write_bin(cmp_ctx_t *ctx, const void *data, uint32_t size);

/*
 * Writes the binary data marker to the backend.  This is useful if you are
 * writing data in chunks instead of a single shot.
 */
FMMODFUNC bool cmp_write_bin_marker(cmp_ctx_t *ctx, uint32_t size);

/* Writes an array to the backend. */
FMMODFUNC bool cmp_write_array(cmp_ctx_t *ctx, uint32_t size);

/* Writes a map to the backend. */
FMMODFUNC bool cmp_write_map(cmp_ctx_t *ctx, uint32_t size);

/* Writes an extended type to the backend */
FMMODFUNC bool cmp_write_ext(cmp_ctx_t *ctx, int8_t type, uint32_t size,
                             const void *data);

/*
 * Writes the extended type marker to the backend.  This is useful if you want
 * to write the type's data in chunks instead of a single shot.
 */
FMMODFUNC bool cmp_write_ext_marker(cmp_ctx_t *ctx, int8_t type, uint32_t size);

/* Writes an object to the backend */
FMMODFUNC bool cmp_write_object(cmp_ctx_t *ctx, cmp_object_t *obj);

/*
 * Writes an object to the backend. This avoids using the STR8 marker, which
 * is unsupported by MessagePack v4, the version implemented by many other
 * MessagePack libraries.
 */
FMMODFUNC bool cmp_write_object_v4(cmp_ctx_t *ctx, cmp_object_t *obj);

/* Reads a signed integer that fits inside a signed char */
FMMODFUNC bool cmp_read_char(cmp_ctx_t *ctx, int8_t *c);

/* Reads a signed integer that fits inside a signed short */
FMMODFUNC bool cmp_read_short(cmp_ctx_t *ctx, int16_t *s);

/* Reads a signed integer that fits inside a signed int */
FMMODFUNC bool cmp_read_int(cmp_ctx_t *ctx, int32_t *i);

/* Reads a signed integer that fits inside a signed long */
FMMODFUNC bool cmp_read_long(cmp_ctx_t *ctx, int64_t *d);

/* Reads a signed integer */
FMMODFUNC bool cmp_read_integer(cmp_ctx_t *ctx, int64_t *d);

/* Reads an unsigned integer that fits inside an unsigned char */
FMMODFUNC bool cmp_read_uchar(cmp_ctx_t *ctx, uint8_t *c);

/* Reads an unsigned integer that fits inside an unsigned short */
FMMODFUNC bool cmp_read_ushort(cmp_ctx_t *ctx, uint16_t *s);

/* Reads an unsigned integer that fits inside an unsigned int */
FMMODFUNC bool cmp_read_uint(cmp_ctx_t *ctx, uint32_t *i);

/* Reads an unsigned integer that fits inside an unsigned long */
FMMODFUNC bool cmp_read_ulong(cmp_ctx_t *ctx, uint64_t *u);

/* Reads an unsigned integer */
FMMODFUNC bool cmp_read_uinteger(cmp_ctx_t *ctx, uint64_t *u);

/*
 * Reads a floating point value (either single or double-precision) from the
 * backend
 */
FMMODFUNC bool cmp_read_decimal(cmp_ctx_t *ctx, double *d);

/* "Reads" (more like "skips") a NULL value from the backend */
FMMODFUNC bool cmp_read_nil(cmp_ctx_t *ctx);

/* Reads a boolean from the backend */
FMMODFUNC bool cmp_read_bool(cmp_ctx_t *ctx, bool *b);

/*
 * Reads a boolean as an unsigned char from the backend; this is useful if your
 * application uses a different boolean type.
 */
FMMODFUNC bool cmp_read_bool_as_u8(cmp_ctx_t *ctx, uint8_t *b);

/* Reads a string's size from the backend */
FMMODFUNC bool cmp_read_str_size(cmp_ctx_t *ctx, uint32_t *size);

/*
 * Reads a string from the backend; according to the spec, the string's data
 * ought to be encoded using UTF-8,
 */
FMMODFUNC bool cmp_read_str(cmp_ctx_t *ctx, char *data, uint32_t *size);

/* Reads the size of packed binary data from the backend */
FMMODFUNC bool cmp_read_bin_size(cmp_ctx_t *ctx, uint32_t *size);

/* Reads packed binary data from the backend */
FMMODFUNC bool cmp_read_bin(cmp_ctx_t *ctx, void *data, uint32_t *size);

/* Reads an array from the backend */
FMMODFUNC bool cmp_read_array(cmp_ctx_t *ctx, uint32_t *size);

/* Reads a map from the backend */
FMMODFUNC bool cmp_read_map(cmp_ctx_t *ctx, uint32_t *size);

/* Reads the extended type's marker from the backend */
FMMODFUNC bool cmp_read_ext_marker(cmp_ctx_t *ctx, int8_t *type,
                                   uint32_t *size);

/* Reads an extended type from the backend */
FMMODFUNC bool cmp_read_ext(cmp_ctx_t *ctx, int8_t *type, uint32_t *size,
                            void *data);

/* Reads an object from the backend */
FMMODFUNC bool cmp_read_object(cmp_ctx_t *ctx, cmp_object_t *obj);

/*
 * Skips the next object from the backend.  If that object is an array or map,
 * this function will:
 *   - If `obj` is not `NULL`, fill in `obj` with that object
 *   - Set `ctx->error` to `SKIP_DEPTH_LIMIT_EXCEEDED_ERROR`
 *   - Return `false`
 * Otherwise:
 *   - (Don't touch `obj`)
 *   - Return `true`
 */
FMMODFUNC bool cmp_skip_object(cmp_ctx_t *ctx, cmp_object_t *obj);

/*
 * This is similar to `cmp_skip_object`, except it tolerates up to `limit`
 * levels of nesting.  For example, in order to skip an array that contains a
 * map, call `cmp_skip_object_limit(ctx, &obj, 2)`.  Or in other words,
 * `cmp_skip_object(ctx, &obj)` acts similarly to `cmp_skip_object_limit(ctx,
 * &obj, 0)`
 *
 * Specifically, `limit` refers to depth, not breadth.  So in order to skip an
 * array that contains two arrays that each contain 3 strings, you would call
 * `cmp_skip_object_limit(ctx, &obj, 2).  In order to skip an array that
 * contains 4 arrays that each contain 1 string, you would still call
 * `cmp_skip_object_limit(ctx, &obj, 2).
 */
FMMODFUNC bool cmp_skip_object_limit(cmp_ctx_t *ctx, cmp_object_t *obj,
                                     uint32_t limit);

/*
 * This is similar to `cmp_skip_object`, except it will continually skip
 * nested data structures.
 *
 * WARNING: This can cause your application to spend an unbounded amount of
 *          time reading nested data structures.  Unless you completely trust
 *          the data source, you should strongly consider `cmp_skip_object` or
 *          `cmp_skip_object_limit`.
 */
FMMODFUNC bool cmp_skip_object_no_limit(cmp_ctx_t *ctx);

/*
 * ============================================================================
 * === Specific API
 * ============================================================================
 */

FMMODFUNC bool cmp_write_pfix(cmp_ctx_t *ctx, uint8_t c);
FMMODFUNC bool cmp_write_nfix(cmp_ctx_t *ctx, int8_t c);

FMMODFUNC bool cmp_write_sfix(cmp_ctx_t *ctx, int8_t c);
FMMODFUNC bool cmp_write_s8(cmp_ctx_t *ctx, int8_t c);
FMMODFUNC bool cmp_write_s16(cmp_ctx_t *ctx, int16_t s);
FMMODFUNC bool cmp_write_s32(cmp_ctx_t *ctx, int32_t i);
FMMODFUNC bool cmp_write_s64(cmp_ctx_t *ctx, int64_t l);

FMMODFUNC bool cmp_write_ufix(cmp_ctx_t *ctx, uint8_t c);
FMMODFUNC bool cmp_write_u8(cmp_ctx_t *ctx, uint8_t c);
FMMODFUNC bool cmp_write_u16(cmp_ctx_t *ctx, uint16_t s);
FMMODFUNC bool cmp_write_u32(cmp_ctx_t *ctx, uint32_t i);
FMMODFUNC bool cmp_write_u64(cmp_ctx_t *ctx, uint64_t l);

FMMODFUNC bool cmp_write_float(cmp_ctx_t *ctx, float f);
FMMODFUNC bool cmp_write_double(cmp_ctx_t *ctx, double d);

FMMODFUNC bool cmp_write_fixstr_marker(cmp_ctx_t *ctx, uint8_t size);
FMMODFUNC bool cmp_write_fixstr(cmp_ctx_t *ctx, const char *data, uint8_t size);
FMMODFUNC bool cmp_write_str8_marker(cmp_ctx_t *ctx, uint8_t size);
FMMODFUNC bool cmp_write_str8(cmp_ctx_t *ctx, const char *data, uint8_t size);
FMMODFUNC bool cmp_write_str16_marker(cmp_ctx_t *ctx, uint16_t size);
FMMODFUNC bool cmp_write_str16(cmp_ctx_t *ctx, const char *data, uint16_t size);
FMMODFUNC bool cmp_write_str32_marker(cmp_ctx_t *ctx, uint32_t size);
FMMODFUNC bool cmp_write_str32(cmp_ctx_t *ctx, const char *data, uint32_t size);

FMMODFUNC bool cmp_write_bin8_marker(cmp_ctx_t *ctx, uint8_t size);
FMMODFUNC bool cmp_write_bin8(cmp_ctx_t *ctx, const void *data, uint8_t size);
FMMODFUNC bool cmp_write_bin16_marker(cmp_ctx_t *ctx, uint16_t size);
FMMODFUNC bool cmp_write_bin16(cmp_ctx_t *ctx, const void *data, uint16_t size);
FMMODFUNC bool cmp_write_bin32_marker(cmp_ctx_t *ctx, uint32_t size);
FMMODFUNC bool cmp_write_bin32(cmp_ctx_t *ctx, const void *data, uint32_t size);

FMMODFUNC bool cmp_write_fixarray(cmp_ctx_t *ctx, uint8_t size);
FMMODFUNC bool cmp_write_array16(cmp_ctx_t *ctx, uint16_t size);
FMMODFUNC bool cmp_write_array32(cmp_ctx_t *ctx, uint32_t size);

FMMODFUNC bool cmp_write_fixmap(cmp_ctx_t *ctx, uint8_t size);
FMMODFUNC bool cmp_write_map16(cmp_ctx_t *ctx, uint16_t size);
FMMODFUNC bool cmp_write_map32(cmp_ctx_t *ctx, uint32_t size);

FMMODFUNC bool cmp_write_fixext1_marker(cmp_ctx_t *ctx, int8_t type);
FMMODFUNC bool cmp_write_fixext1(cmp_ctx_t *ctx, int8_t type, const void *data);
FMMODFUNC bool cmp_write_fixext2_marker(cmp_ctx_t *ctx, int8_t type);
FMMODFUNC bool cmp_write_fixext2(cmp_ctx_t *ctx, int8_t type, const void *data);
FMMODFUNC bool cmp_write_fixext4_marker(cmp_ctx_t *ctx, int8_t type);
FMMODFUNC bool cmp_write_fixext4(cmp_ctx_t *ctx, int8_t type, const void *data);
FMMODFUNC bool cmp_write_fixext8_marker(cmp_ctx_t *ctx, int8_t type);
FMMODFUNC bool cmp_write_fixext8(cmp_ctx_t *ctx, int8_t type, const void *data);
FMMODFUNC bool cmp_write_fixext16_marker(cmp_ctx_t *ctx, int8_t type);
FMMODFUNC bool cmp_write_fixext16(cmp_ctx_t *ctx, int8_t type,
                                  const void *data);

FMMODFUNC bool cmp_write_ext8_marker(cmp_ctx_t *ctx, int8_t type, uint8_t size);
FMMODFUNC bool cmp_write_ext8(cmp_ctx_t *ctx, int8_t type, uint8_t size,
                              const void *data);
FMMODFUNC bool cmp_write_ext16_marker(cmp_ctx_t *ctx, int8_t type,
                                      uint16_t size);
FMMODFUNC bool cmp_write_ext16(cmp_ctx_t *ctx, int8_t type, uint16_t size,
                               const void *data);
FMMODFUNC bool cmp_write_ext32_marker(cmp_ctx_t *ctx, int8_t type,
                                      uint32_t size);
FMMODFUNC bool cmp_write_ext32(cmp_ctx_t *ctx, int8_t type, uint32_t size,
                               const void *data);

FMMODFUNC bool cmp_read_pfix(cmp_ctx_t *ctx, uint8_t *c);
FMMODFUNC bool cmp_read_nfix(cmp_ctx_t *ctx, int8_t *c);

FMMODFUNC bool cmp_read_sfix(cmp_ctx_t *ctx, int8_t *c);
FMMODFUNC bool cmp_read_s8(cmp_ctx_t *ctx, int8_t *c);
FMMODFUNC bool cmp_read_s16(cmp_ctx_t *ctx, int16_t *s);
FMMODFUNC bool cmp_read_s32(cmp_ctx_t *ctx, int32_t *i);
FMMODFUNC bool cmp_read_s64(cmp_ctx_t *ctx, int64_t *l);

FMMODFUNC bool cmp_read_ufix(cmp_ctx_t *ctx, uint8_t *c);
FMMODFUNC bool cmp_read_u8(cmp_ctx_t *ctx, uint8_t *c);
FMMODFUNC bool cmp_read_u16(cmp_ctx_t *ctx, uint16_t *s);
FMMODFUNC bool cmp_read_u32(cmp_ctx_t *ctx, uint32_t *i);
FMMODFUNC bool cmp_read_u64(cmp_ctx_t *ctx, uint64_t *l);

FMMODFUNC bool cmp_read_float(cmp_ctx_t *ctx, float *f);
FMMODFUNC bool cmp_read_double(cmp_ctx_t *ctx, double *d);

FMMODFUNC bool cmp_read_fixext1_marker(cmp_ctx_t *ctx, int8_t *type);
FMMODFUNC bool cmp_read_fixext1(cmp_ctx_t *ctx, int8_t *type, void *data);
FMMODFUNC bool cmp_read_fixext2_marker(cmp_ctx_t *ctx, int8_t *type);
FMMODFUNC bool cmp_read_fixext2(cmp_ctx_t *ctx, int8_t *type, void *data);
FMMODFUNC bool cmp_read_fixext4_marker(cmp_ctx_t *ctx, int8_t *type);
FMMODFUNC bool cmp_read_fixext4(cmp_ctx_t *ctx, int8_t *type, void *data);
FMMODFUNC bool cmp_read_fixext8_marker(cmp_ctx_t *ctx, int8_t *type);
FMMODFUNC bool cmp_read_fixext8(cmp_ctx_t *ctx, int8_t *type, void *data);
FMMODFUNC bool cmp_read_fixext16_marker(cmp_ctx_t *ctx, int8_t *type);
FMMODFUNC bool cmp_read_fixext16(cmp_ctx_t *ctx, int8_t *type, void *data);

FMMODFUNC bool cmp_read_ext8_marker(cmp_ctx_t *ctx, int8_t *type,
                                    uint8_t *size);
FMMODFUNC bool cmp_read_ext8(cmp_ctx_t *ctx, int8_t *type, uint8_t *size,
                             void *data);
FMMODFUNC bool cmp_read_ext16_marker(cmp_ctx_t *ctx, int8_t *type,
                                     uint16_t *size);
FMMODFUNC bool cmp_read_ext16(cmp_ctx_t *ctx, int8_t *type, uint16_t *size,
                              void *data);
FMMODFUNC bool cmp_read_ext32_marker(cmp_ctx_t *ctx, int8_t *type,
                                     uint32_t *size);
FMMODFUNC bool cmp_read_ext32(cmp_ctx_t *ctx, int8_t *type, uint32_t *size,
                              void *data);

/*
 * ============================================================================
 * === Object API
 * ============================================================================
 */

FMMODFUNC bool cmp_object_is_char(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_short(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_int(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_long(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_sinteger(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_uchar(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_ushort(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_uint(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_ulong(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_uinteger(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_float(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_double(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_nil(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_bool(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_str(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_bin(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_array(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_map(cmp_object_t *obj);
FMMODFUNC bool cmp_object_is_ext(cmp_object_t *obj);

FMMODFUNC bool cmp_object_as_char(cmp_object_t *obj, int8_t *c);
FMMODFUNC bool cmp_object_as_short(cmp_object_t *obj, int16_t *s);
FMMODFUNC bool cmp_object_as_int(cmp_object_t *obj, int32_t *i);
FMMODFUNC bool cmp_object_as_long(cmp_object_t *obj, int64_t *d);
FMMODFUNC bool cmp_object_as_sinteger(cmp_object_t *obj, int64_t *d);
FMMODFUNC bool cmp_object_as_uchar(cmp_object_t *obj, uint8_t *c);
FMMODFUNC bool cmp_object_as_ushort(cmp_object_t *obj, uint16_t *s);
FMMODFUNC bool cmp_object_as_uint(cmp_object_t *obj, uint32_t *i);
FMMODFUNC bool cmp_object_as_ulong(cmp_object_t *obj, uint64_t *u);
FMMODFUNC bool cmp_object_as_uinteger(cmp_object_t *obj, uint64_t *u);
FMMODFUNC bool cmp_object_as_float(cmp_object_t *obj, float *f);
FMMODFUNC bool cmp_object_as_double(cmp_object_t *obj, double *d);
FMMODFUNC bool cmp_object_as_bool(cmp_object_t *obj, bool *b);
FMMODFUNC bool cmp_object_as_str(cmp_object_t *obj, uint32_t *size);
FMMODFUNC bool cmp_object_as_bin(cmp_object_t *obj, uint32_t *size);
FMMODFUNC bool cmp_object_as_array(cmp_object_t *obj, uint32_t *size);
FMMODFUNC bool cmp_object_as_map(cmp_object_t *obj, uint32_t *size);
FMMODFUNC bool cmp_object_as_ext(cmp_object_t *obj, int8_t *type,
                                 uint32_t *size);

FMMODFUNC bool cmp_object_to_str(cmp_ctx_t *ctx, cmp_object_t *obj, char *data,
                                 uint32_t buf_size);
FMMODFUNC bool cmp_object_to_bin(cmp_ctx_t *ctx, cmp_object_t *obj, void *data,
                                 uint32_t buf_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*
 * ============================================================================
 * === Backwards compatibility defines
 * ============================================================================
 */

#define cmp_write_int cmp_write_integer
#define cmp_write_sint cmp_write_integer
#define cmp_write_sinteger cmp_write_integer
#define cmp_write_uint cmp_write_uinteger
#define cmp_read_sinteger cmp_read_integer

/* vi: set et ts=2 sw=2: */

#ifdef __cplusplus
}
#endif

#endif /* CMP_H__ */
