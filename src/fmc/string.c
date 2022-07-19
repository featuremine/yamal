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
#include <fmc/string.h>
#include <fmc/error.h>
#include <string.h>


char *fmc_cstr_new(const char*str, fmc_error_t **error) {
  size_t sz = strlen(str) + 1;
  char *s = (char *)calloc(sz, sizeof(*s));
  if(!s) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return NULL;
  }
  memcpy(s, str, sz);
  return s;
}