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
 * @file priority_queue.cpp
 * @date 3 Aug 2022
 * @brief File contains tests for FMC priority queue
 *
 * @see http://www.featuremine.com
 */

#include <fmc/prio_queue.h>
#include <fmc++/gtestwrap.hpp>
#include <uthash/utheap.h>

TEST(prio_queue, allocation) {
  fmc_prio_queue_t q;
  fmc_prio_queue_init(&q);
  fmc_prio_queue_destroy(&q);
}

TEST(prio_queue, push_pop) {
  fmc_prio_queue_t q;
  fmc_prio_queue_init(&q);

  int res;

  ASSERT_FALSE(fmc_prio_queue_pop(&q, &res));

  fmc_error_t *e;
  fmc_prio_queue_push(&q, 55, &e);
  ASSERT_EQ(e, nullptr);

  ASSERT_TRUE(fmc_prio_queue_pop(&q, &res));
  ASSERT_EQ(res, 55);

  ASSERT_FALSE(fmc_prio_queue_pop(&q, &res));

  fmc_prio_queue_destroy(&q);
}

TEST(prio_queue, prio_order) {
  fmc_prio_queue_t q;
  fmc_prio_queue_init(&q);

  int res;

  fmc_error_t *e;
  fmc_prio_queue_push(&q, 55, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(q.size, 1);
  ASSERT_EQ(q.buffer[0], 55);

  fmc_prio_queue_push(&q, 99, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(q.size, 2);
  ASSERT_EQ(q.buffer[0], 99);
  ASSERT_EQ(q.buffer[1], 55);

  fmc_prio_queue_push(&q, 3, &e);
  ASSERT_EQ(e, nullptr);
  ASSERT_EQ(q.size, 3);
  ASSERT_EQ(q.buffer[0], 99);
  ASSERT_EQ(q.buffer[1], 55);
  ASSERT_EQ(q.buffer[2], 3);

  ASSERT_TRUE(fmc_prio_queue_pop(&q, &res));
  ASSERT_EQ(res, 99);
  ASSERT_EQ(q.size, 2);
  ASSERT_EQ(q.buffer[0], 55);
  ASSERT_EQ(q.buffer[1], 3);

  ASSERT_TRUE(fmc_prio_queue_pop(&q, &res));
  ASSERT_EQ(res, 55);
  ASSERT_EQ(q.size, 1);
  ASSERT_EQ(q.buffer[0], 3);

  ASSERT_TRUE(fmc_prio_queue_pop(&q, &res));
  ASSERT_EQ(res, 3);
  ASSERT_EQ(q.size, 0);

  fmc_prio_queue_destroy(&q);
}

TEST(utheap, heap_push) {
  UT_array a;
  utarray_init(&a, &ut_int_icd);

  int val = 55;
  auto cmp = [](void *a, void *b){
    return *(int*)a < *(int*)b;
  };
  utheap_push(&a, (void*)&val, cmp);

  utarray_done(&a);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
