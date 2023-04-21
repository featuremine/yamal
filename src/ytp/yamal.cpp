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

#include <fmc/alignment.h>
#include <fmc/endianness.h>
#include <fmc/error.h>
#include <fmc/process.h>
#include <ytp/yamal.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <thread>

#include "yamal.hpp"

using namespace std::chrono_literals;

typedef size_t mmnode_offs;
typedef struct fm_mmlist fm_mmlist_t;

struct mmnode {
  mmnode(size_t sz) : size(htoye64(sz)) {}
  std::atomic<size_t> size;
  std::atomic<mmnode_offs> next = 0;
  std::atomic<mmnode_offs> prev = 0;
  char data[];
};

static_assert(sizeof(mmnode) == YTP_MMNODE_HEADER_SIZE);

struct yamal_hdr_t {
  std::atomic<size_t> magic_number;
  std::atomic<FMC_CLOSABLE> closable;
  fm_mmnode_t hdr;
};

static const char magic_number[8] = {'Y', 'A', 'M', 'A', 'L', '0', '0', '1'};

static_assert(sizeof(yamal_hdr_t) == YTP_YAMAL_HEADER_SIZE);

static const size_t fm_mmlist_page_sz = YTP_MMLIST_PAGE_SIZE;

template <typename T>
static bool atomic_expect_or_init(std::atomic<T> &data, T desired) {
  T expected = T(0);
  if (!atomic_compare_exchange_weak(&data, &expected, desired)) {
    return expected == desired;
  }
  return true;
}

static void *allocate_page(ytp_yamal_t *yamal, size_t page,
                           fmc_error_t **error) {
  fmc_error_clear(error);
  auto *mem_page = &yamal->pages[page];
  auto *page_ptr = fmc_fview_data(mem_page);

  if (!page_ptr) {
    size_t f_offset = page * fm_mmlist_page_sz;
    if (!yamal->readonly_) {
      fmc_falloc(yamal->fd, f_offset + fm_mmlist_page_sz, error);
      if (*error) {
        return nullptr;
      }
    } else {
      auto size = fmc_fsize(yamal->fd, error);
      if (*error) {
        return nullptr;
      }
      if (size < f_offset + fm_mmlist_page_sz) {
        FMC_ERROR_REPORT(error, "unexpected EOF");
        return nullptr;
      }
    }

    fmc_fview_init(mem_page, fm_mmlist_page_sz, yamal->fd, f_offset, error);
    if (*error) {
      return nullptr;
    }
    page_ptr = fmc_fview_data(mem_page);
    if (!page_ptr) {
      FMC_ERROR_REPORT(error, "mmap failed");
      return nullptr;
    }
  }
  return page_ptr;
}

void ytp_yamal_allocate_page(ytp_yamal_t *yamal, size_t page,
                             fmc_error_t **error) {
  if (page >= fm_mmlist_page_count) {
    FMC_ERROR_REPORT(error, "page index out of range");
    return;
  }
  allocate_page(yamal, page, error);
}

size_t ytp_yamal_reserved_size(ytp_yamal_t *yamal, fmc_error_t **error) {
  auto *hdr = yamal->header(error);
  if (*error) {
    return 0;
  }

  return ye64toh(hdr->size.load());
}

static void *get_mapped_memory(ytp_yamal_t *yamal, mmnode_offs offs,
                               fmc_error_t **error) {
  fmc_error_clear(error);
  size_t loffs = ye64toh(offs);
  size_t page = loffs / fm_mmlist_page_sz;
  size_t mem_offset = loffs % fm_mmlist_page_sz;
  void *page_ptr = fmc_fview_data(&yamal->pages[page]);
  if (!page_ptr) {
    std::lock_guard<std::mutex> lock(yamal->pa_mutex_);
    page_ptr = allocate_page(yamal, page, error);
    if (*error) {
      return nullptr;
    }
  }
  return (char *)page_ptr + mem_offset;
}

static fm_mmnode_t *mmnode_get1(ytp_yamal_t *yamal, mmnode_offs offs,
                                fmc_error_t **error) {
  return (fm_mmnode_t *)get_mapped_memory(yamal, offs, error);
}

static std::atomic<mmnode_offs> *mmnode_it_back(ytp_yamal_t *yamal) {
  fmc_error_t *error;
  return &yamal->header(&error)->prev;
}

static fm_mmnode_t *mmnode_next1(ytp_yamal_t *yamal, fm_mmnode_t *node,
                                 fmc_error_t **error) {
  fmc_error_clear(error);
  if (!node->next)
    return nullptr;
  return mmnode_get1(yamal, node->next, error);
}

static fm_mmnode_t *mmnode_node_from_data(void *data) {
  return (fm_mmnode_t *)(((char *)data) - offsetof(fm_mmnode_t, data));
}

static bool mmlist_pages_allocation1(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);
  auto last_node_off = mmnode_it_back(yamal)->load();
  auto node = mmnode_get1(yamal, last_node_off, error);
  if (*error) {
    return false;
  }
  auto yamal_size = ye64toh(last_node_off) + ye64toh(node->size.load());
  auto pred_yamal_size = yamal_size + YTP_MMLIST_PREALLOC_SIZE;
  auto pred_page_idx = pred_yamal_size / fm_mmlist_page_sz;

  if (!fmc_fview_data(&yamal->pages[pred_page_idx])) {
    std::lock_guard<std::mutex> lock(yamal->pa_mutex_);
    auto page_idx = pred_page_idx;
    for (; !fmc_fview_data(&yamal->pages[page_idx]); page_idx--)
      ;
    for (++page_idx; page_idx <= pred_page_idx; page_idx++) {
      if (allocate_page(yamal, page_idx, error); *error) {
        return false;
      }
    }
  }
  return true;
}

static bool mmlist_sync1(ytp_yamal_t *yamal, fmc_error_t **err) {
  fmc_error_clear(err);
  for (unsigned long int i = 0; i < fm_mmlist_page_count; ++i) {
    if (fmc_fview_data(&yamal->pages[i])) {
      fmc_fview_sync(&yamal->pages[i], fm_mmlist_page_sz, err);
      if (*err) {
        return false;
      }
    }
  }
  return true;
}

fm_mmnode_t *ytp_yamal_t::header(fmc_error_t **err) {
  return mmnode_get1(this, offsetof(yamal_hdr_t, hdr), err);
}

static std::atomic<mmnode_offs> &cast_iterator(ytp_iterator_t iterator) {
  auto &it = *(std::atomic<mmnode_offs> *)iterator;
  return it;
}

static int *_set_yamal_aux_thread_affinity(int *cpuid, bool toset) {
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
  _set_yamal_aux_thread_affinity(NULL, true);
}

void ytp_yamal_set_aux_thread_affinity(int cpuid) {
  int value = cpuid;
  _set_yamal_aux_thread_affinity(&value, true);
}

void ytp_yamal_init(ytp_yamal_t *yamal, int fd, fmc_error_t **error) {
  ytp_yamal_init_2(yamal, fd, true, error);
}

ytp_yamal_t *ytp_yamal_new(int fd, fmc_error_t **error) {
  return ytp_yamal_new_2(fd, true, error);
}

void ytp_yamal_init_2(ytp_yamal_t *yamal, int fd, bool enable_thread,
                      fmc_error_t **error) {
  ytp_yamal_init_3(yamal, fd, enable_thread, FMC_CLOSABLE::UNCLOSABLE, error);
}

ytp_yamal_t *ytp_yamal_new_2(int fd, bool enable_thread, fmc_error_t **error) {
  return ytp_yamal_new_3(fd, enable_thread, FMC_CLOSABLE::UNCLOSABLE, error);
}

void ytp_yamal_init_3(ytp_yamal_t *yamal, int fd, bool enable_thread,
                      FMC_CLOSABLE closable, fmc_error_t **error) {
  fmc_error_clear(error);
  yamal->fd = fd;
  yamal->done_ = false;
  yamal->readonly_ = fmc_freadonly(fd);
  auto *hdr = (yamal_hdr_t *)mmnode_get1(yamal, 0, error);
  if (*error) {
    fmc_error_t save_error;
    fmc_error_init_mov(&save_error, *error);
    ytp_yamal_destroy(yamal, error);
    *error = fmc_error_inst();
    fmc_error_mov(*error, &save_error);
    return;
  }
  auto hdr_sz = sizeof(yamal_hdr_t);
  if (yamal->readonly_) {
    if (hdr->magic_number.load() != *(uint64_t *)magic_number) {
      ytp_yamal_destroy(yamal, error);
      FMC_ERROR_REPORT(error, "invalid yamal file format");
      return;
    }
  } else {
    atomic_expect_or_init<size_t>(hdr->hdr.size, htoye64(hdr_sz));
    atomic_expect_or_init<size_t>(hdr->hdr.prev, offsetof(yamal_hdr_t, hdr));
    if (!atomic_expect_or_init<size_t>(hdr->magic_number, *(uint64_t *)magic_number)) {
      ytp_yamal_destroy(yamal, error);
      FMC_ERROR_REPORT(error, "invalid yamal file format");
      return;
    }
    if (!atomic_expect_or_init<FMC_CLOSABLE>(hdr->closable, closable)) {
      ytp_yamal_destroy(yamal, error);
      std::string errormsg = "configured closable type ";
      errormsg += (closable == FMC_CLOSABLE::CLOSABLE) ? "'closable'" : "'unclosable'";
      errormsg += " differs from file closable type in file";
      FMC_ERROR_REPORT(error, errormsg.c_str());
      return;
    }
    mmlist_pages_allocation1(yamal, error);
    if (enable_thread) {
      yamal->thread_ = std::thread([yamal]() {
        fmc_error_t *err;
        int *cpuid = _set_yamal_aux_thread_affinity(NULL, false);
        if (cpuid) {
          fmc_set_cur_affinity(*cpuid, &err);
        }
        while (!yamal->done_) {
          std::unique_lock<std::mutex> sl_(yamal->m_);
          if (yamal->cv_.wait_for(sl_, 10ms) != std::cv_status::timeout)
            break;

          mmlist_pages_allocation1(yamal, &err);
          mmlist_sync1(yamal, &err);
        }
      });
    }
  }
}

ytp_yamal_t *ytp_yamal_new_3(int fd, bool enable_thread, FMC_CLOSABLE closable, fmc_error_t **error) {
  auto *yamal = new ytp_yamal_t;
  ytp_yamal_init_3(yamal, fd, enable_thread, closable, error);
  if (!*error) {
    return yamal;
  } else {
    delete yamal;
    return nullptr;
  }
}

fmc_fd ytp_yamal_fd(ytp_yamal_t *yamal) { return yamal->fd; }

char *ytp_yamal_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error) {
  fmc_error_clear(error);
  if (sz == 0) {
    FMC_ERROR_REPORT(error, "size is zero");
    return nullptr;
  }
  if (yamal->readonly_) {
    FMC_ERROR_REPORT(error,
                     "unable to reserve using a readonly file descriptor");
    return nullptr;
  }
  auto node_size = fmc_wordceil(sizeof(mmnode) + sz);
  auto *hdr = yamal->header(error);
  if (*error) {
    return nullptr;
  }
  size_t old_reserve = 0;
  do {
#ifdef DIRECT_BYTE_ORDER
    old_reserve = atomic_fetch_add(&hdr->size, node_size);
#else
    size_t val;
    size_t expected = hdr->size.load();
    do {
      old_reserve = ye64toh(expected);
      val = old_reserve + node_size;
    } while (
        !atomic_compare_exchange_weak(&hdr->size, &expected, htoye64(val)));
#endif
  } while (old_reserve % fm_mmlist_page_sz + node_size > fm_mmlist_page_sz);

  mmnode_offs ptr = htoye64(old_reserve);
  if (auto *node_mem = mmnode_get1(yamal, ptr, error); node_mem) {
    auto *mem = new (node_mem) fm_mmnode_t(sz);
    std::memset(mem->data, 0, sz);
    mem->prev = ptr;
    return mem->data;
  }
  FMC_ERROR_REPORT(error, "unable to initialize node in reserved memory");
  return nullptr;
}

ytp_iterator_t ytp_yamal_commit(ytp_yamal_t *yamal, void *data,
                                fmc_error_t **error) {
  auto *node = mmnode_node_from_data(data);
  auto offs = node->prev.load();

  auto *mem = mmnode_get1(yamal, offs, error);
  if (*error)
    return nullptr;
  auto *hdr = yamal->header(error);
  if (*error)
    return nullptr;
  mmnode_offs last = hdr->prev;
  mmnode_offs next_ptr = last;
  do {
    node = mmnode_get1(yamal, next_ptr, error);
    if (*error)
      return nullptr;
    while (node->next) {
      if (node->next == offsetof(yamal_hdr_t, hdr)) {
        fmc_error_set2(error, FMC_ERROR_CLOSED);
        return nullptr;
      }
      last = node->next;
      node = mmnode_get1(yamal, last, error);
      if (*error)
        return nullptr;
    }
    mem->prev = last;
    next_ptr = 0;
  } while (!atomic_compare_exchange_weak(&node->next, &next_ptr, offs));
  hdr->prev = offs;
  return &node->next;
}

void ytp_yamal_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, size_t *size,
                    const char **data, fmc_error_t **error) {
  auto offset = cast_iterator(iterator).load();

  if (auto *mmnode = mmnode_get1(yamal, offset, error); !*error) {
    *data = (const char *)mmnode->data;
    *size = ye64toh(mmnode->size);
  }
}

void ytp_yamal_destroy(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);
  {
    std::lock_guard<std::mutex> sl_(yamal->m_);
    yamal->done_ = true;
  }
  if (yamal->thread_.joinable()) {
    yamal->cv_.notify_all();
    yamal->thread_.join();
  }
  for (unsigned long int i = 0; i < fm_mmlist_page_count; ++i) {
    if (fmc_fview_data(&yamal->pages[i])) {
      fmc_fview_destroy(&yamal->pages[i], fm_mmlist_page_sz, error);
    }
  }
}

void ytp_yamal_del(ytp_yamal_t *yamal, fmc_error_t **error) {
  ytp_yamal_destroy(yamal, error);
  delete yamal;
}

ytp_iterator_t ytp_yamal_begin(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_error_t *err_in;
  return (ytp_iterator_t)&yamal->header(&err_in)->next;
}

ytp_iterator_t ytp_yamal_end(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);
  if (auto &mmit = yamal->header(error)->prev; !*error) {
    if (auto *node = mmnode_get1(yamal, mmit, error); !*error) {
      return (ytp_iterator_t)&node->next;
    }
  }

  return nullptr;
}

bool ytp_yamal_term(ytp_iterator_t iterator) {
  return cast_iterator(iterator) == 0 || cast_iterator(iterator) == offsetof(yamal_hdr_t, hdr);
}

ytp_iterator_t ytp_yamal_next(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                              fmc_error_t **error) {
  if (auto *node = mmnode_get1(yamal, cast_iterator(iterator), error);
      !*error) {
    return &node->next;
  }
  return nullptr;
}

ytp_iterator_t ytp_yamal_prev(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                              fmc_error_t **error) {
  auto &it = cast_iterator(iterator);
  constexpr size_t diff = offsetof(mmnode, prev) - offsetof(mmnode, next);
  auto &prev_it = *(std::atomic<mmnode_offs> *)((char *)&it + diff);
  if (auto *prev = mmnode_get1(yamal, prev_it.load(), error); !*error) {
    return &prev->next;
  }
  return nullptr;
}

ytp_iterator_t ytp_yamal_remove(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                fmc_error_t **error) {
  auto c_off = cast_iterator(iterator).load();

  fmc_error_clear(error);
  std::unique_lock<std::mutex> sl_(yamal->m_);

  if (!c_off) {
    FMC_ERROR_REPORT(error, "invalid offset argument");
    return nullptr;
  }

  if (yamal->readonly_) {
    FMC_ERROR_REPORT(error,
                     "unable to remove using a readonly file descriptor");
    return nullptr;
  }

  auto *curr = mmnode_get1(yamal, c_off, error);
  if (!curr) {
    FMC_ERROR_REPORT(error, "unable to get node in provided offset");
    return nullptr;
  }

  // Note: fm_mmnode_prev does not work because previous can be header
  // If previous is header, fm_mmnode_prev returns nullptr
  auto prev_off = curr->prev.load();
  auto *prev = mmnode_get1(yamal, prev_off, error);
  if (!prev) {
    FMC_ERROR_REPORT(error,
                     "unable to get previous node to node in provided offset");
    return nullptr;
  }

  // if we fail to do the exchange, something else deleted me:
  if (!atomic_compare_exchange_weak(&prev->next, &c_off, curr->next.load())) {
    FMC_ERROR_REPORT(error, "mmnode already deleted");
    return nullptr;
  }

  mmnode_offs end = 0UL;
  // if we fail to do the exchange, additional nodes where added to next
  if (atomic_compare_exchange_weak(&curr->next, &end, curr->prev.load())) {
    atomic_compare_exchange_weak(&yamal->header(error)->prev, &c_off,
                                 curr->prev.load());
  } else {
    prev->next = curr->next.load();
    fm_mmnode_t *next = mmnode_next1(yamal, curr, error);
    if (*error) {
      FMC_ERROR_REPORT(error,
                       "unable to get next node to node in provided offset");
      return nullptr;
    }
    atomic_compare_exchange_weak(&next->prev, &c_off, curr->prev.load());
  }

  return (ytp_iterator_t)&prev->next;
}

ytp_iterator_t ytp_yamal_seek(ytp_yamal_t *yamal, size_t ptr,
                              fmc_error_t **error) {
  if (auto *node = mmnode_get1(yamal, htoye64(ptr), error); !*error) {
    return &node->next;
  }

  return nullptr;
}

size_t ytp_yamal_tell(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                      fmc_error_t **error) {
  if (auto *node = mmnode_get1(yamal, cast_iterator(iterator).load(), error);
      !*error) {
    return ye64toh(node->prev.load());
  }
  return 0;
}

void ytp_yamal_close(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);

  // Validate file is not read only
  if (yamal->readonly_) {
    FMC_ERROR_REPORT(error,
                     "unable to close using a readonly file descriptor");
    return;
  }

  // Validate that sequence is closable
  auto *hdr = (yamal_hdr_t *)mmnode_get1(yamal, 0, error);
  if (*error) {
    return;
  }
  if (!hdr->closable.load()) {
    FMC_ERROR_REPORT(error,
                     "unable to close a non closable sequence");
    return;
  }

  // Find end and close sequence
  mmnode_offs last = hdr->hdr.prev;
  mmnode_offs next_ptr = last;
  fm_mmnode_t *node = nullptr;
  do {
    node = mmnode_get1(yamal, next_ptr, error);
    if (*error)
      return;
    while (node->next) {
      last = node->next;
      if (last == offsetof(yamal_hdr_t, hdr))
        return;
      node = mmnode_get1(yamal, last, error);
      if (*error)
        return;
    }
    next_ptr = 0;
  } while (!atomic_compare_exchange_weak(&node->next, &next_ptr, offsetof(yamal_hdr_t, hdr)));
}

bool ytp_yamal_closed(ytp_yamal_t *yamal, fmc_error_t **error) {
  auto *hdr = yamal->header(error);
  if (*error)
    return false;
  mmnode_offs last = hdr->prev;
  fm_mmnode_t *node = nullptr;
  node = mmnode_get1(yamal, last, error);
  if (*error)
    return false;
  while (node->next) {
    last = node->next;
    if (last == offsetof(yamal_hdr_t, hdr))
      return true;
    node = mmnode_get1(yamal, last, error);
    if (*error)
      return false;
  }
  return false;
}
