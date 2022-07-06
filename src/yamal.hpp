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
 * @file yamal.hpp
 * @author Featuremine Corporation
 * @date 29 Apr 2021
 * @brief File contains C declaration of the mmlist
 *
 * This file contains declarations of the memory mapped list.
 * @see http://www.featuremine.com
 */

#ifndef __FM_YTP_YAMAL_HPP__
#define __FM_YTP_YAMAL_HPP__

#include <stdbool.h>
#include <apr_pools.h> // apr_pool_t
#include <apr_file_io.h> // apr_file_t
#include <apr_mmap.h> // apr_mmap_t

#include <condition_variable>
#include <mutex>
#include <thread>
#include <ytp/yamal.h>

struct ytp_yamal {
  apr_file_t * f;
  std::condition_variable cv_;
  std::mutex m_;
  std::mutex pa_mutex_;
  std::thread thread_;
  bool done_ = false;
  bool readonly_ = false;
  apr_mmap_t *pages[YAMAL_PAGES] = {0};
  apr_pool_t *pool_;
};

#endif // __FM_YTP_YAMAL_HPP__
