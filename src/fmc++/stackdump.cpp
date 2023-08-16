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
 * @file stackdump.cpp
 * @author Maxim Trokhimtchouk
 * @date 18 Dec 2017
 * @brief File contains stack dump utility functions
 *
 * This file contains implementation stack dump
 */

#include <fmc++/stackdump.hpp>
#include <fmc/platform.h>

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

using namespace std;

#if defined(FMC_SYS_LINUX)
#include <cxxabi.h>
#include <elfutils/libdwfl.h>
#include <libunwind.h>

ostream &debug_info(ostream &is, const void *ip) {

  char *debuginfo_path = NULL;

  Dwfl_Callbacks callbacks = {
      dwfl_linux_proc_find_elf,
      dwfl_standard_find_debuginfo,
      NULL,
      &debuginfo_path,
  };

  Dwfl *dwfl = dwfl_begin(&callbacks);
  if (!dwfl) {
    is << "[dwfl_begin error] " << dwfl_errmsg(-1);
    return is;
  }

  if (dwfl_linux_proc_report(dwfl, getpid()) != 0) {
    is << "[dwfl_linux_proc_report error] " << dwfl_errmsg(-1);
    dwfl_end(dwfl);
    return is;
  }
  if (dwfl_report_end(dwfl, NULL, NULL) != 0) {
    is << "[dwfl_report_end error] " << dwfl_errmsg(-1);
    dwfl_end(dwfl);
    return is;
  }

  Dwarf_Addr addr = (uintptr_t)ip;

  Dwfl_Module *module = dwfl_addrmodule(dwfl, addr);

  const char *function_name = dwfl_module_addrname(module, addr);

  int status;
  char *realname = abi::__cxa_demangle(function_name, 0, 0, &status);
  if (realname) {
    is << realname;
    free(realname);
  } else {
    is << function_name;
  }
  is << " (";

  Dwfl_Line *line = dwfl_getsrc(dwfl, addr);
  if (line != NULL) {
    int nline;
    Dwarf_Addr addr;
    const char *filename = dwfl_lineinfo(line, &addr, &nline, NULL, NULL, NULL);
    is << strrchr(filename, '/') + 1 << ":" << nline;
  } else {
    is << ip;
  }

  dwfl_end(dwfl);
  return is;
}

__attribute__((noinline)) ostream &stack_dump(ostream &is, int skip) {
  unw_context_t uc;
  unw_getcontext(&uc);

  unw_cursor_t cursor;
  unw_init_local(&cursor, &uc);

  while (unw_step(&cursor) > 0) {

    unw_word_t ip;
    unw_get_reg(&cursor, UNW_REG_IP, &ip);

    unw_word_t offset;
    char name[1024];
    if (int err = unw_get_proc_name(&cursor, name, sizeof(name), &offset);
        err != 0) {
      is << "cannot determine stack: " << unw_strerror(err);
    }

    if (skip <= 0) {
      is << "\tat ";
      debug_info(is, (void *)(ip - 4));
      is << ")\n";
    }

    if (strcmp(name, "main") == 0)
      break;

    skip--;
  }
  return is;
}

#elif defined(FMC_SYS_MACH)
__attribute__((noinline)) ostream &stack_dump(ostream &is, int skip) {
  return is;
}
#else
#error "os is not supported"
#endif
