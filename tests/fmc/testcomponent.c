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

static void test_component_del(struct test_component *comp) {
  free(comp->teststr);
  free(comp);
};

static struct test_component *test_component_new(struct fmc_cfg_sect_item *cfg,
                                                 fmc_error_t **err) {
  struct test_component *c = (struct test_component *)calloc(1, sizeof(*c));
  if (!c) {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
    return NULL;
  }
  memset(c, 0, sizeof(*c));
  struct fmc_cfg_sect_item *item =
    fmc_cfg_sect_item_get(cfg, FMC_CFG_STR, "teststr", true, err);
  if(*err) goto cleanup;
  c->teststr = fmc_cstr_new(item->node.value.str, err);
  if(*err) goto cleanup;
  c->timesim = fmc_time64_start();
  return c;
cleanup:
  test_component_del(c);
  return NULL;
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
