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
 * @brief File common C implementation useful in includes
 *
 * @see http://www.featuremine.com
 */

#include <fmc/signals.h>
#include <signal.h>

void fmc_set_signal_handler(void (*sig_handler)(int signal)) {
#if defined(FMC_SYS_WIN)
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGABRT, sig_handler);
#else
  struct sigaction act;
  // Setup the handler
  act.sa_handler = sig_handler;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGQUIT);
  sigaddset(&act.sa_mask, SIGABRT);
  sigaddset(&act.sa_mask, SIGTERM);
  act.sa_flags = 0;
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGINT, &act, NULL);
#endif
}
