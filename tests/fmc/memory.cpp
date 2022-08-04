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
  struct fmc_pool_t p;
  fmc_pool_init(&p);
  fmc_pool_destroy(&p);
}

TEST(fmc_memory, init_allocation) {
  struct fmc_pool_t p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_memory_t mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  fmc_memory_init_alloc(&mem, &p, 100, &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node_t *node = (struct fmc_pool_node_t *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);
  fmc_pool_destroy(&p);
}

TEST(fmc_memory, init_view) {
  struct fmc_pool_t p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_memory_t mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  std::string_view view = "valid data";
  fmc_memory_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_EQ(*mem.view, view.data());
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node_t *node = (struct fmc_pool_node_t *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  ASSERT_EQ(p.used, node);
  ASSERT_EQ(p.free, nullptr);

  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_EQ(p.free, node);
  ASSERT_EQ(p.used, nullptr);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, fmc_memory_alloc_copy) {
  struct fmc_pool_t p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_memory_t mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  fmc_memory_init_alloc(&mem, &p, 100, &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node_t *node = (struct fmc_pool_node_t *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  struct fmc_memory_t dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_memory_init_cp(&dest, &mem);
  ASSERT_NE(dest.view, nullptr);
  ASSERT_NE(*dest.view, nullptr);
  ASSERT_EQ(e, nullptr);

  ASSERT_EQ(*mem.view, *dest.view);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 2);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  void *old_view = *mem.view;
  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  ASSERT_EQ(old_view, *dest.view);

  ASSERT_EQ(p.used, node);
  ASSERT_EQ(p.free, nullptr);

  fmc_memory_destroy(&dest, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_EQ(p.free, node);
  ASSERT_EQ(p.used, nullptr);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 0);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, fmc_memory_view_copy) {
  struct fmc_pool_t p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_memory_t mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  std::string_view view = "valid data";
  fmc_memory_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(*mem.view, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node_t *node = (struct fmc_pool_node_t *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  struct fmc_memory_t dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_memory_init_cp(&dest, &mem);
  ASSERT_NE(dest.view, nullptr);
  ASSERT_EQ(*dest.view, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 2);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_NE(*dest.view, (void *)view.data());

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  ASSERT_EQ(view.size(), node->sz);
  ASSERT_EQ(std::string_view((char *)node->buf, node->sz).compare(view), 0);

  ASSERT_EQ(p.used, node);
  ASSERT_EQ(p.free, nullptr);

  fmc_memory_destroy(&dest, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_EQ(p.free, node);
  ASSERT_EQ(p.used, nullptr);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 0);
  ASSERT_EQ(node->next, nullptr);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->prev, nullptr);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  fmc_pool_destroy(&p);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
