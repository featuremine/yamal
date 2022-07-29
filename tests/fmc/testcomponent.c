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

struct test_component {
  fmc_component_HEAD;
  char *teststr;
  fmc_time64_t timesim;
};

int cmp_key(struct fmc_cfg_sect_item *item, const char *key) {
  return strcmp(item->key, key);
}

static struct test_component *test_component_new(struct fmc_cfg_sect_item *cfg,
                                                 fmc_error_t **err) {
  struct test_component *c = (struct test_component *)calloc(1, sizeof(*c));
  if (!c) {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
    return NULL;
  }
  struct fmc_cfg_sect_item *item;
  LL_SEARCH(cfg, item, "teststr", cmp_key);
  if (item) {
    if (item->node.type == FMC_CFG_STR) {
      c->teststr = fmc_cstr_new(item->node.value.str, err);
      c->timesim = fmc_time64_start();
    } else {
      FMC_ERROR_REPORT(err, "Invalid type for string");
    }
  } else {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  }
  return c;
};

static fmc_time64_t test_component_sched(struct test_component *comp) {
  if (fmc_time64_greater(
          comp->timesim,
          fmc_time64_add(fmc_time64_start(), fmc_time64_from_nanos(95)))) {
    return fmc_time64_end();
  }
  return comp->timesim;
}

static bool test_component_process_one(struct test_component *comp,
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

static void test_component_del(struct test_component *comp) {
  free(comp->teststr);
  free(comp);
};

struct fmc_cfg_node_spec test_component_cfg_spec[] = {
    {"teststr", "Test string", true, {FMC_CFG_STR, {NULL}}}, {NULL}};

struct fmc_component_def_v1 components[] = {
    {
        .tp_name = "testcomponent",
        .tp_descr = "Test component",
        .tp_size = sizeof(struct test_component),
        .tp_cfgspec = test_component_cfg_spec,
        .tp_new = (fmc_newfunc)test_component_new,
        .tp_del = (fmc_delfunc)test_component_del,
        .tp_sched = (fmc_schedfunc)test_component_sched,
        .tp_proc = (fmc_procfunc)test_component_process_one,
    },
    {NULL},
};

FMCOMPMODINITFUNC void
FMCompInit_testcomponent(struct fmc_component_api *api,
                         struct fmc_component_module *mod) {
  api->components_add_v1(mod, components);
}
