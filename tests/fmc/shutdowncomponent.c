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

#include "shutdowncomponent.h"

struct fmc_reactor_api_v1 *_reactor;

struct shutdown_component {
  fmc_component_HEAD;
};

static void generic_component_del(struct fmc_component *comp) {
  free(comp);
};

static void queue_component_process_one(struct fmc_component *self,
                                        struct fmc_reactor_ctx *ctx,
                                        fmc_time64_t time) {
  _reactor->queue(ctx);
};

static struct shutdown_component *
no_shutdown_component_new(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  struct shutdown_component *c = (struct shutdown_component *)calloc(1, sizeof(*c));
  if (!c)
    goto cleanup;
  _reactor->on_exec(ctx, &queue_component_process_one);
  _reactor->queue(ctx);
  return c;
cleanup:
  if (c)
    free(c);
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

void shutdown_component_shutdown_cb(struct fmc_component *self,
                                    struct fmc_reactor_ctx *ctx) {
  struct shutdown_component_enabled_cb * typed = (struct shutdown_component_enabled_cb *)self;
  ++typed->shutdown_count;
}

static void shutdown_component_process_one(struct fmc_component *self,
                                        struct fmc_reactor_ctx *ctx,
                                        fmc_time64_t time) {
  struct shutdown_component_enabled_cb * typed = (struct shutdown_component_enabled_cb *)self;
  if (typed->shutdown_count && ++typed->post_shutdown_count == 10) {
    _reactor->finished(ctx);
  }
  _reactor->queue(ctx);
};

static struct shutdown_component_enabled_cb *
shutdown_component_new(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  struct shutdown_component_enabled_cb *c = (struct shutdown_component_enabled_cb *)calloc(1, sizeof(*c));
  if (!c)
    goto cleanup;
  _reactor->on_exec(ctx, &shutdown_component_process_one);
  _reactor->on_shutdown(ctx, &shutdown_component_shutdown_cb);
  _reactor->queue(ctx);
  return c;
cleanup:
  if (c)
    free(c);
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

static void no_queue_component_process_one(struct fmc_component *self,
                                            struct fmc_reactor_ctx *ctx,
                                            fmc_time64_t time) {
  struct shutdown_component_enabled_cb * typed = (struct shutdown_component_enabled_cb *)self;
  if (typed->shutdown_count) {
    ++typed->post_shutdown_count;
    _reactor->finished(ctx);
  }
  _reactor->queue(ctx);
};

static struct shutdown_component_enabled_cb *
no_queue_shutdown_component_new(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  struct shutdown_component_enabled_cb *c = (struct shutdown_component_enabled_cb *)calloc(1, sizeof(*c));
  if (!c)
    goto cleanup;
  _reactor->on_exec(ctx, &no_queue_component_process_one);
  _reactor->on_shutdown(ctx, &shutdown_component_shutdown_cb);
  _reactor->queue(ctx);
  return c;
cleanup:
  if (c)
    free(c);
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

struct fmc_cfg_node_spec empty_component_cfg_spec[] = {{NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "noshutdowncomponent",
        .tp_descr = "Component that queues itself allways",
        .tp_size = sizeof(struct shutdown_component),
        .tp_cfgspec = empty_component_cfg_spec,
        .tp_new = (fmc_newfunc)no_shutdown_component_new,
        .tp_del = (fmc_delfunc)generic_component_del,
    },
    {
        .tp_name = "shutdowncomponent",
        .tp_descr = "Component that queues itself allways, has shutdown",
        .tp_size = sizeof(struct shutdown_component),
        .tp_cfgspec = empty_component_cfg_spec,
        .tp_new = (fmc_newfunc)shutdown_component_new,
        .tp_del = (fmc_delfunc)generic_component_del,
    },
    {
        .tp_name = "noqueueshutdowncomponent",
        .tp_descr = "Component that does not queue itself, has shutdown",
        .tp_size = sizeof(struct shutdown_component),
        .tp_cfgspec = empty_component_cfg_spec,
        .tp_new = (fmc_newfunc)no_queue_shutdown_component_new,
        .tp_del = (fmc_delfunc)generic_component_del,
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
