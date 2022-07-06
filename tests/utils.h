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
 * @file utils.h
 * @author Featuremine Corporation
 * @date 16 May 2022
 * @brief Yamal test utils
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_UTILS_H__
#define __FM_YTP_UTILS_H__

#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_strings.h> // apr_pstrcat()
#include <apr_pools.h> // apr_pool_t
#include <apr_file_io.h> // apr_temp_dir_get() apr_file_mktemp()
#include <ytp/yamal.h>
#include "gtestwrap.hpp"



void test_init(apr_pool_t **pool) {
  ytp_status_t rv = ytp_initialize(); // also calls apr_pool_initialize()
  ASSERT_EQ(rv, APR_SUCCESS);

  rv = apr_pool_create(pool, NULL);
  ASSERT_EQ(rv, APR_SUCCESS);
  ASSERT_NE(*pool, nullptr);
}

void test_end(apr_file_t *f, apr_pool_t *pool) {
  ytp_status_t rv = apr_file_close(f);
  ASSERT_EQ(rv, APR_SUCCESS);
  apr_pool_destroy(pool);
  ytp_terminate();
}

void make_temp_file(apr_file_t **f, apr_pool_t *pool) {
  const char *tempdir = NULL;
  char *filetemplate;

  ytp_status_t rv = apr_temp_dir_get(&tempdir, pool);
  ASSERT_EQ(rv, APR_SUCCESS);
  
  filetemplate = apr_pstrcat(pool, tempdir, "/tempfileXXXXXX", NULL);
  rv = apr_file_mktemp(f, filetemplate, 0, pool);
  ASSERT_EQ(rv, APR_SUCCESS);
}

#endif // __FM_YTP_UTILS_H__