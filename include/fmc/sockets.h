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
 * @file sockets.h
 * @author Alejandro Farfan
 * @date 17 Jun 2021
 * @brief File contains definition socket utilities
 *
 * @see http://www.featuremine.com
 */
#pragma once

#include <fmc/error.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Changes the priority of a socket
 *
 * @param sock_handle
 * @param val
 * @param error out-parameter for error handling
 */
FMMODFUNC void fmc_socket_priority(int sock_handle, int val,
                                   fmc_error_t **error);

#ifdef __cplusplus
}
#endif
