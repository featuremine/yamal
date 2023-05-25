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
  bool stop;
};

static void test_component_del(struct test_component *comp) {
  free(comp->teststr);
  free(comp);
};

static void test_component_shutdown(struct fmc_component *self,
                                    struct fmc_reactor_ctx *ctx) {
  struct test_component *comp = (struct test_component *)self;
  comp->stop = true;
};

static void test_component_process_one_sched(struct fmc_component *self,
                                             struct fmc_reactor_ctx *ctx,
                                             fmc_time64_t time) {
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
  c->stop = false;
  _reactor->on_exec(ctx, test_component_process_one_sched);
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
                                            fmc_time64_t time) {
  struct test_component *comp = (struct test_component *)self;
  fmc_time64_inc(&comp->timesim, fmc_time64_from_nanos(10));
  if (!comp->stop) {
    _reactor->queue(ctx);
  } else {
    _reactor->finished(ctx);
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
  c->stop = false;
  _reactor->on_exec(ctx, test_component_process_one_live);
  _reactor->queue(ctx);
  _reactor->on_shutdown(ctx, test_component_shutdown);
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

static void generic_component_del(struct fmc_component *comp) { free(comp); };

static struct fmc_component *dummy_component_new(struct fmc_cfg_sect_item *cfg,
                                                 struct fmc_reactor_ctx *ctx,
                                                 char **inp_tps) {
  if (inp_tps && inp_tps[0]) {
    _reactor->set_error(ctx, "dummy component does not expect any inputs");
    return NULL;
  }

  {
    struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "booleanval");
    if (item->node.value.boolean != true) {
      _reactor->set_error(ctx, "invalid configured value for booleanval");
      return NULL;
    }
  }

  {
    struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "float64val");
    if (item->node.value.float64 > 44.21 + DBL_EPSILON &&
        item->node.value.float64 < 44.21 - DBL_EPSILON) {
      _reactor->set_error(ctx, "invalid configured value for float64val");
      return NULL;
    }
  }

  {
    struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "int64val");
    if (item->node.value.int64 != 32) {
      _reactor->set_error(ctx, "invalid configured value for int64val");
      return NULL;
    }
  }

  {
    struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "noneval");
    if (item->node.type != FMC_CFG_NONE) {
      _reactor->set_error(ctx, "invalid configured value for noneval");
      return NULL;
    }
  }

  {
    struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "stringval");
    if (strcmp(item->node.value.str, "somestring") != 0) {
      _reactor->set_error(ctx, "invalid configured value for stringval");
      return NULL;
    }
  }

  struct fmc_component *c = (struct fmc_component *)calloc(1, sizeof(*c));
  if (!c) {
    _reactor->set_error(ctx, NULL, FMC_ERROR_MEMORY);
    return NULL;
  }

  memset(c, 0, sizeof(*c));
  return c;
};

#ifdef __cplusplus
extern "C" {
#endif

struct fmc_cfg_node_spec test_component_cfg_spec[] = {
    {"teststr", "Test string", true, {FMC_CFG_STR, {NULL}}}, {NULL}};

struct fmc_cfg_node_spec all_cfg_spec[] = {{
                                               .key = "booleanval",
                                               .descr = "boolean value",
                                               .required = true,
                                               .type =
                                                   {
                                                       .type = FMC_CFG_BOOLEAN,
                                                   },
                                           },
                                           {
                                               .key = "float64val",
                                               .descr = "float64 value",
                                               .required = true,
                                               .type =
                                                   {
                                                       .type = FMC_CFG_FLOAT64,
                                                   },
                                           },
                                           {
                                               .key = "int64val",
                                               .descr = "int64 value",
                                               .required = true,
                                               .type =
                                                   {
                                                       .type = FMC_CFG_INT64,
                                                   },
                                           },
                                           {
                                               .key = "noneval",
                                               .descr = "none value",
                                               .required = true,
                                               .type =
                                                   {
                                                       .type = FMC_CFG_NONE,
                                                   },
                                           },
                                           {
                                               .key = "stringval",
                                               .descr = "string value",
                                               .required = true,
                                               .type =
                                                   {
                                                       .type = FMC_CFG_STR,
                                                   },
                                           },
                                           {NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "dummycomponent",
        .tp_descr = "Dummy component",
        .tp_size = sizeof(struct fmc_component),
        .tp_cfgspec = all_cfg_spec,
        .tp_new = (fmc_newfunc)&dummy_component_new,
        .tp_del = &generic_component_del,
    },
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
