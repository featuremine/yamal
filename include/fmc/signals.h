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
