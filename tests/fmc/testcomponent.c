#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/error.h>
#include <fmc/time.h>
#include <stdlib.h>

struct test_component {
  fmc_comp_HEAD;
  char *teststr;
};

static struct test_component *test_component_new(struct fmc_cfg_sect_item *cfg, fmc_error_t **err) {
  struct test_component *c = (struct test_component *)calloc(1, sizeof(*c));
  if(!c) {
   fmc_error_set2(err, FMC_ERROR_MEMORY);
   return NULL;
  }
  // TODO: Parse config
  return c;
};

static bool test_component_process_one(struct test_component *comp, fm_time64_t time) {
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
  {"teststr", "Test string", true, {FMC_CFG_STR, {NULL}} },
  {NULL}
};


struct fmc_component_type components[] = {
  {
    .name = "testcomponent",
    .descr = "Test component",
    .size = sizeof(struct test_component),
    .cfgspec = test_component_cfg_spec,
    .new = (newfunc)test_component_new,
    .del = (delfunc)test_component_del,
    .sched = (schedproc)NULL,
    .process = (processproc)test_component_process_one,
  },
  { NULL },
};

FMCOMPMODINITFUNC struct fmc_component_type *FMCompInit_testcomponent() {
  return components;
}