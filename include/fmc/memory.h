/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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
 * @file memory.h
 * @date 29 Jul 2022
 * @brief Reactor component API
 *
 * @see http://www.featuremine.com
 */

#include <fmc/error.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pool {
    void *buf;
    size_t sz;
    struct pool *prev;
    struct pool *next;
    int count;
    bool owned;
};

struct memory {
    void **view;
    bool proxy;
};

/**
 * @brief Allocates a pool owning a buffer
 *
 * @param p pointer to empty pointer of pool
 * @param sz size of memory buffer to be allocated
 * @param e out-parameter for error handling
 */
FMMODFUNC void **pool_allocate(struct pool **p, size_t sz, fmc_error_t **e);

/**
 * @brief Allocates a pool for a memory view
 *
 * @param p pointer to empty pointer of pool
 * @param view pointer to memory view
 * @param sz size of memory view
 * @param e out-parameter for error handling
 */
FMMODFUNC void **pool_view(struct pool **p, void *view, size_t sz, fmc_error_t **e);

/**
 * @brief Pool takes ownership of memory view
 *
 * @param p pointer to pool
 * @param e out-parameter for error handling
 */
FMMODFUNC void pool_take(struct pool *p, fmc_error_t **e);

/**
 * @brief Deallocate pool
 *
 * @param p pointer to empty pointer of pool
 * @param proxy flag to signal a proxy pool
 * @param e out-parameter for error handling
 */
FMMODFUNC void pool_free(struct pool *p, bool proxy, fmc_error_t **e);

/**
 * @brief Initialize memory with allocated buffer
 *
 * @param mem pointer to memory structure to be initialized
 * @param pool pointer to pointer of empty pool to manage memory view
 * @param sz size of memory buffer to allocate
 * @param e out-parameter for error handling
 */
FMMODFUNC void memory_init_alloc(struct memory *mem, struct pool **pool, size_t sz, fmc_error_t **e);

/**
 * @brief Initialize memory with memory view
 *
 * @param mem pointer to memory structure to be initialized
 * @param pool pointer to pointer of empty pool to manage memory view
 * @param v address of memory view
 * @param sz size of memory view
 * @param e out-parameter for error handling
 */
FMMODFUNC void memory_init_view(struct memory *mem, struct pool **pool, void *v, size_t sz, fmc_error_t **e);

/**
 * @brief Copy memory
 *
 * @param dest out-parameter pointer to memory structure to be initialized with copy
 * @param src pointer to memory structure used as source
 */
FMMODFUNC void memory_init_cp(struct memory *dest, struct memory *src);

/**
 * @brief Destroy memory
 *
 * @param mem pointer to memory to be destroyed
 * @param e out-parameter for error handling
 */
FMMODFUNC void memory_destroy(struct memory *mem, fmc_error_t **e);

#ifdef __cplusplus
}
#endif
