#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/error.h>
#include <fmc/string.h>
#include <fmc/time.h>
#include <fmc/uthash/utlist.h>
#include <stdlib.h>
#include <string.h>

struct test_component {
  fmc_comp_HEAD;
  char *teststr;
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
    } else {
      FMC_ERROR_REPORT(err, "Invalid type for string");
    }
  } else {
    fmc_error_set2(err, FMC_ERROR_MEMORY);
  }
  return c;
};

static bool test_component_process_one(struct test_component *comp,
                                       fm_time64_t time) {
  struct test_component *c = (struct test_component *)comp;
  // TODO: process one
  return true;
};

static void test_component_del(struct test_component *comp) {
  struct test_component *c = (struct test_component *)comp;
  free(c->teststr);
  free(c);
};

struct fmc_cfg_node_spec test_component_cfg_spec[] = {
    {"teststr", "Test string", true, {FMC_CFG_STR, {NULL}}}, {NULL}};

struct fmc_component_type components[] = {
    {
        .tp_name = "test-component",
        .tp_descr = "Test component",
        .tp_size = sizeof(struct test_component),
        .tp_cfgspec = test_component_cfg_spec,
        .tp_new = (newfunc)test_component_new,
        .tp_del = (delfunc)test_component_del,
        .tp_sched = (schedfunc)NULL,
        .tp_proc = (procfunc)test_component_process_one,
    },
    {NULL},
};

FMCOMPMODINITFUNC struct fmc_component_type *FMCompInit_testcomponent() {
  return components;
}
