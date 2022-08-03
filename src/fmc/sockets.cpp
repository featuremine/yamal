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
