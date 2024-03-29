/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file memory.h
 * @date 29 Jul 2022
 * @brief Reactor component API
 *
 * @see http://www.featuremine.com
 */

#include <fmc/error.h>
#include <fmc/math.h>
#include <fmc/memory.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uthash/utlist.h>

struct fmc_pool_node *fmc_get_pool_node(struct fmc_pool *p) {
  struct fmc_pool_node *tmp = NULL;
  if (p->free) {
    tmp = p->free;
    DL_DELETE(p->free, tmp);
  } else {
    tmp = calloc(1, sizeof(*tmp));
    if (!tmp) {
      return NULL;
    }
    tmp->pool = p;
  }
  tmp->count = 1;
  DL_PREPEND(p->used, tmp);
  return tmp;
}

void **fmc_pool_allocate(struct fmc_pool *p, size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);

  struct fmc_pool_node *tmp = fmc_get_pool_node(p);
  if (!tmp) {
    goto cleanup;
  }

  if (tmp->scratch) {
    tmp->buf = tmp->scratch;
    tmp->scratch = NULL;
  }

  void *temp_mem = realloc(tmp->buf, sz);
  if (!temp_mem) {
    goto cleanup;
  }
  tmp->buf = temp_mem;

  tmp->sz = sz;
  return &tmp->buf;
cleanup:
  fmc_error_set2(e, FMC_ERROR_MEMORY);
  if (tmp) {
    DL_DELETE(p->used, tmp);
    DL_PREPEND(p->free, tmp);
  }
  return NULL;
}

void **fmc_pool_view(struct fmc_pool *p, void *view, size_t sz,
                     fmc_error_t **e) {
  fmc_error_clear(e);
  struct fmc_pool_node *tmp = fmc_get_pool_node(p);
  if (!tmp) {
    goto cleanup;
  }

  if (tmp->buf) {
    tmp->scratch = tmp->buf;
  }

  tmp->buf = view;
  tmp->sz = sz;
  return &tmp->buf;
cleanup:
  fmc_error_set2(e, FMC_ERROR_MEMORY);
  if (tmp) {
    DL_DELETE(p->used, tmp);
    DL_PREPEND(p->free, tmp);
  }
  return NULL;
}

void fmc_pool_init(struct fmc_pool *p) {
  p->free = NULL;
  p->used = NULL;
}

void fmc_pool_node_list_destroy(struct fmc_pool_node *node) {
  while (node) {
    if (!node->owner && node->buf) {
      free(node->buf);
    }
    if (node->scratch) {
      free(node->scratch);
    }
    struct fmc_pool_node *next = node->next;
    free(node);
    node = next;
  }
}

void fmc_pool_destroy(struct fmc_pool *p) {
  fmc_pool_node_list_destroy(p->free);
  fmc_pool_node_list_destroy(p->used);
}

void fmc_shmem_init_alloc(struct fmc_shmem *mem, struct fmc_pool *pool,
                          size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  mem->view = fmc_pool_allocate(pool, sz, e);
}

void fmc_shmem_init_view(struct fmc_shmem *mem, struct fmc_pool *pool, void *v,
                         size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  mem->view = fmc_pool_view(pool, v, sz, e);
  struct fmc_pool_node *p = (struct fmc_pool_node *)mem->view;
  p->owner = mem;
}

void fmc_shmem_init_share(struct fmc_shmem *dest, struct fmc_shmem *src) {
  struct fmc_pool_node *p = (struct fmc_pool_node *)src->view;
  ++p->count;
  dest->view = src->view;
}

void fmc_shmem_init_clone(struct fmc_shmem *dest, struct fmc_shmem *src,
                          fmc_error_t **e) {
  struct fmc_pool_node *p = (struct fmc_pool_node *)src->view;
  dest->view = fmc_pool_allocate(p->pool, p->sz, e);
  if (e) {
    return;
  }
  memcpy(*dest->view, *src->view, p->sz);
}

void fmc_shmem_destroy(struct fmc_shmem *mem, fmc_error_t **e) {
  fmc_error_clear(e);
  struct fmc_pool_node *p = (struct fmc_pool_node *)mem->view;
  if (--p->count) {
    if (p->owner == mem) {
      void *tmp = realloc(p->scratch, p->sz);
      if (!tmp) {
        ++p->count;
        fmc_error_set2(e, FMC_ERROR_MEMORY);
        return;
      }
      p->scratch = NULL;
      memcpy(tmp, p->buf, p->sz);
      p->buf = tmp;
      p->owner = NULL;
    }
  } else {
    DL_DELETE(p->pool->used, p);
    DL_PREPEND(p->pool->free, p);
    if (p->owner) {
      p->buf = NULL;
    }
    p->owner = NULL;
  }
}

void fmc_pool_node_realloc(struct fmc_pool_node *p, size_t sz,
                           fmc_error_t **e) {
  fmc_error_clear(e);
  void *tmp = realloc(p->owner ? p->scratch : p->buf, sz);
  if (!tmp)
    goto cleanup;
  if (p->owner)
    memcpy(tmp, p->buf, FMC_MIN(sz, p->sz));
  p->owner = NULL;
  p->sz = sz;
  p->scratch = NULL;
  p->buf = tmp;
  return;
cleanup:
  fmc_error_set2(e, FMC_ERROR_MEMORY);
}

void fmc_shmem_realloc(struct fmc_shmem *mem, size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  struct fmc_pool_node *p = (struct fmc_pool_node *)mem->view;
  fmc_pool_node_realloc(p, sz, e);
}

size_t fmc_shmem_sz(struct fmc_shmem *mem) {
  return ((struct fmc_pool_node *)mem->view)->sz;
}
