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

struct fmc_shmem_t {
  void **view;
};

struct fmc_pool_t;

struct fmc_pool_node_t {
  void *buf;
  void *scratch;
  struct fmc_shmem_t *owner;
  struct fmc_pool_node_t *prev;
  struct fmc_pool_node_t *next;
  struct fmc_pool_t *pool;
  size_t sz;
  int count;
};

struct fmc_pool_t {
  struct fmc_pool_node_t *used;
  struct fmc_pool_node_t *free;
};

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
FMMODFUNC void fmc_shmem_init_alloc(struct fmc_shmem_t *mem,
                                     struct fmc_pool_t *pool, size_t sz,
                                     fmc_error_t **e);

/**
 * @brief Initialize memory with memory view
 *
 * @param mem pointer to memory structure to be initialized
 * @param pool pointer to pool
 * @param v address of memory view
 * @param sz size of memory view
 * @param e out-parameter for error handling
 */
FMMODFUNC void fmc_shmem_init_view(struct fmc_shmem_t *mem,
                                    struct fmc_pool_t *pool, void *v, size_t sz,
                                    fmc_error_t **e);

/**
 * @brief Share memory view with destination
 *
 * @param dest out-parameter pointer to memory structure to be initialized with
 * copy
 * @param src pointer to memory structure used as source
 */
FMMODFUNC void fmc_shmem_init_share(struct fmc_shmem_t *dest,
                                    struct fmc_shmem_t *src);

/**
 * @brief Clone shared memory
 * Make a deep copy of the source shared memory into a new node
 *
 * @param dest out-parameter pointer to memory structure to be initialized with
 * copy
 * @param src pointer to memory structure used as source
 */
FMMODFUNC void fmc_shmem_init_clone(struct fmc_shmem_t *dest, struct fmc_shmem_t *src, fmc_error_t **e);

/**
 * @brief Destroy memory
 *
 * @param mem pointer to memory to be destroyed
 * @param e out-parameter for error handling
 */
FMMODFUNC void fmc_shmem_destroy(struct fmc_shmem_t *mem, fmc_error_t **e);

/**
 * @brief Resize memory
 *
 * @param mem pointer to memory to be destroyed
 * @param sz desired size of buffer
 * @param e out-parameter for error handling
 */
FMMODFUNC void fmc_shmem_realloc(struct fmc_shmem_t *mem, size_t sz, fmc_error_t **e);

#ifdef __cplusplus
}
#endif
