/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
  struct fmc_pool p;
  fmc_pool_init(&p);
  fmc_pool_destroy(&p);
}

TEST(fmc_memory, init_allocation) {
  struct fmc_pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_shmem mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  fmc_shmem_init_alloc(&mem, &p, 100, &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node *node = (struct fmc_pool_node *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  fmc_shmem_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);
  fmc_pool_destroy(&p);
}

TEST(fmc_memory, init_view) {
  struct fmc_pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_shmem mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  std::string_view view = "valid data";
  fmc_shmem_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_EQ(*mem.view, view.data());
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node *node = (struct fmc_pool_node *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  struct fmc_pool_node *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node);

  fmc_shmem_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

  ASSERT_EQ(p.free, node);
  ASSERT_EQ(p.used, nullptr);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, fmc_shmem_alloc_share) {
  struct fmc_pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_shmem mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  fmc_shmem_init_alloc(&mem, &p, 100, &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  struct fmc_pool_node *node = (struct fmc_pool_node *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  struct fmc_shmem dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_shmem_init_share(&dest, &mem);
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
  fmc_shmem_destroy(&mem, &e);
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

  fmc_shmem_destroy(&dest, &e);
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

TEST(fmc_memory, fmc_shmem_alloc_clone) {
  struct fmc_pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_shmem mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  fmc_shmem_init_alloc(&mem, &p, 100, &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  struct fmc_pool_node *node = (struct fmc_pool_node *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  struct fmc_shmem dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_shmem_init_clone(&dest, &mem, &e);
  ASSERT_NE(dest.view, nullptr);
  ASSERT_NE(*dest.view, nullptr);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 2);

  struct fmc_pool_node *dest_node = (struct fmc_pool_node *)dest.view;
  ASSERT_NE(dest_node->buf, nullptr);
  ASSERT_EQ(dest_node->count, 1);
  ASSERT_EQ(dest_node->owner, nullptr);
  ASSERT_NE(dest_node->pool, nullptr);
  ASSERT_EQ(dest_node->scratch, nullptr);
  ASSERT_EQ(dest_node->sz, 100);

  ASSERT_NE(*mem.view, *dest.view);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  void *old_view = *mem.view;
  fmc_shmem_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_NE(dest_node->buf, nullptr);
  ASSERT_EQ(dest_node->count, 1);
  ASSERT_EQ(dest_node->owner, nullptr);
  ASSERT_NE(dest_node->pool, nullptr);
  ASSERT_EQ(dest_node->scratch, nullptr);
  ASSERT_EQ(dest_node->sz, 100);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 0);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_NE(old_view, *dest.view);

  ASSERT_EQ(p.free, node);
  ASSERT_EQ(p.used, dest_node);

  fmc_shmem_destroy(&dest, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_EQ(p.free, dest_node);
  ASSERT_EQ(p.used, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 2);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

  ASSERT_NE(dest_node->buf, nullptr);
  ASSERT_EQ(dest_node->count, 0);
  ASSERT_EQ(dest_node->owner, nullptr);
  ASSERT_NE(dest_node->pool, nullptr);
  ASSERT_EQ(dest_node->scratch, nullptr);
  ASSERT_EQ(dest_node->sz, 100);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 0);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 100);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, fmc_shmem_view_copy) {
  struct fmc_pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_shmem mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  std::string_view view = "valid data";
  fmc_shmem_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(*mem.view, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  struct fmc_pool_node *node = (struct fmc_pool_node *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  struct fmc_shmem dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_shmem_init_share(&dest, &mem);
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

  fmc_shmem_destroy(&mem, &e);
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

  fmc_shmem_destroy(&dest, &e);
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

TEST(fmc_memory, fmc_shmem_view_clone) {
  struct fmc_pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_shmem mem;
  mem.view = nullptr;
  ASSERT_EQ(mem.view, nullptr);

  std::string_view view = "valid data";
  fmc_shmem_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(mem.view, nullptr);
  ASSERT_NE(*mem.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  struct fmc_pool_node *node = (struct fmc_pool_node *)mem.view;
  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  struct fmc_shmem dest;
  dest.view = nullptr;
  ASSERT_EQ(dest.view, nullptr);

  fmc_shmem_init_clone(&dest, &mem, &e);
  ASSERT_NE(dest.view, nullptr);
  ASSERT_NE(*dest.view, nullptr);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 2);

  struct fmc_pool_node *dest_node = (struct fmc_pool_node *)dest.view;
  ASSERT_NE(dest_node->buf, nullptr);
  ASSERT_EQ(dest_node->count, 1);
  ASSERT_EQ(dest_node->owner, nullptr);
  ASSERT_NE(dest_node->pool, nullptr);
  ASSERT_EQ(dest_node->scratch, nullptr);
  ASSERT_EQ(dest_node->sz, 10);

  ASSERT_NE(*mem.view, *dest.view);

  ASSERT_NE(node->buf, nullptr);
  ASSERT_EQ(node->count, 1);
  ASSERT_EQ(node->owner, &mem);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  void *old_view = *mem.view;
  fmc_shmem_destroy(&mem, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_EQ(node->buf, nullptr);
  ASSERT_EQ(node->count, 0);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_NE(old_view, *dest.view);

  ASSERT_EQ(p.free, node);
  ASSERT_EQ(p.used, dest_node);

  fmc_shmem_destroy(&dest, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_EQ(p.free, dest_node);
  ASSERT_EQ(p.used, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 2);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

  ASSERT_EQ(node->buf, nullptr);
  ASSERT_EQ(node->count, 0);
  ASSERT_EQ(node->owner, nullptr);
  ASSERT_EQ(node->pool, &p);
  ASSERT_EQ(node->scratch, nullptr);
  ASSERT_EQ(node->sz, 10);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, multiple_nodes) {
  struct fmc_pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_shmem one;

  std::string_view view = "valid data";
  fmc_shmem_init_view(&one, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(one.view, nullptr);
  ASSERT_NE(*one.view, nullptr);
  ASSERT_EQ(*one.view, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node *tmp;
  size_t counter = 0;
  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  struct fmc_pool_node *node_one = (struct fmc_pool_node *)one.view;
  ASSERT_EQ(node_one->buf, *one.view);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, &one);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  struct fmc_shmem two;

  fmc_shmem_init_alloc(&two, &p, 100, &e);
  ASSERT_NE(two.view, nullptr);
  ASSERT_NE(*two.view, nullptr);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 2);

  void *two_mem = *two.view;

  struct fmc_pool_node *node_two = (struct fmc_pool_node *)two.view;
  ASSERT_EQ(node_two->buf, *two.view);
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

  fmc_shmem_destroy(&two, &e);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  ASSERT_EQ(p.free, node_two);
  ASSERT_EQ(p.used, node_one);

  ASSERT_EQ(node_two->buf, *two.view);
  ASSERT_EQ(node_two->count, 0);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 100);

  ASSERT_EQ(node_one->buf, *one.view);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, &one);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  fmc_shmem_destroy(&one, &e);
  ASSERT_EQ(e, nullptr);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 2);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 0);

  ASSERT_EQ(p.free, node_one);
  ASSERT_EQ(p.free->next, node_two);
  ASSERT_EQ(p.used, nullptr);

  ASSERT_EQ(node_two->buf, *two.view);
  ASSERT_EQ(node_two->count, 0);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 100);

  ASSERT_EQ(node_one->buf, *one.view);
  ASSERT_EQ(node_one->count, 0);
  ASSERT_EQ(node_one->owner, nullptr);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  fmc_shmem_init_view(&one, &p, (void *)view.data(), view.size(), &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, node_two);
  ASSERT_EQ(p.used, node_one);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  fmc_shmem_init_view(&two, &p, (void *)view.data(), view.size(), &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node_two);
  ASSERT_EQ(p.used->next, node_one);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 2);

  ASSERT_EQ(node_two->buf, *two.view);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, &two);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, two_mem);
  ASSERT_EQ(node_two->sz, 10);

  ASSERT_EQ(node_one->buf, *one.view);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, &one);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  fmc_shmem_destroy(&one, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, node_one);
  ASSERT_EQ(p.used, node_two);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  fmc_shmem_destroy(&two, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, node_two);
  ASSERT_EQ(p.free->next, node_one);
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

  fmc_shmem_init_alloc(&one, &p, 100, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, node_one);
  ASSERT_EQ(p.used, node_two);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 1);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 1);

  fmc_shmem_init_alloc(&two, &p, 100, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(p.free, nullptr);
  ASSERT_EQ(p.used, node_one);
  ASSERT_EQ(p.used->next, node_two);

  DL_COUNT(p.free, tmp, counter);
  ASSERT_EQ(counter, 0);
  DL_COUNT(p.used, tmp, counter);
  ASSERT_EQ(counter, 2);

  ASSERT_EQ(node_two->buf, *one.view);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 100);

  ASSERT_EQ(node_one->buf, *two.view);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, nullptr);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 100);

  fmc_pool_destroy(&p);
}

TEST(fmc_memory, resize) {
  struct fmc_pool p;
  fmc_pool_init(&p);

  fmc_error_t *e = nullptr;

  struct fmc_shmem one;

  std::string_view view = "valid data";
  fmc_shmem_init_view(&one, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(one.view, nullptr);
  ASSERT_NE(*one.view, nullptr);
  ASSERT_EQ(*one.view, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  struct fmc_shmem two;

  fmc_shmem_init_alloc(&two, &p, 100, &e);
  ASSERT_NE(two.view, nullptr);
  ASSERT_NE(*two.view, nullptr);
  ASSERT_EQ(e, nullptr);

  struct fmc_pool_node *node_one = (struct fmc_pool_node *)one.view;
  ASSERT_NE(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, &one);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 10);

  struct fmc_pool_node *node_two = (struct fmc_pool_node *)two.view;
  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 100);

  // resize allocated memory
  fmc_shmem_realloc(&two, 200, &e);
  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 200);

  // resize view
  fmc_shmem_realloc(&one, 200, &e);
  ASSERT_NE(node_one->buf, nullptr);
  ASSERT_EQ(node_one->count, 1);
  ASSERT_EQ(node_one->owner, nullptr);
  ASSERT_EQ(node_one->pool, &p);
  ASSERT_EQ(node_one->scratch, nullptr);
  ASSERT_EQ(node_one->sz, 200);
  ASSERT_EQ(view.compare(std::string_view((char *)node_one->buf, view.size())),
            0);

  fmc_shmem_destroy(&two, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 0);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 200);

  struct fmc_shmem three;

  fmc_shmem_init_view(&three, &p, (void *)view.data(), view.size(), &e);
  ASSERT_NE(three.view, nullptr);
  ASSERT_NE(*three.view, nullptr);
  ASSERT_EQ(*three.view, (void *)view.data());
  ASSERT_EQ(e, nullptr);

  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, &three);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_NE(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 10);

  fmc_shmem_realloc(&three, 200, &e);

  ASSERT_NE(node_two->buf, nullptr);
  ASSERT_EQ(node_two->count, 1);
  ASSERT_EQ(node_two->owner, nullptr);
  ASSERT_EQ(node_two->pool, &p);
  ASSERT_EQ(node_two->scratch, nullptr);
  ASSERT_EQ(node_two->sz, 200);
  ASSERT_EQ(view.compare(std::string_view((char *)node_two->buf, view.size())),
            0);

  fmc_pool_destroy(&p);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
