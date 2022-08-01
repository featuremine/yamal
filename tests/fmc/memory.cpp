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

TEST(fmc_pool, allocation_proxy) {
    struct pool *p = nullptr;
    fmc_error_t *e = nullptr;

    size_t sz = 100;
    pool_allocate(&p, sz, &e);
    ASSERT_NE(p, nullptr);
    ASSERT_EQ(e, nullptr);

    pool_free(p, true, &e);
    ASSERT_EQ(e, nullptr);
}

TEST(fmc_pool, allocation_no_proxy) {
    struct pool *p = nullptr;
    fmc_error_t *e = nullptr;

    size_t sz = 100;
    pool_allocate(&p, sz, &e);
    ASSERT_NE(p, nullptr);
    ASSERT_EQ(e, nullptr);

    pool_free(p, false, &e);
    ASSERT_EQ(e, nullptr);
}

TEST(fmc_pool, view) {
    struct pool *p = nullptr;
    fmc_error_t *e = nullptr;

    size_t sz = 100;
    pool_allocate(&p, sz, &e);

    ASSERT_NE(p, nullptr);
    ASSERT_EQ(e, nullptr);

    std::string_view view = "data string";

    void **pview = pool_view(&p, (void*)view.data(), view.size(), &e);
    ASSERT_NE(pview, nullptr);
    ASSERT_NE(*pview, nullptr);
    ASSERT_EQ(e, nullptr);

    pool_take(p, &e);
    ASSERT_EQ(e, nullptr);

    pool_free(p, true, &e);
    ASSERT_EQ(e, nullptr);
}

TEST(fmc_memory, init_allocation) {
    struct pool *p = nullptr;
    fmc_error_t *e = nullptr;

    struct memory mem;
    mem.view = nullptr;
    ASSERT_EQ(mem.view, nullptr);

    memory_init_alloc(&mem, &p, 100, &e);
    ASSERT_NE(mem.view, nullptr);
    ASSERT_NE(*mem.view, nullptr);
    ASSERT_EQ(mem.proxy, false);
    ASSERT_EQ(e, nullptr);

    memory_destroy(&mem, &e);
    ASSERT_EQ(e, nullptr);
}

TEST(fmc_memory, init_view) {
    struct pool *p = nullptr;
    fmc_error_t *e = nullptr;

    struct memory mem;
    mem.view = nullptr;
    ASSERT_EQ(mem.view, nullptr);

    std::string_view view = "valid data";
    memory_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
    ASSERT_NE(mem.view, nullptr);
    ASSERT_NE(*mem.view, nullptr);
    ASSERT_EQ(mem.proxy, true);
    ASSERT_EQ(e, nullptr);

    memory_destroy(&mem, &e);
    ASSERT_EQ(e, nullptr);
}


TEST(fmc_memory, memory_alloc_copy) {
    struct pool *p = nullptr;
    fmc_error_t *e = nullptr;

    struct memory mem;
    mem.view = nullptr;
    ASSERT_EQ(mem.view, nullptr);

    memory_init_alloc(&mem, &p, 100, &e);
    ASSERT_NE(mem.view, nullptr);
    ASSERT_NE(*mem.view, nullptr);
    ASSERT_EQ(mem.proxy, false);
    ASSERT_EQ(e, nullptr);

    struct memory dest;
    dest.view = nullptr;
    ASSERT_EQ(dest.view, nullptr);

    memory_init_cp(&dest, &mem);
    ASSERT_NE(dest.view, nullptr);
    ASSERT_NE(*dest.view, nullptr);
    ASSERT_EQ(dest.proxy, false);
    ASSERT_EQ(e, nullptr);

    void* old_view = *dest.view;
    memory_destroy(&mem, &e);
    ASSERT_EQ(e, nullptr);
    void* new_view = *dest.view;
    ASSERT_EQ(old_view, new_view);
    memory_destroy(&dest, &e);
    ASSERT_EQ(e, nullptr);
}

TEST(fmc_memory, memory_view_copy) {
    struct pool *p = nullptr;
    fmc_error_t *e = nullptr;

    struct memory mem;
    mem.view = nullptr;
    ASSERT_EQ(mem.view, nullptr);

    std::string_view view = "valid data";
    memory_init_view(&mem, &p, (void *)view.data(), view.size(), &e);
    ASSERT_NE(mem.view, nullptr);
    ASSERT_NE(*mem.view, nullptr);
    ASSERT_EQ(mem.proxy, true);
    ASSERT_EQ(e, nullptr);

    struct memory dest;
    dest.view = nullptr;
    ASSERT_EQ(dest.view, nullptr);

    memory_init_cp(&dest, &mem);
    ASSERT_NE(dest.view, nullptr);
    ASSERT_NE(*dest.view, nullptr);
    ASSERT_EQ(dest.proxy, false);
    ASSERT_EQ(e, nullptr);

    void* old_view = *dest.view;
    ASSERT_EQ(old_view, view.data());
    memory_destroy(&mem, &e);
    ASSERT_EQ(e, nullptr);
    void* new_view = *dest.view;
    ASSERT_NE(old_view, new_view);
    memory_destroy(&dest, &e);
    ASSERT_EQ(e, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
