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

#pragma once

#include <stdatomic.h>

#define atomic_load_cast(a) atomic_load((_Atomic typeof(*(a)) *)(a))
#define atomic_fetch_add_cast(a, b)                                            \
  atomic_fetch_add((_Atomic typeof(*(a)) *)(a), (b))

#define atomic_compare_exchange_weak_check(a, e, d)                            \
  ({                                                                           \
    atomic_compare_exchange_weak(((_Atomic typeof(*(a)) *)a), (e), (d))        \
        ? true                                                                 \
        : *(e) == (d);                                                         \
  })

#define atomic_expect_or_init(a, d)                                            \
  ({                                                                           \
    typeof(*(a)) desired = (d);                                                \
    typeof(*(a)) expected = 0;                                                 \
    atomic_compare_exchange_weak_check((a), &expected, desired);               \
  })
