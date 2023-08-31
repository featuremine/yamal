/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
