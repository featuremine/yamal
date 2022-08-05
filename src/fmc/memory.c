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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uthash/utlist.h>

struct fmc_pool_node_t *fmc_get_pool_node(struct fmc_pool_t *p) {
  struct fmc_pool_node_t *tmp = NULL;
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

void **fmc_pool_allocate(struct fmc_pool_t *p, size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);

  struct fmc_pool_node_t *tmp = fmc_get_pool_node(p);
  if (!tmp) {
    goto cleanup;
  }

  if (tmp->scratch) {
    tmp->buf = tmp->scratch;
    tmp->scratch = NULL;
  }

  void* temp_mem = realloc(tmp->buf, sz);
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

void **fmc_pool_view(struct fmc_pool_t *p, void *view, size_t sz,
                     fmc_error_t **e) {
  fmc_error_clear(e);
  struct fmc_pool_node_t *tmp = fmc_get_pool_node(p);
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

void fmc_pool_init(struct fmc_pool_t *p) {
  p->free = NULL;
  p->used = NULL;
}

void fmc_pool_node_list_destroy(struct fmc_pool_node_t *node) {
  while (node) {
    if (!node->owner && node->buf) {
      free(node->buf);
    }
    if (node->scratch) {
      free(node->scratch);
    }
    struct fmc_pool_node_t *next = node->next;
    free(node);
    node = next;
  }
}

void fmc_pool_destroy(struct fmc_pool_t *p) {
  fmc_pool_node_list_destroy(p->free);
  fmc_pool_node_list_destroy(p->used);
}

void fmc_memory_init_alloc(struct fmc_memory_t *mem, struct fmc_pool_t *pool,
                           size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  mem->view = fmc_pool_allocate(pool, sz, e);
}

void fmc_memory_init_view(struct fmc_memory_t *mem, struct fmc_pool_t *pool,
                          void *v, size_t sz, fmc_error_t **e) {
  fmc_error_clear(e);
  mem->view = fmc_pool_view(pool, v, sz, e);
  struct fmc_pool_node_t *p = (struct fmc_pool_node_t *)mem->view;
  p->owner = mem;
}

void fmc_memory_init_cp(struct fmc_memory_t *dest, struct fmc_memory_t *src) {
  struct fmc_pool_node_t *p = (struct fmc_pool_node_t *)src->view;
  ++p->count;
  dest->view = src->view;
}

void fmc_memory_destroy(struct fmc_memory_t *mem, fmc_error_t **e) {
  fmc_error_clear(e);
  struct fmc_pool_node_t *p = (struct fmc_pool_node_t *)mem->view;
  void* tmp = NULL;
  if (--p->count) {
    if (p->owner == mem) {
      if (p->scratch) {
        tmp = realloc(p->scratch, p->sz);
      } else {
        tmp = malloc(p->sz);
      }
      if (!tmp) {
        ++p->count;
        fmc_error_set2(e, FMC_ERROR_MEMORY);
        return;
      }
      if (p->scratch) {
        p->scratch = NULL;
      }
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

void fmc_memory_realloc(struct fmc_memory_t *mem, size_t sz, fmc_error_t **e) {
  struct fmc_pool_node_t *p = (struct fmc_pool_node_t *)mem->view;
  void *tmp_view = NULL;

  if (p->owner) {
    if (p->owner == mem) {
      if (p->scratch) {
        tmp_view = p->buf;
        p->buf = p->scratch;
        p->scratch = NULL;
      } else {
        void *tmp = malloc(p->sz);
        if (!tmp) {
          fmc_error_set2(e, FMC_ERROR_MEMORY);
          return;
        }
        memcpy(tmp, p->buf, p->sz);
        p->buf = tmp;
      }
      p->owner = NULL;
    } else {
      // What to do here
      assert(false);
    }
  }
  void* temp_mem = realloc(p->buf, sz);
  if (!temp_mem) {
    if (tmp_view) {
      p->scratch = p->buf;
      p->buf = tmp_view;
      p->owner = mem;
    }
    fmc_error_set2(e, FMC_ERROR_MEMORY);
    return;
  }
  p->buf = temp_mem;
  p->sz = sz;
}