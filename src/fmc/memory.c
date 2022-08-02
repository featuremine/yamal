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
#include <fmc/memory.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Allocates a pool owning a buffer
 *
 * @param p pointer to empty pointer of pool
 * @param sz size of memory buffer to be allocated
 * @param e out-parameter for error handling
 */
void **fmc_pool_allocate(struct fmc_pool_t *p, size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  struct fmc_pool_node_t *tmp = NULL;
  if (p->free) {
    tmp = p->free;
    p->free = tmp->next;
    if (tmp->owned) {
      tmp->buf = realloc(tmp->buf, sz);
    } else {
      tmp = (struct fmc_pool_node_t *)calloc(1, sizeof(*tmp));
      if (!tmp)
        goto cleanup;
      tmp->pool = p;
      tmp->owned = true;
    }
  } else {
    tmp = (struct fmc_pool_node_t *)calloc(1, sizeof(*tmp));
    if (!tmp)
      goto cleanup;
    tmp->buf = calloc(1, sz);
    tmp->pool = p;
    tmp->owned = true;
  }
  if (!tmp->buf)
    goto cleanup;
  tmp->sz = sz;
  tmp->count = 1;
  tmp->prev = NULL;
  tmp->next = p->used;
  p->used = tmp;
  return &tmp->buf;
cleanup:
  fmc_error_set2(e, FMC_ERROR_MEMORY);
  if (tmp) {
    if (tmp->buf)
      free(tmp->buf);
    free(tmp);
  }
  return NULL;
}

/**
 * @brief Allocates a pool for a memory view
 *
 * @param p pointer to empty pointer of pool
 * @param view pointer to memory view
 * @param sz size of memory view
 * @param e out-parameter for error handling
 */
void **fmc_pool_view(struct fmc_pool_t *p, void *view, size_t sz,
                               fmc_error_t **e) {
  fmc_error_clear(e);
  struct fmc_pool_node_t *tmp = NULL;
  if (p->free) {
    tmp = p->free;
    p->free = tmp->next;
    if (tmp->owned) {
      free(tmp->buf);
      tmp->owned = false;
    }
  } else {
    tmp = (struct fmc_pool_node_t *)calloc(1, sizeof(*tmp));
    if (!tmp)
      goto cleanup;
    tmp->pool = p;
    tmp->owned = false;
  }
  tmp->buf = view;
  tmp->sz = sz;
  tmp->count = 1;
  tmp->prev = NULL;
  tmp->next = p->used;
  p->used = tmp;
  return &tmp->buf;
cleanup:
  fmc_error_set2(e, FMC_ERROR_MEMORY);
  if (tmp) {
    free(tmp);
  }
  return NULL;
}

/**
 * @brief Initializes the pool
 *
 * @param p pointer to pointer of pool
 */
void fmc_pool_init(struct fmc_pool_t *p) {
  p->free = NULL;
  p->used = NULL;
}

/**
 * @brief Destroys the pool
 *
 * @param p pointer to pointer of pool
 */
void fmc_pool_destroy(struct fmc_pool_t *p) {
  struct fmc_pool_node_t *tmp = NULL;
  tmp = p->free;
  while (tmp) {
    if (tmp->owned && tmp->buf)
      free(tmp->buf);
    struct fmc_pool_node_t *next = tmp->next;
    free(tmp);
    tmp = next;
  }
  tmp = p->used;
  while (tmp) {
    if (tmp->owned && tmp->buf)
      free(tmp->buf);
    struct fmc_pool_node_t *next = tmp->next;
    free(tmp);
    tmp = next;
  }
}

/**
 * @brief Initialize memory with allocated buffer
 *
 * @param mem pointer to memory structure to be initialized
 * @param pool pointer to pointer of empty pool to manage memory view
 * @param sz size of memory buffer to allocate
 * @param e out-parameter for error handling
 */
void fmc_memory_init_alloc(struct fmc_memory_t *mem, struct fmc_pool_t *pool,
                           size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  mem->view = fmc_pool_allocate(pool, sz, e);
  struct fmc_pool_node_t *p = (struct fmc_pool_node_t *)mem->view;
  p->owner = mem;
}

/**
 * @brief Initialize memory with memory view
 *
 * @param mem pointer to memory structure to be initialized
 * @param pool pointer to pointer of empty pool to manage memory view
 * @param v address of memory view
 * @param sz size of memory view
 * @param e out-parameter for error handling
 */
void fmc_memory_init_view(struct fmc_memory_t *mem, struct fmc_pool_t *pool, void *v,
                          size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  mem->view = fmc_pool_view(pool, v, sz, e);
  struct fmc_pool_node_t *p = (struct fmc_pool_node_t *)mem->view;
  p->owner = mem;
}

/**
 * @brief Copy memory
 *
 * @param dest out-parameter pointer to memory structure to be initialized with
 * copy
 * @param src pointer to memory structure used as source
 */
void fmc_memory_init_cp(struct fmc_memory_t *dest, struct fmc_memory_t *src) {
  struct fmc_pool_node_t *p = (struct fmc_pool_node_t *)src->view;
  ++p->count;
  dest->view = src->view;
}

/**
 * @brief Destroy memory
 *
 * @param mem pointer to memory to be destroyed
 * @param e out-parameter for error handling
 */
void fmc_memory_destroy(struct fmc_memory_t *mem, fmc_error_t **e) {
  fmc_error_clear(e);
  struct fmc_pool_node_t *p = (struct fmc_pool_node_t *)mem->view;
  if (--p->count) {
    if (p->owner == mem) {
      if (p->owned)
        return;
      void *tmp = malloc(p->sz);
      if (!tmp) {
        fmc_error_set2(e, FMC_ERROR_MEMORY);
        return;
      }
      memcpy(tmp, p->buf, p->sz);
      p->buf = tmp;
      p->owned = true;
    }
  } else {
    if (p->prev)
        p->prev->next = p->next;
    else
        p->pool->used = p->next;
    if (p->next)
        p->next->prev = p->prev;
    if (p->pool->free)
        p->pool->free->prev = p;
    p->prev = NULL;
    p->next = p->pool->free;
    p->pool->free = p;
  }
}
