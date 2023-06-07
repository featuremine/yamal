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

#include "atomic.h"
#include "endianess.h"

#include <fmc/alignment.h>
#include <fmc/error.h>
#include <fmc/process.h>
#include <ytp/yamal.h>

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static const char magic_number[8] = {'Y', 'A', 'M', 'A', 'L', '0', '0', '1'};

static void *allocate_page(ytp_yamal_t *yamal, size_t page,
                           fmc_error_t **error) {
  fmc_error_clear(error);
  struct fmc_fview *mem_page = &yamal->pages_[page];
  void *page_ptr = fmc_fview_data(mem_page);

  if (!page_ptr) {
    size_t f_offset = page * YTP_MMLIST_PAGE_SIZE;
    if (!yamal->readonly_) {
      fmc_falloc(yamal->fd_, f_offset + YTP_MMLIST_PAGE_SIZE, error);
      if (*error) {
        return NULL;
      }
    } else {
      size_t size = fmc_fsize(yamal->fd_, error);
      if (*error) {
        return NULL;
      }
      if (size < f_offset + YTP_MMLIST_PAGE_SIZE) {
        FMC_ERROR_REPORT(error, "unexpected EOF");
        return NULL;
      }
    }

    fmc_fview_init(mem_page, YTP_MMLIST_PAGE_SIZE, yamal->fd_, f_offset, error);
    if (*error) {
      return NULL;
    }
    page_ptr = fmc_fview_data(mem_page);
    if (!page_ptr) {
      FMC_ERROR_REPORT(error, "mmap failed");
      return NULL;
    }
  }
  return page_ptr;
}

void ytp_yamal_allocate_page(ytp_yamal_t *yamal, size_t page,
                             fmc_error_t **error) {
  if (page >= YTP_MMLIST_PAGE_COUNT_MAX) {
    FMC_ERROR_REPORT(error, "page index out of range");
    return;
  }
  allocate_page(yamal, page, error);
}

static void *get_mapped_memory(ytp_yamal_t *yamal, ytp_mmnode_offs offs,
                               fmc_error_t **error) {
  fmc_error_clear(error);
  size_t loffs = ye64toh(offs);
  size_t page = loffs / YTP_MMLIST_PAGE_SIZE;
  size_t mem_offset = loffs % YTP_MMLIST_PAGE_SIZE;
  void *page_ptr = fmc_fview_data(&yamal->pages_[page]);
  if (!page_ptr) {
    if (pthread_mutex_lock(&yamal->pa_mutex_) != 0) {
      FMC_ERROR_REPORT(error, "pthread_mutex_lock failed");
      return NULL;
    }
    page_ptr = allocate_page(yamal, page, error);
    if (pthread_mutex_unlock(&yamal->pa_mutex_) != 0) {
      FMC_ERROR_REPORT(error, "pthread_mutex_unlock failed");
      return NULL;
    }
    if (*error) {
      return NULL;
    }
  }
  return (char *)page_ptr + mem_offset;
}

static struct ytp_hdr *ytp_yamal_header(ytp_yamal_t *yamal, fmc_error_t **err) {
  return (struct ytp_hdr *)get_mapped_memory(yamal, 0, err);
}

size_t ytp_yamal_reserved_size(ytp_yamal_t *yamal, fmc_error_t **error) {
  struct ytp_hdr *hdr = ytp_yamal_header(yamal, error);
  if (*error) {
    return 0;
  }

  return ye64toh(atomic_load_cast(&hdr->size));
}

static struct ytp_mmnode *mmnode_from_offset(ytp_yamal_t *yamal,
                                             ytp_mmnode_offs offs,
                                             fmc_error_t **error) {
  return (struct ytp_mmnode *)get_mapped_memory(yamal, offs, error);
}

static struct ytp_mmnode *mmnode_from_data(void *data) {
  return (struct ytp_mmnode *)(((char *)data) -
                               offsetof(struct ytp_mmnode, data));
}

static struct ytp_mmnode *
mmnode_next(ytp_yamal_t *yamal, struct ytp_mmnode *node, fmc_error_t **error) {
  fmc_error_clear(error);
  if (!node->next)
    return NULL;
  return mmnode_from_offset(yamal, node->next, error);
}

static void mmlist_pages_allocation(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);
  struct ytp_hdr *hdr = ytp_yamal_header(yamal, error);
  if (*error) {
    return;
  }

  size_t yamal_size = ye64toh(atomic_load_cast(&hdr->size));
  size_t pred_yamal_size = yamal_size + YTP_MMLIST_PREALLOC_SIZE;
  size_t pred_page_idx = pred_yamal_size / YTP_MMLIST_PAGE_SIZE;

  if (!fmc_fview_data(&yamal->pages_[pred_page_idx])) {
    if (pthread_mutex_lock(&yamal->pa_mutex_) != 0) {
      FMC_ERROR_REPORT(error, "pthread_mutex_lock failed");
      return;
    }
    size_t page_idx = pred_page_idx;
    for (; !fmc_fview_data(&yamal->pages_[page_idx]); page_idx--)
      ;
    for (++page_idx; page_idx <= pred_page_idx; page_idx++) {
      allocate_page(yamal, page_idx, error);
      if (*error) {
        goto cleanup;
      }
    }

  cleanup:
    if (pthread_mutex_unlock(&yamal->pa_mutex_) != 0) {
      FMC_ERROR_REPORT(error, "pthread_mutex_unlock failed");
      return;
    }
  }
}

static bool mmlist_sync1(ytp_yamal_t *yamal, fmc_error_t **err) {
  fmc_error_clear(err);
  for (size_t i = 0; i < YTP_MMLIST_PAGE_COUNT_MAX; ++i) {
    if (fmc_fview_data(&yamal->pages_[i])) {
      fmc_fview_sync(&yamal->pages_[i], YTP_MMLIST_PAGE_SIZE, err);
      if (*err) {
        return false;
      }
    }
  }
  return true;
}

static ytp_mmnode_offs *offset_from_iterator(ytp_iterator_t iterator) {
  return (ytp_mmnode_offs *)iterator;
}

static int *set_yamal_aux_thread_affinity(const int *cpuid, bool toset) {
  static int _id = 0;
  static int *_set = NULL;
  if (toset) {
    if (cpuid) {
      _id = *cpuid;
      _set = &_id;
    } else {
      _set = NULL;
    }
  }
  return _set;
}

void ytp_yamal_clear_aux_thread_affinity() {
  set_yamal_aux_thread_affinity(NULL, true);
}

void ytp_yamal_set_aux_thread_affinity(int cpuid) {
  int value = cpuid;
  set_yamal_aux_thread_affinity(&value, true);
}

void ytp_yamal_init(ytp_yamal_t *yamal, int fd, fmc_error_t **error) {
  ytp_yamal_init_2(yamal, fd, true, error);
}

ytp_yamal_t *ytp_yamal_new(int fd, fmc_error_t **error) {
  return ytp_yamal_new_2(fd, true, error);
}

void ytp_yamal_init_2(ytp_yamal_t *yamal, int fd, bool enable_thread,
                      fmc_error_t **error) {
  ytp_yamal_init_3(yamal, fd, enable_thread, YTP_UNCLOSABLE, error);
}

ytp_yamal_t *ytp_yamal_new_2(int fd, bool enable_thread, fmc_error_t **error) {
  return ytp_yamal_new_3(fd, enable_thread, YTP_UNCLOSABLE, error);
}

static void *ytp_aux_thread(void *closure) {
  ytp_yamal_t *yamal = (ytp_yamal_t *)closure;
  fmc_error_t *error;
  int *cpuid = set_yamal_aux_thread_affinity(NULL, false);
  if (cpuid) {
    fmc_set_cur_affinity(*cpuid, &error);
  }

  if (pthread_mutex_lock(&yamal->m_) != 0) {
    FMC_ERROR_REPORT(&error, "pthread_mutex_lock failed");
    return NULL;
  }
  while (!yamal->done_) {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    uint64_t ns = 10ull * 1000ll * 1000 + spec.tv_nsec;
    spec.tv_nsec += ns % (1000ull * 1000ull * 1000ull);
    spec.tv_sec += ns >= (1000ull * 1000ull * 1000ull);
    if (pthread_cond_timedwait(&yamal->cv_, &yamal->m_, &spec) == ETIMEDOUT) {
      break;
    }
    mmlist_pages_allocation(yamal, &error);
    mmlist_sync1(yamal, &error);
  }
  if (pthread_mutex_unlock(&yamal->m_) != 0) {
    FMC_ERROR_REPORT(&error, "pthread_mutex_unlock failed");
    return NULL;
  }

  return NULL;
}

void ytp_yamal_init_3(ytp_yamal_t *yamal, int fd, bool enable_thread,
                      YTP_CLOSABLE_MODE closable, fmc_error_t **error) {
  fmc_error_clear(error);
  if (pthread_mutex_init(&yamal->m_, NULL) != 0) {
    goto cleanup_0;
  }

  if (pthread_mutex_init(&yamal->pa_mutex_, NULL) != 0) {
    goto cleanup_1;
  }

  if (pthread_cond_init(&yamal->cv_, NULL) != 0) {
    goto cleanup_2;
  }

  memset(yamal->pages_, 0, sizeof(yamal->pages_));
  yamal->fd_ = fd;
  yamal->done_ = false;
  yamal->readonly_ = fmc_freadonly(fd);
  yamal->thread_created_ = false;

  struct ytp_hdr *hdr = ytp_yamal_header(yamal, error);
  if (*error) {
    goto cleanup_3;
  }
  size_t hdr_sz = sizeof(struct ytp_hdr);
  if (yamal->readonly_) {
    if (atomic_load_cast(&hdr->magic_number) != *(uint64_t *)magic_number) {
      FMC_ERROR_REPORT(error, "invalid yamal file format");
      goto cleanup_4;
    }
    return;
  }

  if (!atomic_expect_or_init(&hdr->magic_number, *(uint64_t *)magic_number)) {
    FMC_ERROR_REPORT(error, "invalid yamal file format");
    goto cleanup_4;
  }

  atomic_expect_or_init(&hdr->size, htoye64(hdr_sz));

  for (size_t lstidx = 0; lstidx < YTP_YAMAL_LISTS; ++lstidx) {
    const uint64_t hdrnode =
        htoye64((ytp_mmnode_offs) & ((struct ytp_hdr *)0)->hdr[lstidx]);
    atomic_expect_or_init(&hdr->hdr[lstidx].prev, hdrnode);
  }

  if (!atomic_expect_or_init(&hdr->closable, closable)) {
    char errormsg[128];
    snprintf(
        errormsg, sizeof(errormsg),
        "configured closable type '%s' differs from file closable type in file",
        (closable == YTP_CLOSABLE) ? "closable" : "unclosable");
    FMC_ERROR_REPORT(error, errormsg);
    goto cleanup_4;
  }

  mmlist_pages_allocation(yamal, error);
  if (*error) {
    goto cleanup_4;
  }

  if (enable_thread) {
    if (pthread_create(&yamal->thread_, NULL, ytp_aux_thread, yamal) != 0) {
      FMC_ERROR_REPORT(error, "unable to create yamal auxiliary thread");
      goto cleanup_4;
    }
    yamal->thread_created_ = true;
  }
  return;

cleanup_4 : {
  struct fmc_error saved_error;
  if (*error) {
    fmc_error_init_mov(&saved_error, *error);
  } else {
    fmc_error_init_none(&saved_error);
  }
  ytp_yamal_destroy(yamal, error);
  if (fmc_error_has(&saved_error)) {
    *error = fmc_error_inst();
    fmc_error_mov(*error, &saved_error);
    fmc_error_destroy(&saved_error);
  }
  return;
}

cleanup_3:
  pthread_cond_destroy(&yamal->cv_);

cleanup_2:
  pthread_mutex_destroy(&yamal->pa_mutex_);

cleanup_1:
  pthread_mutex_destroy(&yamal->m_);

cleanup_0:
  return;
}

ytp_yamal_t *ytp_yamal_new_3(int fd, bool enable_thread,
                             YTP_CLOSABLE_MODE closable, fmc_error_t **error) {
  ytp_yamal_t *yamal =
      (ytp_yamal_t *)aligned_alloc(_Alignof(ytp_yamal_t), sizeof(ytp_yamal_t));
  if (!yamal) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return NULL;
  }

  ytp_yamal_init_3(yamal, fd, enable_thread, closable, error);
  if (*error) {
    free(yamal);
    return NULL;
  }

  return yamal;
}

fmc_fd ytp_yamal_fd(ytp_yamal_t *yamal) { return yamal->fd_; }

char *ytp_yamal_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error) {
  fmc_error_clear(error);
  if (sz == 0) {
    FMC_ERROR_REPORT(error, "size is zero");
    return NULL;
  }
  if (yamal->readonly_) {
    FMC_ERROR_REPORT(error,
                     "unable to reserve using a readonly file descriptor");
    return NULL;
  }
  size_t node_size = fmc_wordceil(sizeof(struct ytp_mmnode) + sz);
  struct ytp_hdr *hdr = ytp_yamal_header(yamal, error);
  if (*error) {
    return NULL;
  }
  size_t old_reserve;
  do {
#ifdef DIRECT_BYTE_ORDER
    old_reserve = atomic_fetch_add_cast(&hdr->size, node_size);
#else
    size_t val;
    size_t expected = atomic_load_cast(&hdr->size);
    do {
      old_reserve = ye64toh(expected);
      val = old_reserve + node_size;
    } while (!atomic_compare_exchange_weak_check(&hdr->size, &expected,
                                                 htoye64(val)));
#endif
  } while (old_reserve % YTP_MMLIST_PAGE_SIZE + node_size >
           YTP_MMLIST_PAGE_SIZE);

  ytp_mmnode_offs ptr = htoye64(old_reserve);
  struct ytp_mmnode *node_mem = mmnode_from_offset(yamal, ptr, error);
  if (*error) {
    FMC_ERROR_REPORT(error, "unable to initialize node in reserved memory");
    return NULL;
  }

  memset(node_mem->data, 0, sz);
  node_mem->size = htoye64(sz);
  node_mem->prev = ptr;
  return node_mem->data;
}

ytp_iterator_t ytp_yamal_commit(ytp_yamal_t *yamal, void *data, size_t lstidx,
                                fmc_error_t **error) {
  struct ytp_mmnode *node = mmnode_from_data(data);
  ytp_mmnode_offs offs = atomic_load_cast(&node->prev);

  struct ytp_mmnode *mem = mmnode_from_offset(yamal, offs, error);
  if (*error)
    return NULL;
  struct ytp_hdr *ytp_hdr = ytp_yamal_header(yamal, error);
  if (*error)
    return NULL;
  struct ytp_mmnode *hdr = &ytp_hdr->hdr[lstidx];
  ytp_mmnode_offs closed =
      htoye64((ytp_mmnode_offs) & ((struct ytp_hdr *)0)->hdr[lstidx]);
  ytp_mmnode_offs last = atomic_load_cast(&hdr->prev);
  ytp_mmnode_offs next_ptr = last;
  do {
    node = mmnode_from_offset(yamal, next_ptr, error);
    if (*error)
      return NULL;
    while ((next_ptr = atomic_load_cast(&node->next)) != 0) {
      if (next_ptr == closed) {
        fmc_error_set2(error, FMC_ERROR_CLOSED);
        return NULL;
      }
      last = next_ptr;
      node = mmnode_from_offset(yamal, last, error);
      if (*error)
        return NULL;
    }
    mem->prev = last;
    mem->seqno = htoye64(ye64toh(node->seqno) + 1);
  } while (!atomic_compare_exchange_weak_check(&node->next, &next_ptr, offs));
  hdr->prev = offs;
  return &node->next;
}

void ytp_yamal_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                    uint64_t *seqno, size_t *size, const char **data,
                    fmc_error_t **error) {
  ytp_mmnode_offs offset = atomic_load_cast(offset_from_iterator(iterator));

  struct ytp_mmnode *node = mmnode_from_offset(yamal, offset, error);
  if (*error) {
    return;
  }

  *data = (const char *)node->data;
  *size = ye64toh(node->size);
  *seqno = ye64toh(node->seqno);
}

void ytp_yamal_destroy(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);
  if (yamal->thread_created_) {
    if (pthread_mutex_lock(&yamal->m_) != 0) {
      FMC_ERROR_REPORT(error, "pthread_mutex_lock failed");
      return;
    }
    yamal->done_ = true;
    if (pthread_mutex_unlock(&yamal->m_) != 0) {
      FMC_ERROR_REPORT(error, "pthread_mutex_unlock failed");
      return;
    }

    pthread_cond_signal(&yamal->cv_);
    if (pthread_join(yamal->thread_, NULL) != 0) {
      FMC_ERROR_REPORT(error, "pthread_join failed");
      return;
    }
  }

  for (unsigned long int i = 0; i < YTP_MMLIST_PAGE_COUNT_MAX; ++i) {
    if (fmc_fview_data(&yamal->pages_[i])) {
      fmc_fview_destroy(&yamal->pages_[i], YTP_MMLIST_PAGE_SIZE, error);
      if (*error) {
        return;
      }
    }
  }

  pthread_cond_destroy(&yamal->cv_);
  pthread_mutex_destroy(&yamal->m_);
  pthread_mutex_destroy(&yamal->pa_mutex_);
}

void ytp_yamal_del(ytp_yamal_t *yamal, fmc_error_t **error) {
  ytp_yamal_destroy(yamal, error);
  free(yamal);
}

ytp_iterator_t ytp_yamal_begin(ytp_yamal_t *yamal, size_t lstidx,
                               fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_error_t *err_in;
  return (ytp_iterator_t)&ytp_yamal_header(yamal, &err_in)->hdr[lstidx].next;
}

ytp_iterator_t ytp_yamal_end(ytp_yamal_t *yamal, size_t lstidx,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  ytp_mmnode_offs offset =
      atomic_load_cast(&ytp_yamal_header(yamal, error)->hdr[lstidx].prev);

  struct ytp_mmnode *node = mmnode_from_offset(yamal, offset, error);

  if (*error) {
    return NULL;
  }

  return (ytp_iterator_t)&node->next;
}

bool ytp_yamal_term(ytp_iterator_t iterator) {
  return ye64toh(atomic_load_cast(offset_from_iterator(iterator))) <
         sizeof(struct ytp_hdr);
}

ytp_iterator_t ytp_yamal_next(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                              fmc_error_t **error) {
  ytp_mmnode_offs offset = atomic_load_cast(offset_from_iterator(iterator));
  struct ytp_mmnode *node = mmnode_from_offset(yamal, offset, error);
  if (*error) {
    return NULL;
  }

  return &node->next;
}

ytp_iterator_t ytp_yamal_prev(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                              fmc_error_t **error) {
  ytp_mmnode_offs *it = offset_from_iterator(iterator);
  const size_t diff =
      offsetof(struct ytp_mmnode, prev) - offsetof(struct ytp_mmnode, next);
  ytp_mmnode_offs *prev_it = (ytp_mmnode_offs *)((char *)it + diff);

  struct ytp_mmnode *prev =
      mmnode_from_offset(yamal, atomic_load_cast(prev_it), error);
  if (*error) {
    return NULL;
  }

  return &prev->next;
}

ytp_iterator_t ytp_yamal_remove(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                fmc_error_t **error) {
  fmc_error_clear(error);

  ytp_iterator_t ret = NULL;
  ytp_mmnode_offs c_off = atomic_load_cast(offset_from_iterator(iterator));

  if (!c_off) {
    FMC_ERROR_REPORT(error, "invalid offset argument");
    return NULL;
  }

  if (yamal->readonly_) {
    FMC_ERROR_REPORT(error,
                     "unable to remove using a readonly file descriptor");
    return NULL;
  }

  struct ytp_mmnode *curr = mmnode_from_offset(yamal, c_off, error);
  if (!curr) {
    FMC_ERROR_REPORT(error, "unable to get node in provided offset");
    return NULL;
  }

  if (pthread_mutex_lock(&yamal->m_) != 0) {
    FMC_ERROR_REPORT(error, "pthread_mutex_lock failed");
    return NULL;
  }

  // Note: fm_mmnode_prev does not work because previous can be header
  // If previous is header, fm_mmnode_prev returns NULL
  ytp_mmnode_offs prev_off = atomic_load_cast(&curr->prev);
  struct ytp_mmnode *prev = mmnode_from_offset(yamal, prev_off, error);
  if (!prev) {
    FMC_ERROR_REPORT(error,
                     "unable to get previous node to node in provided offset");
    goto cleanup;
  }

  // if we fail to do the exchange, something else deleted me:
  if (!atomic_compare_exchange_weak_check(&prev->next, &c_off,
                                          atomic_load_cast(&curr->next))) {
    FMC_ERROR_REPORT(error, "ytp_mmnode already deleted");
    goto cleanup;
  }

  ytp_mmnode_offs end = 0UL;
  // if we fail to do the exchange, additional nodes where added to next
  if (atomic_compare_exchange_weak_check(&curr->next, &end,
                                         atomic_load_cast(&curr->prev))) {
    /*atomic_compare_exchange_weak_check(&ytp_yamal_header(yamal,
       error)->hdr[lstidx].prev, &c_off, atomic_load_cast(&curr->prev));*/
  } else {
    prev->next = atomic_load_cast(&curr->next);
    struct ytp_mmnode *next = mmnode_next(yamal, curr, error);
    if (*error) {
      FMC_ERROR_REPORT(error,
                       "unable to get next node to node in provided offset");
      goto cleanup;
    }
    atomic_compare_exchange_weak_check(&next->prev, &c_off,
                                       atomic_load_cast(&curr->prev));
  }

  ret = (ytp_iterator_t)&prev->next;

cleanup:
  if (pthread_mutex_unlock(&yamal->m_) != 0) {
    FMC_ERROR_REPORT(error, "pthread_mutex_unlock failed");
    return NULL;
  }

  return ret;
}

ytp_iterator_t ytp_yamal_seek(ytp_yamal_t *yamal, ytp_mmnode_offs ptr,
                              fmc_error_t **error) {
  struct ytp_mmnode *node = mmnode_from_offset(yamal, htoye64(ptr), error);
  if (*error) {
    return NULL;
  }

  return &node->next;
}

ytp_mmnode_offs ytp_yamal_tell(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                               fmc_error_t **error) {
  struct ytp_hdr *hdr = ytp_yamal_header(yamal, error);
  if (*error) {
    return 0;
  }

  char *hdr_base = (char *)hdr;
  char *it_addr = (char *)iterator;
  char *hdrs_begin = (char *)&hdr->hdr[0];
  char *hdrs_end = (char *)&hdr->hdr[YTP_YAMAL_LISTS];
  if (hdrs_begin <= it_addr && it_addr < hdrs_end) {
    return (it_addr - offsetof(struct ytp_mmnode, next)) - hdr_base;
  }

  const size_t diff =
      offsetof(struct ytp_mmnode, prev) - offsetof(struct ytp_mmnode, next);
  ytp_mmnode_offs prev_it =
      atomic_load_cast(offset_from_iterator(it_addr + diff));
  struct ytp_mmnode *node = mmnode_from_offset(yamal, prev_it, error);
  if (*error) {
    return 0;
  }

  return ye64toh(atomic_load_cast(&node->next));
}

void ytp_yamal_close(ytp_yamal_t *yamal, size_t lstidx, fmc_error_t **error) {
  fmc_error_clear(error);

  // Validate file is not read only
  if (yamal->readonly_) {
    FMC_ERROR_REPORT(error, "unable to close using a readonly file descriptor");
    return;
  }

  // Validate that sequence is closable
  struct ytp_hdr *hdr = ytp_yamal_header(yamal, error);
  if (*error) {
    return;
  }
  if (atomic_load_cast(&hdr->closable) != YTP_CLOSABLE) {
    FMC_ERROR_REPORT(error, "unable to close a non closable sequence");
    return;
  }

  // Find end and close sequence
  const ytp_mmnode_offs closed =
      htoye64((ytp_mmnode_offs) & ((struct ytp_hdr *)0)->hdr[lstidx]);
  ytp_mmnode_offs last = hdr->hdr[lstidx].prev;
  ytp_mmnode_offs next_ptr = last;
  struct ytp_mmnode *node = NULL;
  do {
    node = mmnode_from_offset(yamal, next_ptr, error);
    if (*error) {
      return;
    }
    while ((next_ptr = atomic_load_cast(&node->next)) != 0) {
      last = next_ptr;
      if (last == closed) {
        return;
      }
      node = mmnode_from_offset(yamal, last, error);
      if (*error) {
        return;
      }
    }
  } while (!atomic_compare_exchange_weak_check(&node->next, &next_ptr, closed));
}

bool ytp_yamal_closed(ytp_yamal_t *yamal, size_t lstidx, fmc_error_t **error) {
  struct ytp_hdr *hdr = ytp_yamal_header(yamal, error);
  if (*error)
    return false;
  ytp_mmnode_offs last = hdr->hdr[lstidx].prev;
  struct ytp_mmnode *node = NULL;
  node = mmnode_from_offset(yamal, last, error);
  if (*error)
    return false;
  const ytp_mmnode_offs closed =
      htoye64((ytp_mmnode_offs) & ((struct ytp_hdr *)0)->hdr[lstidx]);
  ytp_mmnode_offs next_ptr;
  while ((next_ptr = atomic_load_cast(&node->next)) != 0) {
    last = next_ptr;
    if (last == closed) {
      return true;
    }
    node = mmnode_from_offset(yamal, last, error);
    if (*error) {
      return false;
    }
  }
  return false;
}
