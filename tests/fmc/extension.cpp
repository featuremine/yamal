/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file extension.cpp
 * @date 13 Jul 2022
 * @brief File contains tests for extensions
 *
 * @see http://www.featuremine.com
 */

#include <fmc/component.h>
#include <fmc/error.h>
#include <fmc/extension.h>

#include <fmc++/gtestwrap.hpp>

std::string component_path;

TEST(error, extension_simple) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_ext_t ext = fmc_ext_open(component_path.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_NE(ext, nullptr);
  fmc_ext_close(ext);
}

static void components_add(struct fmc_component_module *mod,
                           struct fmc_component_def_v1 *d) {
  return;
}
static struct fmc_component_api api = {.reactor_v1 = nullptr,
                                       .components_add_v1 = components_add};

TEST(error, extension_symbol2) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_ext_t ext = fmc_ext_open(component_path.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_NE(ext, nullptr);
  fmc_component_module_init_func mod_init =
      (fmc_component_module_init_func)fmc_ext_sym(
          ext, "FMCompInit_testcomponent", &err);
  ASSERT_EQ(err, nullptr);
  struct fmc_component_module mod;
  mod_init(&api, &mod);
  fmc_ext_close(ext);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  component_path = argv[1];
  return RUN_ALL_TESTS();
}
