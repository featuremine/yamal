/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file alignment.h
 * @author Maxim Trokhimtchouk
 * @date 4 Aug 2017
 * @brief File contains various utilities used throughout the code
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(FMC_SYS_WIN)
#define FMC_FLOORLOG2(X)                                                       \
  ((unsigned)(8 * sizeof(unsigned long long) - __lzcnt64((X)) - 1))
#else
#define FMC_FLOORLOG2(X)                                                       \
  ((unsigned)(8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))
#endif

#define FMC_POW2BND(X) ((1 << (FMC_FLOORLOG2(X) + 1)) - 1)

#define FMC_WORDSIZE (sizeof(int *))

#define FMC_WORDMASK (FMC_WORDSIZE - 1)

// @todo need to check this
#if defined __SSE__
#define FMC_BLOCKSIZE 64 * 2
#elif defined __SSE2__
#define FMC_BLOCKSIZE 64 * 2
#elif defined __SSE3__
#define FMC_BLOCKSIZE 64 * 2
#elif defined __AVX__
#define FMC_BLOCKSIZE 64 * 4
#elif defined __AVX2__
#define FMC_BLOCKSIZE 64 * 4
#elif defined __AVX512__
#define FMC_BLOCKSIZE 64 * 8
#else
#define FMC_BLOCKSIZE FMC_WORDSIZE
#endif

#define FMC_BLOCKMASK (FMC_BLOCKSIZE - 1)

static inline size_t fmc_hash_combine(size_t seed, size_t hash) {
  seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  return seed;
}

static inline size_t fmc_wordceil(size_t s) {
  const size_t wordmask = FMC_WORDMASK;
  const size_t wordsize = FMC_WORDSIZE;
  return (s & ~wordmask) + wordsize * !!(s & wordmask);
}

static inline size_t fmc_blockceil(size_t s) {
  const size_t blockmask = FMC_BLOCKMASK;
  const size_t blocksize = FMC_BLOCKSIZE;
  return (s & ~blockmask) + blocksize * !!(s & blockmask);
}

#ifdef __cplusplus
}
#endif
