/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file error.h
 * @author Federico Ravchina
 * @date 10 May 2021
 * @brief File contains C declaration of fmc error handling
 *
 * This file contains C declaration of fmc error handling.
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  FMC_ERROR_NONE,
  FMC_ERROR_MEMORY,
  FMC_ERROR_CUSTOM,
  FMC_ERROR_CLOSED
} FMC_ERROR_CODE;

struct fmc_error {
  FMC_ERROR_CODE code;
  char *buf;
};

typedef struct fmc_error fmc_error_t;

/**
 * @brief Initializes the error struct
 *
 * User must provide a buf if code is FMC_ERROR_CUSTOM
 *
 * @param err
 * @param code FMC_ERROR_CODE
 * @param buf error string. Could be NULL if code is not FMC_ERROR_CUSTOM
 */
FMMODFUNC void fmc_error_init(fmc_error_t *err, FMC_ERROR_CODE code,
                              const char *buf);

/**
 * @brief Destroys and initializes the error struct
 *
 * User must provide a buf if code is FMC_ERROR_CUSTOM
 *
 * @param err
 * @param code FMC_ERROR_CODE
 * @param buf error string. Could be NULL if code is not FMC_ERROR_CUSTOM
 */
FMMODFUNC void fmc_error_reset(fmc_error_t *err, FMC_ERROR_CODE code,
                               const char *buf);

/**
 * @brief Movesone error into another
 *
 * @param err destination error
 * @param from source error
 */
FMMODFUNC void fmc_error_init_mov(fmc_error_t *err, fmc_error_t *from);

/**
 * @brief Initializes the error struct with error code FMC_ERROR_NONE
 *
 * @param err
 */
FMMODFUNC void fmc_error_init_none(fmc_error_t *err);

/**
 * @brief Destroys and initializes the error struct with error code
 * FMC_ERROR_NONE
 *
 * @param err
 */
FMMODFUNC void fmc_error_reset_none(fmc_error_t *err);

/**
 * @brief Inistializes the error with code FMC_ERROR_CUSTOM and an error
 * message.
 *
 * @param err
 * @param fmt error format string
 * @param ... depending on the format string, a sequence of additional arguments
 * is expected
 */
FMMODFUNC void fmc_error_init_sprintf(fmc_error_t *err, const char *fmt, ...);

/**
 * @brief Destroys and inistializes the error with code FMC_ERROR_CUSTOM and
 * an error message.
 *
 * @param err
 * @param fmt error format string
 * @param ... depending on the format string, a sequence of additional arguments
 * is expected
 */
FMMODFUNC void fmc_error_reset_sprintf(fmc_error_t *err, const char *fmt, ...);

/**
 * @brief Destroy the error struct
 * @param err
 */
FMMODFUNC void fmc_error_destroy(fmc_error_t *err);

/**
 * @brief Clears the error pointer
 * @param err
 */
FMMODFUNC void fmc_error_clear(fmc_error_t **err);

/**
 * @brief Returns the message associated with the error object
 *
 * @param err
 * @return a C-string error message
 */
FMMODFUNC const char *fmc_error_msg(const fmc_error_t *err);

/**
 * @brief Copies one error into another
 *
 * @param errdest destination error
 * @param errsrc source error
 */
FMMODFUNC void fmc_error_cpy(fmc_error_t *errdest, const fmc_error_t *errsrc);

/**
 * @brief Moves one error into another
 *
 * @param err1 destination error
 * @param err2 source error
 */
FMMODFUNC void fmc_error_mov(fmc_error_t *err1, fmc_error_t *err2);

/**
 * @brief Initializes an error by joining 2 existing ones.
 * The error code is FMC_ERROR_CUSTOM.
 *
 * @param errdest destination error
 * @param errsrc1 source error 1: Its error goes first in the final string
 * @param errsrc2 source error 2: Its error goes last in the final string
 * @param sep separator in between the two error strings
 */
FMMODFUNC void fmc_error_init_join(fmc_error_t *errdest,
                                   const fmc_error_t *errsrc1,
                                   const fmc_error_t *errsrc2, const char *sep);

/**
 * @brief Concatenates the error string into another.
 * The error code becomes FMC_ERROR_CUSTOM
 *
 * @param errdest destination error
 * @param errsrc source error: Its error goes last in the final string
 * @param sep separator in between the two error strings
 */
FMMODFUNC void fmc_error_cat(fmc_error_t *errdest, const fmc_error_t *errsrc,
                             const char *sep);

/**
 * @brief Returns true if it has an error (i.e. is different to FMC_ERROR_NONE)
 *
 * @param err
 */
FMMODFUNC bool fmc_error_has(const fmc_error_t *err);

/**
 * @brief Returns the current thread error object
 * @return the current thread error object
 */
FMMODFUNC fmc_error_t *fmc_error_inst();

/**
 * @brief Returns the last system error message
 *
 * @return a C-string error message
 */
FMMODFUNC const char *fmc_syserror_msg();

/* Functions and macros below use the global fmc_error_t error instance*/

#define FMC_ERROR_REPORT(err, msg)                                             \
  fmc_error_set(err, "%s (%s:%d)", msg, __FILE__, __LINE__)

/**
 * @brief Set an error message and assigns a pointer to the error
 *
 * @param err_ptr
 * @param fmt error format string
 * @param ... depending on the format string, a sequence of additional arguments
 * is expected
 */
FMMODFUNC void fmc_error_set(fmc_error_t **err_ptr, const char *fmt, ...);

/**
 * @brief Set an error code and assigns a pointer to the error
 *
 * @param err_ptr
 * @param code FMC_ERROR_CODE
 */
FMMODFUNC void fmc_error_set2(fmc_error_t **err_ptr, FMC_ERROR_CODE code);

/**
 * @brief Macro to populate error with parameter expansion
 *
 * @param err error pointer
 * @param fmt format string
 */
#define FMC_ERROR_FORMAT(err, fmt)                                             \
  do {                                                                         \
    va_list _args1;                                                            \
    va_start(_args1, fmt);                                                     \
    va_list _args2;                                                            \
    va_copy(_args2, _args1);                                                   \
    int _size = vsnprintf(NULL, 0, fmt, _args1) + 1;                           \
    char _buf[_size];                                                          \
    va_end(_args1);                                                            \
    vsnprintf(_buf, _size, fmt, _args2);                                       \
    va_end(_args2);                                                            \
    fmc_error_init(err, FMC_ERROR_CUSTOM, _buf);                               \
  } while (0)

#ifdef __cplusplus
}
#endif
