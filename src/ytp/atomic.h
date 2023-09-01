/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
