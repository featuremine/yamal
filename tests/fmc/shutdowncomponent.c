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

#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/error.h>
#include <fmc/string.h>
#include <fmc/time.h>
#include <stdlib.h>
#include <string.h>
#include <uthash/utlist.h>

struct fmc_reactor_api_v1 *_reactor;

struct no_shutdown_component {
  fmc_component_HEAD;
};

static void no_shutdown_component_del(struct no_shutdown_component *comp) {
  free(comp);
};

static void no_shutdown_component_process_one(struct fmc_component *self,
                                            struct fmc_reactor_ctx *ctx,
                                            fmc_time64_t time) {
  _reactor->queue(ctx);
};

static struct no_shutdown_component *
no_shutdown_component_new_sched(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  struct no_shutdown_component *c = (struct no_shutdown_component *)calloc(1, sizeof(*c));
  if (!c)
    goto cleanup;
  _reactor->on_exec(ctx, &no_shutdown_component_process_one);
  _reactor->queue(ctx);
  return c;
cleanup:
  if (c)
    free(c);
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

struct fmc_cfg_node_spec no_shutdown_component_cfg_spec[] = {{NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "noshutdowncomponent",
        .tp_descr = "Component that queues itself allways",
        .tp_size = sizeof(struct no_shutdown_component),
        .tp_cfgspec = no_shutdown_component_cfg_spec,
        .tp_new = (fmc_newfunc)no_shutdown_component_new_sched,
        .tp_del = (fmc_delfunc)no_shutdown_component_del,
    },
};

#ifdef __cplusplus
extern "C" {
#endif

FMCOMPMODINITFUNC void
FMCompInit_shutdowncomponent(struct fmc_component_api *api,
                         struct fmc_component_module *mod) {
  api->components_add_v1(mod, components);
  _reactor = api->reactor_v1;
}

#ifdef __cplusplus
}
#endif
