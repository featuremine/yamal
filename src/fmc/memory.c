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

#ifdef __cplusplus
extern "C" {
#endif

void fmc_pool_init(struct pool *p) {
  p->free = NULL;
  p->used = NULL;
}

void fmc_pool_destroy(struct pool *p) {
  struct pool_node *tmp = NULL;
  tmp = p->free;
  while (tmp) {
    if (tmp->owned && tmp->buf)
      free(tmp->buf);
    struct pool_node *next = tmp->next;
    free(tmp);
    tmp = next;
  }
  tmp = p->used;
  while (tmp) {
    if (tmp->owned && tmp->buf)
      free(tmp->buf);
    struct pool_node *next = tmp->next;
    free(tmp);
    tmp = next;
  }
}

void **fmc_pool_allocate(struct pool *p, size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  struct pool_node *tmp = NULL;
  if (p->free) {
    tmp = p->free;
    p->free = tmp->next;
    if (tmp->owned) {
      tmp->buf = realloc(tmp->buf, sz);
    } else {
      tmp = (struct pool_node *)calloc(1, sizeof(*tmp));
      tmp->owned = true;
    }
  } else {
    tmp = (struct pool_node *)calloc(1, sizeof(*tmp));
    if (!tmp)
      goto cleanup;
    tmp->buf = calloc(1, sz);
  }
  if (!tmp->buf)
    goto cleanup;
  tmp->sz = sz;
  tmp->count = 1;
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

void **fmc_pool_view(struct pool *p, void *view, size_t sz,
                           fmc_error_t **e) {
  fmc_error_clear(e);
  struct pool_node *tmp = NULL;
  if (p->free) {
    tmp = p->free;
    p->free = tmp->next;
    if (tmp->owned) {
      free(tmp->buf);
    }
  } else {
    tmp = (struct pool_node *)calloc(1, sizeof(*tmp));
    if (!tmp)
      goto cleanup;
  }
  tmp->owned = false;
  tmp->buf = view;
  tmp->sz = sz;
  tmp->count = 1;
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

void **pool_view(struct pool **p, void *view, size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  // struct pool *tmp = (struct pool *)calloc(1, sizeof(*tmp));
  // if (!tmp)
  //   goto cleanup;
  // tmp->buf = view;
  // tmp->sz = sz;
  // tmp->next = *p;
  // if (*p)
  //   (*p)->prev = tmp;
  // *p = tmp;
  // tmp->count = 1;
  // tmp->owned = false;
  // return &tmp->buf;
  // cleanup:
  //   fmc_error_set2(e, FMC_ERROR_MEMORY);
  //   if (tmp)
  //     free(tmp);
  return NULL;
}

void fmc_pool_free(struct pool_node *p, struct memory *m, fmc_error_t **e) {
  fmc_error_clear(e);
  // --p->count;
  // if (--p->count) {
  //   if (p->owner == m) {
  //     // if (p->owned)
  //     //   return;
  //     void *tmp = malloc(p->sz);
  //     if (!tmp) {
  //       fmc_error_set2(e, FMC_ERROR_MEMORY);
  //       return;
  //     }
  //     memcpy(tmp, p->buf, p->sz);
  //     p->buf = tmp;
  //     // p->owned = true;
  //   }
  // } else {
  //   // if (p->owned) {
  //   //   free(p->buf);
  //   // }
  //   p->next->prev = p->prev;
  //   p->prev->next = p->next;
  //   // free(p);
  // }
}

void memory_init_alloc(struct memory *mem, struct pool **pool, size_t sz,
                       fmc_error_t **e) {
  fmc_error_clear(e);
  // void **view = pool_allocate(pool, sz, e);
  // if (*e)
  //   return;
  // mem->view = view;
  // mem->proxy = false;
}

void memory_init_view(struct memory *mem, struct pool **pool, void *v,
                      size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  // void **view = pool_view(pool, v, sz, e);
  // if (*e)
  //   return;
  // mem->view = view;
  // mem->proxy = true;
}

void memory_init_cp(struct memory *dest, struct memory *src) {
  dest->view = src->view;
}

void memory_destroy(struct memory *mem, fmc_error_t **e) {
  // fmc_error_clear(e);
  // struct pool *p = (struct pool *)mem->view;
  // pool_free(p, mem->proxy, e);
}

#ifdef __cplusplus
}
#endif
