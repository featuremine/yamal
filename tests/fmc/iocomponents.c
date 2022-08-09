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

struct producer_component {
  fmc_component_HEAD;
};

static void producer_component_del(struct producer_component *comp) {
  free(comp);
};

static struct producer_component *producer_component_new(struct fmc_cfg_sect_item *cfg,
                                                         struct fmc_reactor_ctx *ctx,
                                                         char **inp_tps,
                                                         fmc_error_t **err) {
  struct producer_component *c = (struct producer_component *)calloc(1, sizeof(*c));
  if (!c) {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
    return NULL;
  }

  _reactor->queue(ctx);

  memset(c, 0, sizeof(*c));
  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "teststr");
  return c;
};

static bool producer_component_process_one(struct producer_component *comp,
                                       fmc_time64_t time) {
  static bool ret = false;
  if (fmc_time64_less(
          comp->timesim,
          fmc_time64_add(fmc_time64_start(), fmc_time64_from_nanos(100)))) {
    fmc_time64_inc(&comp->timesim, fmc_time64_from_nanos(10));
  }
  ret = !ret;
  return ret;
};

struct fmc_cfg_node_spec producer_component_cfg_spec[] = {
    {"teststr", "Test string", true, {FMC_CFG_STR, {NULL}}}, {NULL}};

struct consumer_component {
  fmc_component_HEAD;
  char *teststr;
  fmc_time64_t timesim;
};

static void consumer_component_del(struct consumer_component *comp) {
  free(comp->teststr);
  free(comp);
};

static struct consumer_component *consumer_component_new(struct fmc_cfg_sect_item *cfg,
                                                   struct fmc_reactor_ctx *ctx,
                                                   char **inp_tps,
                                                   fmc_error_t **err) {
  if (!inp_tps[0] || inp_tps[1]) {
    fmc_error_init_sprintf(err, "Expected 1 input");
    return NULL;
  }

  if (strncmp(inp_tps[0], "valid input type", 16) != 0) {
    fmc_error_init_sprintf(err, "Invalid input type '%s' provided", inp_tps[0]);
    return NULL;
  }

  struct consumer_component *c = (struct consumer_component *)calloc(1, sizeof(*c));
  if (!c) {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
    return NULL;
  }
  memset(c, 0, sizeof(*c));
  struct fmc_cfg_sect_item *item = fmc_cfg_sect_item_get(cfg, "teststr");
  return c;
};

struct fmc_cfg_node_spec consumer_component_cfg_spec[] = {
    {"teststr", "Test string", true, {FMC_CFG_STR, {NULL}}}, {NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "component",
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
