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

struct test_component {
  fmc_component_HEAD;
  char *teststr;
  fmc_time64_t timesim;
};

static void test_component_del(struct test_component *comp) {
  free(comp->teststr);
  free(comp);
};

static void test_component_process_one_sched(struct fmc_component *self,
                                             struct fmc_reactor_ctx *ctx,
                                             fmc_time64_t time, int argc,
                                             struct fmc_shmem a[]) {
  struct test_component *comp = (struct test_component *)self;
  if (fmc_time64_less(
          comp->timesim,
          fmc_time64_add(fmc_time64_start(), fmc_time64_from_nanos(100)))) {
    fmc_time64_inc(&comp->timesim, fmc_time64_from_nanos(10));
    _reactor->schedule(ctx, comp->timesim);
  }
};

static struct test_component *
test_component_new_sched(struct fmc_cfg_sect_item *cfg,
                         struct fmc_reactor_ctx *ctx, char **inp_tps) {
  fmc_error_t *err = NULL;
  struct test_component *c = (struct test_component *)calloc(1, sizeof(*c));
  if (!c)
    goto cleanup;
  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "teststr");
  c->teststr = fmc_cstr_new(item->node.value.str, &err);
  if (err)
    goto cleanup;
  c->timesim = fmc_time64_start();
  _reactor->on_exec(ctx, &test_component_process_one_sched);
  _reactor->schedule(ctx, c->timesim);
  return c;
cleanup:
  if (c)
    test_component_del(c);
  if (!err)
    _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  else
    _reactor->set_error(ctx, fmc_error_msg(err));
  return NULL;
};

static void test_component_process_one_live(struct fmc_component *self,
                                            struct fmc_reactor_ctx *ctx,
                                            fmc_time64_t time, int argc,
                                            struct fmc_shmem a[]) {
  struct test_component *comp = (struct test_component *)self;
  if (fmc_time64_less(
          comp->timesim,
          fmc_time64_add(fmc_time64_start(), fmc_time64_from_nanos(100)))) {
    fmc_time64_inc(&comp->timesim, fmc_time64_from_nanos(10));
    _reactor->queue(ctx);
  }
};

static struct test_component *
test_component_new_live(struct fmc_cfg_sect_item *cfg,
                        struct fmc_reactor_ctx *ctx, char **inp_tps) {
  fmc_error_t *err = NULL;
  struct test_component *c = (struct test_component *)calloc(1, sizeof(*c));
  if (!c)
    goto cleanup;
  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "teststr");
  c->teststr = fmc_cstr_new(item->node.value.str, &err);
  if (err)
    goto cleanup;
  c->timesim = fmc_time64_start();
  _reactor->on_exec(ctx, &test_component_process_one_live);
  _reactor->queue(ctx);
  return c;
cleanup:
  if (c)
    test_component_del(c);
  if (!err)
    _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
  else
    _reactor->set_error(ctx, fmc_error_msg(err));
  return NULL;
};

#ifdef __cplusplus
extern "C" {
#endif

struct fmc_cfg_node_spec test_component_cfg_spec[] = {
    {"teststr", "Test string", true, {FMC_CFG_STR, {NULL}}}, {NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "testcomponentsched",
        .tp_descr = "Test component scheduled",
        .tp_size = sizeof(struct test_component),
        .tp_cfgspec = test_component_cfg_spec,
        .tp_new = (fmc_newfunc)test_component_new_sched,
        .tp_del = (fmc_delfunc)test_component_del,
    },
    {
        .tp_name = "testcomponentlive",
        .tp_descr = "Test component live",
        .tp_size = sizeof(struct test_component),
        .tp_cfgspec = test_component_cfg_spec,
        .tp_new = (fmc_newfunc)test_component_new_live,
        .tp_del = (fmc_delfunc)test_component_del,
    },
    {NULL},
};

FMCOMPMODINITFUNC void
FMCompInit_testcomponent(struct fmc_component_api *api,
                         struct fmc_component_module *mod) {
  api->components_add_v1(mod, components);
  _reactor = api->reactor_v1;
}

#ifdef __cplusplus
}
#endif
