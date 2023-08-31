/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file sockets.cpp
 * @author Alejandro Farfan
 * @date 17 Jun 2021
 * @brief File contains implementation of socket utilities
 *
 * @see http://www.featuremine.com
 */

#include <fmc/sockets.h>

#ifdef FMC_SYS_LINUX
#include <sched.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#endif

void fmc_socket_priority(int sock_handle, int val, fmc_error_t **error) {
  fmc_error_clear(error);
#if defined(FMC_SYS_LINUX)
  if (setsockopt(sock_handle, SOL_SOCKET, SO_PRIORITY, &val, sizeof(val)) !=
      0) {
    FMC_ERROR_REPORT(error, fmc_syserror_msg());
  }
#elif defined(FMC_SYS_MACH)
#else
#error "Unsupported operating system"
#endif
}
