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

struct fmc_memory_t {
  void **view;
};

struct fmc_pool_t;

struct fmc_pool_node_t {
  void *buf;
  size_t sz;
  int count;
  struct fmc_memory_t *owner;
  bool owned;
  struct fmc_pool_node_t *prev;
  struct fmc_pool_node_t *next;
  struct fmc_pool_t *pool;
};

struct fmc_pool_t {
    struct fmc_pool_node_t *used;
    struct fmc_pool_node_t *free;
};


/**
 * @brief Allocates a view for an owned memory buffer
 *
 * @param p pointer to pool
 * @param sz size of memory buffer to be allocated
 * @param e out-parameter for error handling
 */
FMMODFUNC void **fmc_pool_allocate(struct fmc_pool_t *p, size_t sz, fmc_error_t **e);

/**
 * @brief Allocates a view for an external memory view
 *
 * @param p pointer to pool
 * @param view pointer to memory view
 * @param sz size of memory view
 * @param e out-parameter for error handling
 */
FMMODFUNC void **fmc_pool_view(struct fmc_pool_t *p, void *view, size_t sz,
                           fmc_error_t **e);

/**
 * @brief Initializes the pool
 *
 * @param p pointer to pool
 */
FMMODFUNC void fmc_pool_init(struct fmc_pool_t *p);

/**
 * @brief Destroys the pool
 *
 * @param p pointer to pool
 */
FMMODFUNC void fmc_pool_destroy(struct fmc_pool_t *p);

/**
 * @brief Initialize memory with allocated buffer
 *
 * @param mem pointer to memory structure to be initialized
 * @param pool pointer to pool
 * @param sz size of memory buffer to allocate
 * @param e out-parameter for error handling
 */
FMMODFUNC void fmc_memory_init_alloc(struct fmc_memory_t *mem, struct fmc_pool_t *pool,
                                 size_t sz, fmc_error_t **e);

/**
 * @brief Initialize memory with memory view
 *
 * @param mem pointer to memory structure to be initialized
 * @param pool pointer to pool
 * @param v address of memory view
 * @param sz size of memory view
 * @param e out-parameter for error handling
 */
FMMODFUNC void fmc_memory_init_view(struct fmc_memory_t *mem, struct fmc_pool_t *pool, void *v,
                                size_t sz, fmc_error_t **e);

/**
 * @brief Copy memory
 *
 * @param dest out-parameter pointer to memory structure to be initialized with
 * copy
 * @param src pointer to memory structure used as source
 */
FMMODFUNC void fmc_memory_init_cp(struct fmc_memory_t *dest, struct fmc_memory_t *src);

/**
 * @brief Destroy memory
 *
 * @param mem pointer to memory to be destroyed
 * @param e out-parameter for error handling
 */
FMMODFUNC void fmc_memory_destroy(struct fmc_memory_t *mem, fmc_error_t **e);

#ifdef __cplusplus
}
#endif
