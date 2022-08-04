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

template<typename A, typename I, typename CMP>
void macro_utheap_heapify_down(A &&a, I &&i, CMP&& cmp) {
  do {                                                                                
    size_t idx = i;                                                                   
    while (1) {
      size_t left = 2 * idx;
      size_t right = 2 * idx + 1;
      size_t largest = idx;
      if (left <= (a)->i && cmp(utarray_eltptr(a, left), utarray_eltptr(a, largest))) {
        largest = left;
      }
      if (right <= (a)->i && cmp(utarray_eltptr(a, right), utarray_eltptr(a, largest))) {
        largest = right;
      }
      if (largest == idx) {
        break;
      }
      /*We would use the tmp buffer on our UT_heap here instead of allocating*/       
      void* tmp = calloc(1, (a)->icd.sz);                                             
      if ((a)->icd.copy) {                                                            
        (a)->icd.copy(tmp, utarray_eltptr(a, idx));                                   
      } else {                                                                        
        memcpy(tmp, utarray_eltptr(a, idx), (a)->icd.sz);                             
      };                                                                              
      if ((a)->icd.copy) {                                                            
        (a)->icd.copy(utarray_eltptr(a, idx), utarray_eltptr(a, largest));       
      } else {                                                                        
        memcpy(utarray_eltptr(a, idx), utarray_eltptr(a, largest), (a)->icd.sz); 
      };                                                                              
      if ((a)->icd.copy) {                                                            
        (a)->icd.copy(utarray_eltptr(a, largest), tmp);                          
      } else {                                                                        
        memcpy(utarray_eltptr(a, largest), tmp, (a)->icd.sz);                    
      };                                                                              
      free(tmp);                                                                      
    }
  } while (0);
}

template<typename A, typename I, typename CMP>
void macro_utheap_pop(A &&a, I &&i, CMP&& cmp) {
do {                                                                   
  if ((a)->i) {
    if ((a)->icd.copy) {                                                            
      (a)->icd.copy(utarray_eltptr(a, 0), utarray_eltptr(a, (a)->i - 1));                          
    } else {                                                                        
      memcpy(utarray_eltptr(a, 0), utarray_eltptr(a, (a)->i - 1), (a)->icd.sz);                    
    };                                                                              
    utarray_resize(a, (a)->i);
    macro_utheap_heapify_down(a, 0, cmp);
  }
} while (0);
}

TEST(utheap, heap_push) {
  UT_array a;
  utarray_init(&a, &ut_int_icd);

  auto cmp = [](void *a, void *b){
    return *(int*)a > *(int*)b;
  };

  int val = 55;
  utheap_push(&a, (void*)&val, cmp);
  int* buff = (int*)a.d;
  ASSERT_EQ(buff[0], 55);

  val = 99;
  utheap_push(&a, (void*)&val, cmp);
  buff = (int*)a.d;
  ASSERT_EQ(buff[0], 99);
  ASSERT_EQ(buff[1], 55);

  val = 3;
  utheap_push(&a, (void*)&val, cmp);
  buff = (int*)a.d;
  ASSERT_EQ(buff[0], 99);
  ASSERT_EQ(buff[1], 55);
  ASSERT_EQ(buff[2], 3);

  macro_utheap_pop(&a, (void*)&val, cmp);
  buff = (int*)a.d;
  ASSERT_EQ(buff[0], 55);
  ASSERT_EQ(buff[1], 3);

  macro_utheap_pop(&a, (void*)&val, cmp);
  buff = (int*)a.d;
  ASSERT_EQ(buff[0], 3);

  macro_utheap_pop(&a, (void*)&val, cmp);
  ASSERT_EQ(val, 3);
  
  utarray_done(&a);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
