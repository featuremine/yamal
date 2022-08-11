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
#include "iocomponent.h"

struct fmc_reactor_api_v1 *_reactor;
int64_t value = 9855;

static void generic_component_del(struct fmc_component *comp) {
  free(comp);
};

static void producer_component_process_one(struct fmc_component *self,
                                           struct fmc_reactor_ctx *ctx,
                                           fmc_time64_t time){
  struct producer_component *c = (struct producer_component *)self;
  if (c->count == 10) {
    return;
  }

  ++c->count;
  struct fmc_shmem mem;
  fmc_error_t *err = NULL;
  ++value;
  fmc_shmem_init_view(&mem, _reactor->get_pool(ctx), &value, sizeof(value), &err);
  if (err) {
    _reactor->set_error(ctx, fmc_error_msg(err));
  } else {
    _reactor->notify(ctx, 0, mem);
    _reactor->queue(ctx);
  }
};

static struct producer_component *
producer_component_new(struct fmc_cfg_sect_item *cfg,
                       struct fmc_reactor_ctx *ctx, char **inp_tps) {
  if (inp_tps && inp_tps[0]) {
    _reactor->set_error(ctx, "producer component does not expect any inputs");
    return NULL;
  }
  struct producer_component *c =
      (struct producer_component *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  memset(c, 0, sizeof(*c));
  _reactor->on_exec(ctx, &producer_component_process_one);
  _reactor->add_output(ctx, "valid output type", "valid output");
  _reactor->queue(ctx);
  return c;
cleanup:
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

static struct producer_component_2 *
producer_component_2_new(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  if (inp_tps && inp_tps[0]) {
    _reactor->set_error(ctx, "producer component does not expect any inputs");
    return NULL;
  }
  struct producer_component_2 *c =
      (struct producer_component_2 *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  memset(c, 0, sizeof(*c));
  _reactor->add_output(ctx, "valid output type", "valid output");
  _reactor->add_output(ctx, "valid output type", "valid output 2");
  return c;
cleanup:
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

static struct producer_component_3 *
producer_component_3_new(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  if (inp_tps && inp_tps[0]) {
    _reactor->set_error(ctx, "producer component does not expect any inputs");
    return NULL;
  }
  struct producer_component_3 *c =
      (struct producer_component_3 *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  memset(c, 0, sizeof(*c));
  _reactor->add_output(ctx, "valid output type", "valid output");
  _reactor->add_output(ctx, "valid output type", "valid output 2");
  _reactor->add_output(ctx, "valid output type", "valid output 3");
  return c;
cleanup:
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

void consumer_component_on_dep(struct fmc_component *self,
                               struct fmc_reactor_ctx *ctx, int idx,
                               struct fmc_shmem in) {
  if (idx != 0) {
    _reactor->set_error(ctx, "Invalid input updated %d, expected 0", idx);
    return;
  }
  size_t incoming = *(size_t*)*in.view;
  if (incoming != value) {
    _reactor->set_error(ctx, "Received invalid value %lu, expected %lu", incoming, value);
  }
}

static void consumer_component_process_one(struct fmc_component *self,
                                           struct fmc_reactor_ctx *ctx,
                                           fmc_time64_t time){
  struct consumer_component *c = (struct consumer_component *)self;
  ++c->executed;
};

static struct consumer_component *
consumer_component_new(struct fmc_cfg_sect_item *cfg,
                       struct fmc_reactor_ctx *ctx, char **inp_tps) {
  if (!inp_tps || !inp_tps[0] || inp_tps[1]) {
    _reactor->set_error(ctx, "consumer component expects a single input");
    return NULL;
  }
  struct consumer_component *c =
      (struct consumer_component *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  memset(c, 0, sizeof(*c));
  _reactor->on_exec(ctx, &consumer_component_process_one);
  _reactor->on_dep(ctx, &consumer_component_on_dep);
  return c;
cleanup:
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

static struct consumer_component_2 *
consumer_component_2_new(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  if (!inp_tps || !inp_tps[0] || !inp_tps[1] || inp_tps[2]) {
    _reactor->set_error(ctx, "consumer component expects two inputs");
    return NULL;
  }
  struct consumer_component_2 *c =
      (struct consumer_component_2 *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  memset(c, 0, sizeof(*c));
  return c;
cleanup:
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

static struct consumer_component_3 *
consumer_component_3_new(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  if (!inp_tps || !inp_tps[0] || !inp_tps[1] || !inp_tps[2] || inp_tps[3]) {
    _reactor->set_error(ctx, "consumer component expects three inputs");
    return NULL;
  }
  struct consumer_component_3 *c =
      (struct consumer_component_3 *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  memset(c, 0, sizeof(*c));
  return c;
cleanup:
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

struct fmc_cfg_node_spec empty_cfg_spec[] = {{NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "producercomponent",
        .tp_descr = "Producer component",
        .tp_size = sizeof(struct producer_component),
        .tp_cfgspec = empty_cfg_spec,
        .tp_new = (fmc_newfunc)&producer_component_new,
        .tp_del = &generic_component_del,
    },
    {
        .tp_name = "producercomponent2",
        .tp_descr = "Producer component with two outputs",
        .tp_size = sizeof(struct producer_component_2),
        .tp_cfgspec = empty_cfg_spec,
        .tp_new = (fmc_newfunc)&producer_component_2_new,
        .tp_del = &generic_component_del,
    },
    {
        .tp_name = "producercomponent3",
        .tp_descr = "Producer component with three outputs",
        .tp_size = sizeof(struct producer_component_3),
        .tp_cfgspec = empty_cfg_spec,
        .tp_new = (fmc_newfunc)&producer_component_3_new,
        .tp_del = &generic_component_del,
    },
    {
        .tp_name = "consumercomponent",
        .tp_descr = "Consumer component",
        .tp_size = sizeof(struct consumer_component),
        .tp_cfgspec = empty_cfg_spec,
        .tp_new = (fmc_newfunc)&consumer_component_new,
        .tp_del = &generic_component_del,
    },
    {
        .tp_name = "consumercomponent2",
        .tp_descr = "Consumer component with two inputs",
        .tp_size = sizeof(struct consumer_component_2),
        .tp_cfgspec = empty_cfg_spec,
        .tp_new = (fmc_newfunc)&consumer_component_2_new,
        .tp_del = &generic_component_del,
    },
    {
        .tp_name = "consumercomponent3",
        .tp_descr = "Consumer component with three inputs",
        .tp_size = sizeof(struct consumer_component_3),
        .tp_cfgspec = empty_cfg_spec,
        .tp_new = (fmc_newfunc)&consumer_component_3_new,
        .tp_del = &generic_component_del,
    },
    {NULL},
};

#ifdef __cplusplus
extern "C" {
#endif

FMCOMPMODINITFUNC void
FMCompInit_iocomponent(struct fmc_component_api *api,
                       struct fmc_component_module *mod) {
  api->components_add_v1(mod, components);
  _reactor = api->reactor_v1;
}

#ifdef __cplusplus
}
#endif
