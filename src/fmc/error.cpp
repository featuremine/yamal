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
 * @file error.cpp
 * @author Federico Ravchina
 * @date 10 May 2021
 * @brief File contains C implementation of fmc error handling
 *
 * This file contains C implementation of fmc error handling.
 * @see http://www.featuremine.com
 */

#include <fmc/platform.h>

#if defined(FMC_SYS_WIN)
#include <windows.h>
#endif

#include <fmc/error.h>

#include <cerrno>
#include <cstdarg>
#include <cstdio>  // vsnprintf
#include <cstring> //memcpy
#include <string>

const char *error_msgs[] = {"None", "Could not allocate memory"};

void fmc_error_init(fmc_error_t *err, FMC_ERROR_CODE code, const char *buf) {
  err->code = code;
  err->buf = NULL;
  if (code == FMC_ERROR_CUSTOM) {
    buf = buf ? buf : "UNKNOWN";
    err->buf = (char *)calloc(strlen(buf) + 1, sizeof(*buf));
    strcpy(err->buf, buf);
  }
}

void fmc_error_reset(fmc_error_t *err, FMC_ERROR_CODE code, const char *buf) {
  fmc_error_destroy(err);
  fmc_error_init(err, code, buf);
}

void fmc_error_init_mov(fmc_error_t *err, fmc_error_t *from) {
  err->code = from->code;
  err->buf = from->buf;
  from->code = FMC_ERROR_NONE;
  from->buf = NULL;
}

void fmc_error_init_none(fmc_error_t *err) {
  fmc_error_init(err, FMC_ERROR_NONE, NULL);
}

void fmc_error_reset_none(fmc_error_t *err) {
  fmc_error_reset(err, FMC_ERROR_NONE, NULL);
}

void fmc_error_init_sprintf(fmc_error_t *err, const char *fmt, ...) {
  FMC_ERROR_FORMAT(err, fmt);
}

void fmc_error_reset_sprintf(fmc_error_t *err, const char *fmt, ...) {
  fmc_error_destroy(err);
  FMC_ERROR_FORMAT(err, fmt);
}

void fmc_error_mov(fmc_error_t *err1, fmc_error_t *err2) {
  fmc_error_destroy(err1);
  fmc_error_init_mov(err1, err2);
}

void fmc_error_set(fmc_error_t **err_ptr, const char *fmt, ...) {
  fmc_error_t res;
  FMC_ERROR_FORMAT(&res, fmt);
  fmc_error_t *err = fmc_error_inst();
  fmc_error_mov(err, &res);
  fmc_error_destroy(&res);
  *err_ptr = err;
}

void fmc_error_set2(fmc_error_t **err_ptr, FMC_ERROR_CODE code) {
  fmc_error_t *err = fmc_error_inst();
  fmc_error_destroy(err);
  fmc_error_init(err, code, NULL);
  *err_ptr = err;
}

void fmc_error_destroy(fmc_error_t *err) {
  err->code = FMC_ERROR_NONE;
  if (err->buf) {
    free(err->buf);
    err->buf = NULL;
  }
}

void fmc_error_clear(fmc_error_t **err) { *err = NULL; }

const char *fmc_error_msg(fmc_error_t *err) {
  return err->code == FMC_ERROR_CUSTOM ? err->buf : error_msgs[err->code];
}

void fmc_error_cpy(fmc_error_t *err1, fmc_error_t *err2) {
  fmc_error_destroy(err1);
  fmc_error_init(err1, err2->code, err2->buf);
}

void fmc_error_init_join(fmc_error_t *res, fmc_error_t *err1, fmc_error_t *err2,
                         const char *sep) {
  fmc_error_init_sprintf(
      res, "%s%s%s", err1->code != FMC_ERROR_NONE ? fmc_error_msg(err1) : "",
      err1->code != FMC_ERROR_NONE && sep ? sep : "",
      err2->code != FMC_ERROR_NONE ? fmc_error_msg(err2) : "");
}

void fmc_error_cat(fmc_error_t *err1, fmc_error_t *err2, const char *sep) {
  fmc_error_t res;
  fmc_error_init_join(&res, err1, err2, sep);
  fmc_error_cpy(err1, &res);
  fmc_error_destroy(&res);
}

bool fmc_error_has(fmc_error_t *err) { return err->code != FMC_ERROR_NONE; }

struct fmc_error_wrap {
  fmc_error_wrap() { fmc_error_init_none(&error); }
  ~fmc_error_wrap() { fmc_error_destroy(&error); }
  fmc_error error;
};

fmc_error_t *fmc_error_inst() {
  static thread_local fmc_error_wrap wrap;
  return &wrap.error;
}

const char *fmc_syserror_msg() {
#if defined(FMC_SYS_WIN)
  static thread_local std::string msg_buffer;
  // From
  // https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf, 0, NULL);
  msg_buffer = (const char *)lpMsgBuf;
  LocalFree(lpMsgBuf);
  return msg_buffer.c_str();
#else
  return strerror(errno);
#endif
}
