/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file process.cpp
 * @author Alejandro Farfan
 * @date 17 Jun 2021
 * @brief File contains affinity and priority utilities
 *
 * @see http://www.featuremine.com
 */

#include <fmc/platform.h>
#include <fmc/process.h>
#include <sstream>
#include <string>

#if defined(FMC_SYS_UNIX)
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

pid_t fmc_exec(const char *cmd, fmc_error_t **err) {
  fmc_error_clear(err);
  pid_t cpid;

  if ((cpid = fork()) == -1) {
    fmc_error_set(err, "could not fork: %s", strerror(errno));
    return -1;
  }

  if (cpid == 0) {
    execl("/bin/sh", "/bin/sh", "-c", cmd, (char *)NULL);
    fprintf(stderr, "failed to execute %s with error: %s", cmd,
            strerror(errno));
    _exit(EXIT_FAILURE);
  }

  return cpid;
}

int fmc_waitpid(pid_t pid, fmc_error_t **err) {
  fmc_error_clear(err);
  int status;
  if (waitpid(pid, &status, 0) < 0) {
    fmc_error_set(err, "failed to wait for the process %d with error: %s", pid,
                  strerror(errno));
    return -1;
  }
  return status;
}

fmc_tid fmc_tid_cur(fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_UNIX)
  return pthread_self();
#elif defined(FMC_SYS_WIN)
  return GetCurrentThread();
#else
#error "Unsupported operating system"
#endif
}

void fmc_set_sched_normal(fmc_tid tid, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_UNIX)
  const struct sched_param normal_sp = {.sched_priority = 0};
  int r = pthread_setschedparam(tid, SCHED_OTHER, &normal_sp);
  if (r) {
    errno = r;
    FMC_ERROR_REPORT(error, strerror(errno));
  }
#else
#error "Unsupported operating system"
#endif
}

void fmc_set_sched_fifo(fmc_tid tid, int priority, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_UNIX)
  const struct sched_param fifo_sp = {.sched_priority = priority};
  int r = pthread_setschedparam(tid, SCHED_FIFO, &fifo_sp);
  if (r) {
    errno = r;
    FMC_ERROR_REPORT(error, strerror(errno));
  }
#else
#error "Unsupported operating system"
#endif
}

void fmc_set_affinity(fmc_tid tid, int cpuid, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_LINUX)
  cpu_set_t my_set;
  CPU_ZERO(&my_set);
  CPU_SET(cpuid, &my_set);
  int r = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &my_set);
  if (r) {
    errno = r;
    FMC_ERROR_REPORT(error, strerror(errno));
  }
#elif defined(FMC_SYS_MACH)
#elif defined(FMC_SYS_WIN)
  if (!SetProcessAffinityMask(GetCurrentProcess(), 1 << cpuid)) {
    FMC_ERROR_REPORT(error, "Can not set affinity");
  }

  if (!SetThreadPriority(tid, THREAD_PRIORITY_HIGHEST)) {
    FMC_ERROR_REPORT(error, "Can not set prority");
  }
#else
#error "Unsupported operating system"
#endif
}

FMMODFUNC void fmc_set_cur_affinity(int cpuid, fmc_error_t **error) {
  fmc_error_clear(error);
  auto tid = fmc_tid_cur(error);
  if (*error)
    return;
  fmc_set_affinity(tid, cpuid, error);
}