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
 * @file string.c
 * @date 19 Jul 2022
 * @brief File contains C implementation of fmc string API
 *
 * This file contains C implementation of fmc string API.
 * @see http://www.featuremine.com
 */
#include <fmc/error.h>
#include <fmc/string.h>
#include <stdlib.h> // calloc()
#include <string.h> // memcpy() strlen()

char *fmc_cstr_new2(const char *str, size_t sz, fmc_error_t **error) {
  char *s = (char *)calloc(sz + 1, sizeof(*s));
  if (!s) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return NULL;
  }
  memcpy(s, str, sz);
  return s;
}

char *fmc_cstr_new(const char *str, fmc_error_t **error) {
  return fmc_cstr_new2(str, strlen(str), error);
}

size_t fmc_cstr_biparse(const char *targ, const char *str1, const char *str2) {
  size_t s = 0;
  for (;*str1 != '\0'; s++, targ++, str1++, str2++) {
    if (*targ != *str1 && *targ != *str2) {
      return 0;
    }
  }
  return s;
}
