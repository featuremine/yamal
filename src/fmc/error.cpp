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
#include <cstdio>
#include <cstring>
#include <string>

struct fmc_error {
  std::string msg;
  std::string buffer;
};

const char *fmc_error_msg(fmc_error_t *err) { return err->msg.c_str(); }

void fmc_error_clear(fmc_error_t **err) { *err = NULL; }

void fmc_error_set(fmc_error_t **err_ptr, const char *fmt, ...) {
  fmc_error_t *err = fmc_error_inst();
  *err_ptr = err;

  va_list args1;
  va_start(args1, fmt);
  va_list args2;
  va_copy(args2, args1);
  auto size = vsnprintf(NULL, 0, fmt, args1) + 1;
  err->buffer.swap(err->msg);
  err->msg.resize(size);
  va_end(args1);
  vsnprintf(err->msg.data(), size, fmt, args2);
  va_end(args2);
}

fmc_error_t *fmc_error_inst() {
  static thread_local fmc_error_t err;
  return &err;
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
