/******************************************************************************

        COPYRIGHT (c) 2018 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file component.cpp
 * @date 19 Jul 2022
 * @brief File contains tests for components
 *
 * @see http://www.featuremine.com
 */

#include <fmc/component.h>
#include <fmc/config.h>
#include <fmc/error.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>

struct test_component {
  fmc_component_HEAD;
  char *teststr;
};

std::string components_path;
struct fmc_component_sys sys;

TEST(component, sys) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_component_sys_init(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
  const char *paths[2];
  paths[0] = components_path.c_str();
  paths[1] = nullptr;

  fmc_component_sys_paths_set(&sys, paths, &err);
  ASSERT_EQ(err, nullptr);
  fmc_component_path_list_t *p = fmc_component_sys_paths_get(&sys);
  ASSERT_EQ(sys.modules, nullptr);
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(std::string(p->path), std::string(paths[0]));
  ASSERT_EQ(p->next, nullptr);
  ASSERT_EQ(p, p->prev);

  const char new_path[] = "new_path";
  fmc_component_sys_paths_add(&sys, new_path, &err);
  ASSERT_EQ(err, nullptr);
  fmc_component_path_list_t *p2 = fmc_component_sys_paths_get(&sys);
  ASSERT_EQ(sys.modules, nullptr);
  ASSERT_NE(p2, nullptr);
  ASSERT_NE(p2, p2->next);
  EXPECT_EQ(std::string(p2->path), std::string(paths[0]));
  EXPECT_EQ(std::string(p2->next->path), std::string(new_path));

  fmc_component_sys_destroy(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
}

TEST(component, module) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_component_sys_init(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
  const char *paths[2];
  paths[0] = components_path.c_str();
  paths[1] = nullptr;

  fmc_component_sys_paths_set(&sys, paths, &err);
  ASSERT_EQ(err, nullptr);
  fmc_component_path_list_t *p = fmc_component_sys_paths_get(&sys);
  ASSERT_EQ(sys.modules, nullptr);
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(std::string(p->path), std::string(paths[0]));
  ASSERT_EQ(p->next, nullptr);
  ASSERT_EQ(p, p->prev);

  struct fmc_component_module *modfail =
      fmc_component_module_new(&sys, "failcomponent", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(modfail, nullptr);

  struct fmc_component_module *mod =
      fmc_component_module_new(&sys, "testcomponent", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(mod->sys, &sys);
  ASSERT_EQ(std::string(mod->name), std::string("testcomponent"));
  ASSERT_EQ(sys.modules, mod);
  ASSERT_EQ(sys.modules->prev, mod);
  fmc_component_module_del(mod);
  ASSERT_EQ(sys.modules, nullptr);

  fmc_component_sys_destroy(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
}

TEST(component, component) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_component_sys_init(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
  const char *paths[2];
  paths[0] = components_path.c_str();
  paths[1] = nullptr;

  fmc_component_sys_paths_set(&sys, paths, &err);
  ASSERT_EQ(err, nullptr);
  fmc_component_path_list_t *p = fmc_component_sys_paths_get(&sys);
  ASSERT_EQ(sys.modules, nullptr);
  ASSERT_NE(p, nullptr);
  EXPECT_EQ(std::string(p->path), std::string(paths[0]));
  ASSERT_EQ(p->next, nullptr);
  ASSERT_EQ(p, p->prev);

  struct fmc_component_module *mod =
      fmc_component_module_new(&sys, "testcomponent", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(mod->sys, &sys);
  ASSERT_EQ(std::string(mod->name), std::string("testcomponent"));
  ASSERT_EQ(sys.modules, mod);
  ASSERT_EQ(sys.modules->prev, mod);

  struct fmc_cfg_sect_item *cfginvalid =
      fmc_cfg_sect_item_add_str(nullptr, "invalidkey", "message", &err);
  ASSERT_EQ(err, nullptr);
  struct fmc_component *compinvalid =
      fmc_component_new(mod, "test-component", cfginvalid, &err);
  ASSERT_NE(err, nullptr);
  ASSERT_EQ(compinvalid, nullptr);

  struct fmc_cfg_sect_item *cfg =
      fmc_cfg_sect_item_add_str(nullptr, "teststr", "message", &err);
  ASSERT_EQ(err, nullptr);
  struct fmc_component *comp =
      fmc_component_new(mod, "test-component", cfg, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(sys.modules, comp->_mod);
  ASSERT_EQ(std::string(comp->_vt->tp_name), std::string("test-component"));
  ASSERT_EQ(comp->_err.code, FMC_ERROR_NONE);
  struct test_component *testcomp = (struct test_component *)comp;
  ASSERT_EQ(std::string(testcomp->teststr), std::string("message"));

  fmc_component_del(comp);
  fmc_cfg_sect_del(cfginvalid);
  fmc_cfg_sect_del(cfg);

  fmc_component_module_del(mod);
  ASSERT_EQ(sys.modules, nullptr);

  fmc_component_sys_destroy(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  components_path = fs::path(argv[1]).parent_path();
  return RUN_ALL_TESTS();
}
