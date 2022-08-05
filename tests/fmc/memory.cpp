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

#include <uthash/utlist.h>

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
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
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
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  struct fmc_pool_node_t *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node);

  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

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

  struct fmc_pool_node_t *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  struct fmc_pool_node_t *node = (struct fmc_pool_node_t *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  struct fmc_memory_t dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_memory_init_cp(&dest, &mem);
  ASSERT_NE(dest.view, nullptr);
  ASSERT_NE(*dest.view, nullptr);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_EQ(*mem.view, *dest.view);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 2);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  void *old_view = *mem.view;
  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_EQ(old_view, *dest.view);

  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node);

  fmc_memory_destroy(&dest, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_EQ(p.free, node);
  ASSERT_EQ(p.used, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 0);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
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

  struct fmc_pool_node_t *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  struct fmc_pool_node_t *node = (struct fmc_pool_node_t *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  struct fmc_memory_t dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_memory_init_cp(&dest, &mem);
  ASSERT_NE(dest.view, nullptr);
  ASSERT_EQ(*dest.view, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 2);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  fmc_memory_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_NE(*dest.view, (void *)view.data());

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  ASSERT_EQ(view.size(), node->sz);
  ASSERT_EQ(std::string_view((char *)node->buf, node->sz).compare(view), 0);

  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node);

  fmc_memory_destroy(&dest, &e);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

  ASSERT_EQ(p.free, node);
  ASSERT_EQ(p.used, nullptr);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 0);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, multiple_nodes) {
  struct fmc_pool_t p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_memory_t one;
  one.view = nullptr;
  ASSERT_EQ(one.view, nullptr);

  std::string_view view = "valid data";
  fmc_memory_init_view(&one, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(one.view, nullptr);
  ASSERT_NE(*one.view, nullptr);
  ASSERT_EQ(*one.view, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node_t *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  struct fmc_pool_node_t *node_one = (struct fmc_pool_node_t *)one.view;
  ASSERT_NE(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, &one);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  struct fmc_memory_t two;
  two.view = nullptr;
  ASSERT_EQ(two.view, nullptr);

  fmc_memory_init_alloc(&two, &p, 100, &e);
  ASSERT_NE(two.view, nullptr);
  ASSERT_NE(*two.view, nullptr);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 2);

  void *two_mem = *two.view;

  struct fmc_pool_node_t *node_two = (struct fmc_pool_node_t *)two.view;
  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 100);

  ASSERT_NE(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, &one);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node_two);

  fmc_memory_destroy(&two, &e);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_EQ(p.free, node_two);
  ASSERT_EQ(p.used, node_one);

  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 0);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 100);

  ASSERT_NE(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, &one);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  fmc_memory_destroy(&one, &e);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 2);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

  ASSERT_EQ(p.free, node_one);
  ASSERT_EQ(p.used, nullptr);

  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 0);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 100);

  ASSERT_EQ(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 0);
  ASSERT_EQ(node_one->owner, nullptr);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  fmc_memory_init_view(&one, &p, (void *)view.data(), view.size(), &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, node_two);
  ASSERT_EQ(p.used, node_one);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  fmc_memory_init_view(&two, &p, (void *)view.data(), view.size(), &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node_two);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 2);

  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, &two);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, two_mem);
  ASSERT_EQ(node_two->sz, 10);

  ASSERT_NE(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, &one);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  fmc_memory_destroy(&one, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, node_one);
  ASSERT_EQ(p.used, node_two);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  fmc_memory_destroy(&two, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, node_two);
  ASSERT_EQ(p.used, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 2);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

  ASSERT_EQ(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 0);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, two_mem);
  ASSERT_EQ(node_two->sz, 10);

  ASSERT_EQ(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 0);
  ASSERT_EQ(node_one->owner, nullptr);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  fmc_memory_init_alloc(&one, &p, 100, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, node_one);
  ASSERT_EQ(p.used, node_two);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  fmc_memory_init_alloc(&two, &p, 100, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node_one);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 2);

  ASSERT_EQ(node_two->buf, two_mem);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 100);

  ASSERT_NE(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, nullptr);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 100);

  fmc_pool_destroy(&p);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
