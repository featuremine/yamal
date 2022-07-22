/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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
 * @file reactor.h
 * @date 21 Jul 2022
 * @brief Reactor component API
 *
 * @see http://www.featuremine.com
 */

/* Usage example:
fmc_error_t *error;
fmc_reactor loop;
fmc_reactor_init(&loop);
fmc_reactor_component_add(&loop, gateway, &error); // Add priority later
fmc_reactor_component_add(&loop, manager, &error);
fmc_reactor_run(&loop);
fmc_reactor_destroy(&loop);
// Sould we need a function to stop it from another thread?
fmc_reactor_stop(&loop);
*/

#pragma once

#include <fmc/component.h>
#include <fmc/error.h>
#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

struct fmc_reactor {
  struct fmc_component_list *comps;
  volatile bool stop;
  bool done;
};

FMMODFUNC void fmc_reactor_init(struct fmc_reactor *reactor);
FMMODFUNC void fmc_reactor_destroy(struct fmc_reactor *reactor);
FMMODFUNC void fmc_reactor_component_add(struct fmc_reactor *reactor,
                                         struct fmc_component *comp,
                                         fmc_error_t **error);
FMMODFUNC void fmc_reactor_run(struct fmc_reactor *reactor);
FMMODFUNC void fmc_reactor_stop(struct fmc_reactor *reactor);
FMMODFUNC bool fmc_reactor_is_done(struct fmc_reactor *reactor);

#ifdef __cplusplus
}
#endif
