/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file process.h
 * @author Alejandro Farfan
 * @date 17 Jun 2021
 * @brief File contains affinity and priority utilities
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/error.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(FMC_SYS_UNIX)
#include <pthread.h>
#include <sched.h>
typedef pthread_t fmc_tid;
#elif defined(FMC_SYS_WIN)
#include <windows.h>
typedef HANDLE fmc_tid;
#endif

/**
 * @brief Executes a command in a shell
 * @param error out-parameter for error handling
 * @return pid_t child's process id
 */
pid_t fmc_exec(const char *cmd, fmc_error_t **err);

/**
 * @brief Waits for a given process to change status
 * @param error out-parameter for error handling
 * @return int process status
 */
int fmc_waitpid(pid_t pid, fmc_error_t **err);

/**
 * @brief Returns the current thread id
 * @param error out-parameter for error handling
 * @return fmc_tid
 */
FMMODFUNC fmc_tid fmc_tid_cur(fmc_error_t **error);

/**
 * @brief Sets a thread's CPU affinity
 * @param tid thread id
 * @param cpuid CPU id
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_set_affinity(fmc_tid tid, int cpuid, fmc_error_t **error);

/**
 * @brief Sets current thread's CPU affinity
 * @param cpuid CPU id
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_set_cur_affinity(int cpuid, fmc_error_t **error);

/**
 * @brief Sets the priority of a thread using normal scheduling
 *
 * @param tid thread id
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_set_sched_normal(fmc_tid tid, fmc_error_t **error);

/**
 * @brief Sets the priority of a thread using FIFO scheduling
 *
 * @param tid thread id
 * @param priority priority being 1 the lowest and 99 the maximum
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_set_sched_fifo(fmc_tid tid, int priority,
                                  fmc_error_t **error);

#ifdef __cplusplus
}
#endif
