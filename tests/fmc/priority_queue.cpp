/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file priority_queue.cpp
 * @date 3 Aug 2022
 * @brief File contains tests for FMC priority queue
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/gtestwrap.hpp>
#include <fmc/prio_queue.h>
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

  int res;

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

  auto cmp = [](void *a, void *b) { return *(int *)a > *(int *)b; };

  int val = 55;
  utheap_push(&a, (void *)&val, cmp);
  int *buff = (int *)a.d;
  ASSERT_EQ(buff[0], 55);

  val = 99;
  utheap_push(&a, (void *)&val, cmp);
  buff = (int *)a.d;
  ASSERT_EQ(buff[0], 99);
  ASSERT_EQ(buff[1], 55);

  val = 3;
  utheap_push(&a, (void *)&val, cmp);
  buff = (int *)a.d;
  ASSERT_EQ(buff[0], 99);
  ASSERT_EQ(buff[1], 55);
  ASSERT_EQ(buff[2], 3);

  utheap_pop(&a, cmp);
  buff = (int *)a.d;
  ASSERT_EQ(buff[0], 55);
  ASSERT_EQ(buff[1], 3);

  utheap_pop(&a, cmp);
  buff = (int *)a.d;
  ASSERT_EQ(buff[0], 3);

  utheap_pop(&a, cmp);
  ASSERT_EQ(val, 3);

  ASSERT_EQ(utarray_front(&a), nullptr);

  utarray_done(&a);
}

TEST(utheap, heap_random) {
  UT_array a;
  utarray_init(&a, &ut_int_icd);

  auto cmp = [](void *a, void *b) { return *(int *)a > *(int *)b; };

  int val = 0;
  int upper = 100;
  int total = 500;
  srand(time(0));
  for (int i = 0; i < total; i++) {
    int inval = rand() % upper;
    utheap_push(&a, (void *)&inval, cmp);
  }

  int last = upper;
  for (int i = 0; i < total; i++) {
    val = *(int *)utarray_front(&a);
    utheap_pop(&a, cmp);
    ASSERT_TRUE(val <= last);
    last = val;
  }

  utarray_done(&a);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
