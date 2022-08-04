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
