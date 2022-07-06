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

#include <iostream>
#include <chrono>
#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_mmap.h> // apr_mmap_t
#include <ytp/yamal.h>
#include <sys/mman.h> // msync()
#include "utils.hpp"

#if defined(__linux__) || defined(DARWIN)
#include <time.h> // clock_gettime() timespec CLOCK_REALTIME
#endif

int64_t cur_time_ns(void) {
#if defined(__linux__) || defined(DARWIN)
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(ts.tv_sec)) +
          std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::nanoseconds(ts.tv_nsec)))
      .count();
#else
  return duration_cast<chrono::nanoseconds>(
             system_clock::now().time_since_epoch())
      .count();
#endif
}

void print_percentile(std::ostream &os, log_bucket &buckets, double percentile) {
  os << percentile << "% Percentile: " << buckets.percentile(percentile)
            << " nanoseconds" << std::endl;
}

ytp_status_t mmap_sync(apr_mmap_t *map, apr_size_t size) {
  ytp_status_t rv = YTP_STATUS_OK;
#if defined(WIN32)
  if (!FlushViewOfFile(map->mm, 0)) {
    return YTP_STATUS_EMEM;
  } // Async flush of dirty pages
  if (!FlushFileBuffers(map->mv)) {
    return YTP_STATUS_EMEM;
  } // flush metadata and wait
#elif defined(__linux__) || defined(DARWIN)
  if (msync(map->mm, size, MS_ASYNC) != 0) {
    return errno;
  }
#endif
  return rv;
}