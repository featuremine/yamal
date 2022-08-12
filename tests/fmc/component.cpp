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
#include <fmc/reactor.h>
#include <stdlib.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>

#if defined(FMC_SYS_UNIX)
#define FMC_MOD_SEARCHPATH_CUR ""
#define FMC_MOD_SEARCHPATH_USRLOCAL ".local/lib/yamal/modules"
#define FMC_MOD_SEARCHPATH_SYSLOCAL "/usr/local/lib/yamal/modules"
#define FMC_MOD_SEARCHPATH_ENV "YAMALCOMPPATH"
#else
#error "Unsupported operating system"
#endif

struct test_component {
  fmc_component_HEAD;
  char *teststr;
  fmc_time64_t timesim;
};

std::string components_path;
struct fmc_component_sys sys;

TEST(component, sys_paths) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_component_sys_init(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);

  fmc_component_sys_paths_set_default(&sys, &err);
  ASSERT_EQ(err, nullptr);
  fmc_component_path_list_t *pdef = fmc_component_sys_paths_get(&sys);
  ASSERT_EQ(sys.modules, nullptr);
  ASSERT_NE(pdef, nullptr);
  EXPECT_EQ(std::string(pdef->path), std::string(FMC_MOD_SEARCHPATH_CUR));
  EXPECT_EQ(std::string(pdef->next->path),
            std::string(getenv("HOME")) +
                std::string("/" FMC_MOD_SEARCHPATH_USRLOCAL));
  EXPECT_EQ(std::string(pdef->next->next->path),
            std::string(FMC_MOD_SEARCHPATH_SYSLOCAL));
  ASSERT_EQ(pdef->next->next->next, nullptr);

  setenv(FMC_MOD_SEARCHPATH_ENV, "/first/path:/second/path", 1);
  fmc_component_sys_paths_set_default(&sys, &err);
  ASSERT_EQ(err, nullptr);
  pdef = fmc_component_sys_paths_get(&sys);
  ASSERT_EQ(sys.modules, nullptr);
  ASSERT_NE(pdef, nullptr);
  EXPECT_EQ(std::string(pdef->path), std::string(FMC_MOD_SEARCHPATH_CUR));
  EXPECT_EQ(std::string(pdef->next->path),
            std::string(getenv("HOME")) +
                std::string("/" FMC_MOD_SEARCHPATH_USRLOCAL));
  EXPECT_EQ(std::string(pdef->next->next->path),
            std::string(FMC_MOD_SEARCHPATH_SYSLOCAL));
  EXPECT_EQ(std::string(pdef->next->next->next->path),
            std::string("/first/path"));
  EXPECT_EQ(std::string(pdef->next->next->next->next->path),
            std::string("/second/path"));
  ASSERT_EQ(pdef->next->next->next->next->next, nullptr);

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
      fmc_component_module_get(&sys, "failcomponent", &err);
  ASSERT_NE(err, nullptr);
  ASSERT_EQ(modfail, nullptr);

  struct fmc_component_module *mod =
      fmc_component_module_get(&sys, "testcomponent", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(mod->sys, &sys);
  ASSERT_EQ(std::string(mod->name), std::string("testcomponent"));
  ASSERT_EQ(sys.modules, mod);
  ASSERT_EQ(sys.modules->prev, mod);

  struct fmc_component_module *samemod =
      fmc_component_module_get(&sys, "testcomponent", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(samemod->sys, &sys);
  ASSERT_EQ(std::string(samemod->name), std::string("testcomponent"));
  ASSERT_EQ(sys.modules, samemod);
  ASSERT_EQ(sys.modules->prev, samemod);
  ASSERT_EQ(sys.modules->prev, samemod);
  ASSERT_EQ(samemod, mod);

  fmc_component_module_del(mod);
  ASSERT_EQ(sys.modules, nullptr);

  fmc_component_sys_destroy(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
}

TEST(component, component) {
  struct fmc_reactor r;
  fmc_reactor_init(&r);
  ASSERT_EQ(r.stop, 0);
  ASSERT_EQ(r.size, 0);
  ASSERT_EQ(r.finishing, 0);
  ASSERT_EQ(r.ctxs, nullptr);

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
      fmc_component_module_get(&sys, "testcomponent", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(mod->sys, &sys);
  ASSERT_EQ(std::string(mod->name), std::string("testcomponent"));
  ASSERT_EQ(sys.modules, mod);
  ASSERT_EQ(sys.modules->prev, mod);

  struct fmc_component_type *tpinvalid =
      fmc_component_module_type_get(mod, "invalid-component", &err);
  ASSERT_NE(err, nullptr);
  ASSERT_EQ(err->code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(tpinvalid, nullptr);

  struct fmc_component_type *tp =
      fmc_component_module_type_get(mod, "testcomponentsched", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_NE(tp, nullptr);

  struct fmc_cfg_sect_item *cfginvalid =
      fmc_cfg_sect_item_add_str(nullptr, "invalidkey", "message", &err);
  ASSERT_EQ(err, nullptr);
  struct fmc_component *compinvalid =
      fmc_component_new(&r, tp, cfginvalid, nullptr, &err);
  ASSERT_NE(err, nullptr);
  ASSERT_EQ(compinvalid, nullptr);
  ASSERT_EQ(r.size, 0);
  ASSERT_EQ(r.ctxs, nullptr);

  struct fmc_cfg_sect_item *cfg =
      fmc_cfg_sect_item_add_str(nullptr, "teststr", "message", &err);
  ASSERT_EQ(err, nullptr);
/*  struct fmc_component *comp = fmc_component_new(&r, tp, cfg, nullptr, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(std::string(comp->_vt->tp_name), std::string("testcomponentsched"));
  ASSERT_EQ(comp->_ctx->err.code, FMC_ERROR_NONE);
  struct test_component *testcomp = (struct test_component *)comp;
  ASSERT_EQ(std::string(testcomp->teststr), std::string("message"));
  ASSERT_TRUE(fmc_time64_equal(testcomp->timesim, fmc_time64_start()));
  ASSERT_EQ(r.size, 1);
  ASSERT_NE(r.ctxs, nullptr);
*/
  fmc_reactor_destroy(&r);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(r.stop, 0);
  ASSERT_EQ(r.size, 0);
  ASSERT_EQ(r.finishing, 0);
  ASSERT_EQ(r.ctxs, nullptr);

  //fmc_component_del(comp);
  fmc_cfg_sect_del(cfginvalid);
  fmc_cfg_sect_del(cfg);

  fmc_component_module_del(mod);
  ASSERT_EQ(sys.modules, nullptr);

  fmc_component_sys_destroy(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
}

TEST(reactor, reactorsched) {
  struct fmc_reactor r;
  fmc_reactor_init(&r);
  ASSERT_EQ(r.stop, 0);
  ASSERT_EQ(r.size, 0);
  ASSERT_EQ(r.finishing, 0);
  ASSERT_EQ(r.ctxs, nullptr);

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
      fmc_component_module_get(&sys, "testcomponent", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(mod->sys, &sys);
  ASSERT_EQ(std::string(mod->name), std::string("testcomponent"));
  ASSERT_EQ(sys.modules, mod);
  ASSERT_EQ(sys.modules->prev, mod);

  struct fmc_component_type *tp =
      fmc_component_module_type_get(mod, "testcomponentsched", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_NE(tp, nullptr);

  struct fmc_cfg_sect_item *cfg =
      fmc_cfg_sect_item_add_str(nullptr, "teststr", "message", &err);
  ASSERT_EQ(err, nullptr);
  struct fmc_component *comp = fmc_component_new(&r, tp, cfg, nullptr, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(std::string(comp->_vt->tp_name), std::string("testcomponentsched"));
  ASSERT_EQ(comp->_ctx->err.code, FMC_ERROR_NONE);
  struct test_component *testcomp = (struct test_component *)comp;
  ASSERT_EQ(std::string(testcomp->teststr), std::string("message"));
  ASSERT_TRUE(fmc_time64_equal(testcomp->timesim, fmc_time64_start()));
  ASSERT_EQ(r.size, 1);
  ASSERT_NE(r.ctxs, nullptr);

  fmc_reactor_run(&r, false, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_time64_equal(
      testcomp->timesim,
      fmc_time64_add(fmc_time64_start(), fmc_time64_from_nanos(100))));
  ASSERT_TRUE(fmc_time64_equal(fmc_reactor_sched(&r), fmc_time64_end()));
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(r.stop, 0);
  ASSERT_EQ(r.size, 1);
  ASSERT_EQ(r.finishing, 0);
  ASSERT_NE(r.ctxs, nullptr);

  fmc_reactor_destroy(&r);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(r.stop, 0);
  ASSERT_EQ(r.size, 0);
  ASSERT_EQ(r.finishing, 0);
  ASSERT_EQ(r.ctxs, nullptr);

  fmc_component_del(comp);
  fmc_cfg_sect_del(cfg);

  fmc_component_module_del(mod);
  ASSERT_EQ(sys.modules, nullptr);

  fmc_component_sys_destroy(&sys);
  ASSERT_EQ(sys.search_paths, nullptr);
  ASSERT_EQ(sys.modules, nullptr);
}

TEST(reactor, reactorlive) {
  struct fmc_reactor r;
  fmc_reactor_init(&r);
  ASSERT_EQ(r.stop, 0);
  ASSERT_EQ(r.size, 0);
  ASSERT_EQ(r.finishing, 0);
  ASSERT_EQ(r.ctxs, nullptr);

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
      fmc_component_module_get(&sys, "testcomponent", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(mod->sys, &sys);
  ASSERT_EQ(std::string(mod->name), std::string("testcomponent"));
  ASSERT_EQ(sys.modules, mod);
  ASSERT_EQ(sys.modules->prev, mod);

  struct fmc_component_type *tp =
      fmc_component_module_type_get(mod, "testcomponentlive", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_NE(tp, nullptr);

  struct fmc_cfg_sect_item *cfg =
      fmc_cfg_sect_item_add_str(nullptr, "teststr", "message", &err);
  ASSERT_EQ(err, nullptr);
  struct fmc_component *comp = fmc_component_new(&r, tp, cfg, nullptr, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(std::string(comp->_vt->tp_name), std::string("testcomponentlive"));
  ASSERT_EQ(comp->_ctx->err.code, FMC_ERROR_NONE);
  struct test_component *testcomp = (struct test_component *)comp;
  ASSERT_EQ(std::string(testcomp->teststr), std::string("message"));
  ASSERT_TRUE(fmc_time64_equal(testcomp->timesim, fmc_time64_start()));
  ASSERT_EQ(r.size, 1);
  ASSERT_NE(r.ctxs, nullptr);

  struct fmc_reactor *rptr = &r;
  std::thread thr([rptr]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    fmc_reactor_stop(rptr);
  });

  fmc_reactor_run(&r, true, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(r.stop, 1);
  ASSERT_EQ(r.size, 1);
  ASSERT_EQ(r.finishing, 0);
  ASSERT_NE(r.ctxs, nullptr);
  thr.join();
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_time64_greater_or_equal(
      testcomp->timesim,
      fmc_time64_add(fmc_time64_start(), fmc_time64_from_nanos(100))));
  ASSERT_TRUE(fmc_time64_equal(fmc_reactor_sched(&r), fmc_time64_end()));

  fmc_reactor_destroy(&r);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(r.stop, 0);
  ASSERT_EQ(r.size, 0);
  ASSERT_EQ(r.finishing, 0);
  ASSERT_EQ(r.ctxs, nullptr);

  fmc_component_del(comp);
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
