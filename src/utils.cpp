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

#include <sstream>
#include <string>

#if defined(__linux__) || defined(DARWIN)
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
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

_tid _tid_cur() {
#if defined(__linux__) || defined(DARWIN)
  return pthread_self();
#elif defined(WIN32)
  return GetCurrentThread();
#else
#error "Unsupported operating system"
#endif
}

ytp_status_t _set_sched_normal(_tid tid) {
#if defined(__linux__) || defined(DARWIN)
  const struct sched_param normal_sp = {.sched_priority = 0};
  int r = pthread_setschedparam(tid, SCHED_OTHER, &normal_sp);
  if (r) {
    errno = r;
    return errno;
  }
  return YTP_STATUS_OK;
#else
#error "Unsupported operating system"
#endif
}

ytp_status_t _set_sched_fifo(_tid tid, int priority) {
#if defined(__linux__) || defined(DARWIN)
  const struct sched_param fifo_sp = {.sched_priority = priority};
  int r = pthread_setschedparam(tid, SCHED_FIFO, &fifo_sp);
  if (r) {
    errno = r;
    return errno;
  }
  return YTP_STATUS_OK;
#else
#error "Unsupported operating system"
#endif
}

ytp_status_t _set_affinity(_tid tid, int cpuid) {
#if defined(__linux__)
  cpu_set_t my_set;
  CPU_ZERO(&my_set);
  CPU_SET(cpuid, &my_set);
  int r = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &my_set);
  if (r) {
    errno = r;
    return errno;
  }
  return YTP_STATUS_OK;
#elif defined(DARWIN)
#elif defined(WIN32)
  if (!SetProcessAffinityMask(GetCurrentProcess(), 1 << cpuid)) {
    return APR_EPROC_UNKNOWN;
  }

  if (!SetThreadPriority(tid, THREAD_PRIORITY_HIGHEST)) {
    return APR_EPROC_UNKNOWN;
  }
  return YTP_STATUS_OK;
#else
#error "Unsupported operating system"
#endif
}

ytp_status_t _set_cur_affinity(int cpuid) {
  auto tid = _tid_cur();
  return _set_affinity(tid, cpuid);
}