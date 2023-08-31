/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/error.h>
#include <fmc/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fmc_reactor_api_v1 *_reactor;

struct testcomponent {
  fmc_component_HEAD;
  FILE *fp;
  int run;
};

struct consumer_component {
  fmc_component_HEAD;
};

static void testcomponent_del(struct testcomponent *comp) {
  if (comp->fp)
    fclose(comp->fp);
  free(comp);
}

static void testcomponent_process_one(struct fmc_component *self,
                                      struct fmc_reactor_ctx *ctx,
                                      fmc_time64_t time) {
  struct testcomponent *comp = (struct testcomponent *)self;
  ++comp->run;
  fprintf(comp->fp, "%ld\n", fmc_time64_to_nanos(time));
  if (comp->run < 5) {
    _reactor->schedule(ctx, fmc_time64_from_nanos(comp->run));
  }
}

static struct testcomponent *testcomponent_new(struct fmc_cfg_sect_item *cfg,
                                               struct fmc_reactor_ctx *ctx,
                                               char **inp_tps,
                                               fmc_error_t **err) {
  struct testcomponent *c = (struct testcomponent *)calloc(1, sizeof(*c));
  if (!c)
    goto cleanup;
  c->run = 0;
  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "filename");
  c->fp = fopen(item->node.value.str, "w");
  if (!c->fp)
    goto cleanup;
  _reactor->add_output(ctx, "type1", "out1");
  _reactor->add_output(ctx, "type2", "_component");
  _reactor->add_output(ctx, "type3", "_reactor");
  _reactor->on_exec(ctx, &testcomponent_process_one);
  _reactor->schedule(ctx, fmc_time64_from_nanos(c->run));
  return c;
cleanup:
  if (c)
    testcomponent_del(c);
  if (!*err)
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  return NULL;
}

static struct consumer_component *
consumer_component_new(struct fmc_cfg_sect_item *cfg,
                       struct fmc_reactor_ctx *ctx, char **inp_tps) {
  if (!inp_tps) {
    _reactor->set_error(ctx, "consumer component expects inputs");
    return NULL;
  }
  struct consumer_component *c =
      (struct consumer_component *)calloc(1, sizeof(*c));
  if (!c)
    goto cleanup;
  return c;
cleanup:
  _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  return NULL;
};

static void consumer_component_del(struct fmc_component *comp) { free(comp); };

struct fmc_cfg_node_spec subsect[] = {
    {"int64", "int64 descr", true, {FMC_CFG_INT64, {NULL}}}, {NULL}};

struct fmc_cfg_type arr = {FMC_CFG_INT64, {NULL}};

struct fmc_cfg_node_spec testcomponent_cfg_spec[] = {
    {"none", "none descr", true, {FMC_CFG_NONE, {NULL}}},
    {"boolean", "boolean descr", true, {FMC_CFG_BOOLEAN, {NULL}}},
    {"int64", "int64 descr", true, {FMC_CFG_INT64, {NULL}}},
    {"float64", "float64 descr", false, {FMC_CFG_FLOAT64, {NULL}}},
    {"filename", "filename descr", true, {FMC_CFG_STR, {NULL}}},
    {"sect", "sect descr", true, {FMC_CFG_SECT, {.node = subsect}}},
    {"arr", "arr descr", true, {FMC_CFG_ARR, {.array = &arr}}},
    {NULL}};

struct fmc_cfg_node_spec empty_cfg_spec[] = {{NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "testcomponent",
        .tp_descr = "Test component",
        .tp_size = sizeof(struct testcomponent),
        .tp_cfgspec = testcomponent_cfg_spec,
        .tp_new = (fmc_newfunc)testcomponent_new,
        .tp_del = (fmc_delfunc)testcomponent_del,
    },
    {
        .tp_name = "consumercomponent",
        .tp_descr = "Consumer component",
        .tp_size = sizeof(struct consumer_component),
        .tp_cfgspec = empty_cfg_spec,
        .tp_new = (fmc_newfunc)&consumer_component_new,
        .tp_del = &consumer_component_del,
    },
    {NULL},
};

#ifdef __cplusplus
extern "C" {
#endif

FMCOMPMODINITFUNC void FMCompInit_testmodule(struct fmc_component_api *api,
                                             struct fmc_component_module *mod) {
  api->components_add_v1(mod, components);
  _reactor = api->reactor_v1;
}

#ifdef __cplusplus
}
#endif
