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
 * @file memory.cpp
 * @date 1 Aug 2022
 * @brief File contains tests for memorys
 *
 * @see http://www.featuremine.com
 */

#include <fmc/error.h>
#include <fmc/memory.h>

#include <fmc++/gtestwrap.hpp>
#include <string_view>

TEST(fmc_pool, allocation_no_pools_created) {
  struct pool p;
  fmc_pool_init(&p);
  fmc_pool_destroy(&p);
}

TEST(fmc_pool, owned_view_allocation_pool_cleanup) {
  struct pool p;
  fmc_pool_init(&p);

  size_t sz = 100;
  fmc_error_t *e = nullptr;
  void **view = fmc_pool_allocate(&p, sz, &e);
  ASSERT_NE(view, nullptr);
  ASSERT_NE(*view, nullptr);
  ASSERT_EQ(e, nullptr);

  fmc_pool_destroy(&p);
}

TEST(fmc_pool, not_owned_view_allocation) {
  struct pool p;
  fmc_pool_init(&p);

  std::string_view view = "data string";

  fmc_error_t *e = nullptr;
  void **pview = fmc_pool_view(&p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(pview, nullptr);
  ASSERT_EQ(*pview, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, init_allocation) {
  struct pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct memory mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  fmc_memory_init_alloc(&mem, &p, 100, &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);
  fmc_pool_destroy(&p);
}

TEST(fmc_memory, init_view) {
  struct pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct memory mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  std::string_view view = "valid data";
  fmc_memory_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_EQ(*mem.view, view.data());
  ASSERT_EQ(e, nullptr);

  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);
  fmc_pool_destroy(&p);
}

TEST(fmc_memory, fmc_memory_alloc_copy) {
  struct pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct memory mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  fmc_memory_init_alloc(&mem, &p, 100, &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct memory dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_memory_init_cp(&dest, &mem);
  ASSERT_NE(dest.view, nullptr);
  ASSERT_NE(*dest.view, nullptr);
  ASSERT_EQ(e, nullptr);

  void *old_view = *dest.view;
  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);
  void *new_view = *dest.view;
  ASSERT_EQ(old_view, new_view);
  fmc_memory_destroy(&dest, &e);
  ASSERT_EQ(e, nullptr);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, fmc_memory_view_copy) {
  struct pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct memory mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  std::string_view view = "valid data";
  fmc_memory_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct memory dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_memory_init_cp(&dest, &mem);
  ASSERT_NE(dest.view, nullptr);
  ASSERT_NE(*dest.view, nullptr);
  ASSERT_EQ(e, nullptr);

  void *old_view = *dest.view;
  ASSERT_EQ(old_view, view.data());
  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_NE(old_view, *dest.view);
  fmc_memory_destroy(&dest, &e);
  ASSERT_EQ(e, nullptr);

  fmc_pool_destroy(&p);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
