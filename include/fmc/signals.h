/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file singals.h
 * @author Federico Ravchina
 * @date 11 May 2021
 * @brief File common C declaration useful in includes
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Changes the action taken by a process on receipt of SIGQUIT, SIGABRT,
 * SIGTERM or SIGINT signal.
 * @param sig_handler a pointer to a signal handling function which receives the
 * signal number as its only argument.
 */
FMMODFUNC void fmc_set_signal_handler(void (*sig_handler)(int signal));

#ifdef __cplusplus
}
#endif
