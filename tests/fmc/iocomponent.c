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
#include <fmc/reactor.h>
#include <fmc/memory.h>

struct fmc_reactor_api_v1 *_reactor;

struct producer_component {
  fmc_component_HEAD;
  size_t count;
  struct fmc_shmem mem;
};

static void producer_component_del(struct producer_component *comp) {
  free(comp);
};

static void producer_component_process_one(struct fmc_component *self,
                                           struct fmc_reactor_ctx *ctx,
                                           fmc_time64_t now,
                                           int argc, struct fmc_shmem argv[]) {
  struct producer_component* comp = (struct producer_component*)self;
  char* data = (char*)*argv[0].view;
  _reactor->notify(ctx, 0, comp->mem);
};

static struct producer_component *producer_component_new(struct fmc_cfg_sect_item *cfg,
                                                 struct fmc_reactor_ctx *ctx,
                                                 char **inp_tps,
                                                 fmc_error_t **err) {
  if (inp_tps && inp_tps[0]) {
    fmc_error_set(err, "Invalid number of inputs, expected 0");    
    return NULL;
  }
  struct producer_component *c = (struct producer_component *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  memset(c, 0, sizeof(*c));
  struct pool* pool = _reactor->get_pool(ctx);
  fmc_shmem_init_view(&c->mem, pool, &c->count, sizeof(c->count), &err);
  if (err) goto cleanup;
  _reactor->on_exec(ctx, &producer_component_process_one);
  _reactor->add_output(ctx, "valid output", "valid output type");
  return c;
cleanup:
  if (c) free(c);
  if (!err || !*err)
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  return NULL;
};

struct fmc_cfg_node_spec producer_component_cfg_spec[] = {{NULL}};

struct consumer_component {
  fmc_component_HEAD;
  size_t proc;
  bool valid_values;
};

static void consumer_component_del(struct consumer_component *comp) {
  free(comp);
};

static void consumer_component_process_one(struct fmc_component *self,
                                           struct fmc_reactor_ctx *ctx,
                                           fmc_time64_t now,
                                           int argc, struct fmc_shmem argv[]) {
  struct consumer_component* comp = (struct consumer_component*)self;
  size_t* data = (size_t*)*argv[0].view;
  if (*data != comp->proc)
    comp->valid_values = false;
  ++comp->proc;
};

static struct consumer_component *consumer_component_new(struct fmc_cfg_sect_item *cfg,
                                                         struct fmc_reactor_ctx *ctx,
                                                         char **inp_tps,
                                                         fmc_error_t **err) {
  if (!inp_tps || !inp_tps[0] || inp_tps[1]) {
    fmc_error_set(err, "Invalid number of inputs, expected 1");    
    return NULL;
  }
  if (strncmp(inp_tps[0], "valid input type", 16) != 0) {
    fmc_error_set(err, "Invalid input type %s, expected 'valid input type'", inp_tps[0]);    
    return NULL;
  }
  struct consumer_component *c = (struct consumer_component *)calloc(1, sizeof(*c));
  if (!c) goto cleanup;
  memset(c, 0, sizeof(*c));
  _reactor->on_exec(ctx, &consumer_component_process_one);
  c->valid_values = true;
  return c;
cleanup:
  if (c) free(c);
  if (!err || !*err)
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  return NULL;
};

struct fmc_cfg_node_spec consumer_component_cfg_spec[] = {{NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "producercomponent",
        .tp_descr = "Producer component",
        .tp_size = sizeof(struct producer_component),
        .tp_cfgspec = producer_component_cfg_spec,
        .tp_new = (fmc_newfunc)producer_component_new,
        .tp_del = (fmc_delfunc)producer_component_del,
    },
    {
        .tp_name = "consumercomponent",
        .tp_descr = "Consumer component",
        .tp_size = sizeof(struct consumer_component),
        .tp_cfgspec = consumer_component_cfg_spec,
        .tp_new = (fmc_newfunc)consumer_component_new,
        .tp_del = (fmc_delfunc)consumer_component_del,
    },
    {NULL},
};

FMCOMPMODINITFUNC void
FMCompInit_testcomponent(struct fmc_component_api *api,
                         struct fmc_component_module *mod) {
  api->components_add_v1(mod, components);
  if (!api->reactor_v1) {
    fmc_error_init_sprintf(&mod->error, "Unable to find reactor api v1");
  }
  _reactor = api->reactor_v1;
}
