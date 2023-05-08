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

#include <fmc++/error.hpp>
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
  mmnode_offs seqno = 0;
  char data[];
};

static_assert(sizeof(mmnode) == YTP_MMNODE_HEADER_SIZE);

struct yamal_hdr_t {
  std::atomic<size_t> magic_number;
  std::atomic<FMC_CLOSABLE> closable;
  std::atomic<size_t> size;
  mmnode hdr[YTP_YAMAL_LISTS];
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

static mmnode *mmnode_get1(ytp_yamal_t *yamal, mmnode_offs offs,
                           fmc_error_t **error) {
  return (mmnode *)get_mapped_memory(yamal, offs, error);
}

static std::atomic<mmnode_offs> *mmnode_it_back(ytp_yamal_t *yamal,
                                                size_t lstidx) {
  fmc_error_t *error;
  return &yamal->header(&error)->hdr[lstidx].prev;
}

static mmnode *mmnode_next1(ytp_yamal_t *yamal, mmnode *node,
                            fmc_error_t **error) {
  fmc_error_clear(error);
  if (!node->next)
    return nullptr;
  return mmnode_get1(yamal, node->next, error);
}

static mmnode *mmnode_node_from_data(void *data) {
  return (mmnode *)(((char *)data) - offsetof(mmnode, data));
}

static bool mmlist_pages_allocation1(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);
  auto *hdr = yamal->header(error);
  if (*error) {
    return false;
  }
  auto yamal_size = ye64toh(hdr->size.load());
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

yamal_hdr_t *ytp_yamal_t::header(fmc_error_t **err) {
  return (yamal_hdr_t *)get_mapped_memory(this, 0, err);
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
  try {
    new (yamal) ytp_yamal_t(fd, enable_thread, closable);
  }
  catch (fmc::error &e) {
    *error = fmc_error_inst();
    fmc_error_mov(*error, &e);
  }
}

ytp_yamal_t *ytp_yamal_new_3(int fd, bool enable_thread, FMC_CLOSABLE closable,
                             fmc_error_t **error) {
  auto *yamal = static_cast<ytp_yamal_t *>(aligned_alloc(alignof(ytp_yamal_t), sizeof(ytp_yamal_t)));
  if (!yamal) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return {};
  }

  ytp_yamal_init_3(yamal, fd, enable_thread, closable, error);
  if (*error) {
    free(yamal);
    return {};
  }

  return yamal;
}

void ytp_yamal_destroy(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);
  yamal->~ytp_yamal();
}

void ytp_yamal_del(ytp_yamal_t *yamal, fmc_error_t **error) {
  ytp_yamal_destroy(yamal, error);
  if (error) {
    return;
  }

  free(yamal);
}

ytp_yamal::ytp_yamal(int fd, bool enable_thread, FMC_CLOSABLE closable) {
  fmc_error_t *error;

  this->fd = fd;
  done_ = false;
  readonly_ = fmc_freadonly(fd);
  auto *hdr = (yamal_hdr_t *)mmnode_get1(this, 0, &error);
  if (error) {
    fmc::error reterror(*error);
    ytp_yamal_destroy(this, &error);
    throw reterror;
  }
  auto hdr_sz = sizeof(yamal_hdr_t);
  if (readonly_) {
    if (hdr->magic_number.load() != *(uint64_t *)magic_number) {
      ytp_yamal_destroy(this, &error);
      FMC_ERROR_REPORT(&error, "invalid yamal file format");
      throw fmc::error(*error);
    }
  } else {
    if (!atomic_expect_or_init<size_t>(hdr->magic_number,
                                       *(uint64_t *)magic_number)) {
      ytp_yamal_destroy(this, &error);
      FMC_ERROR_REPORT(&error, "invalid yamal file format");
      throw fmc::error(*error);
    }
    atomic_expect_or_init<size_t>(hdr->size, htoye64(hdr_sz));
    for (size_t lstidx = 0; lstidx < YTP_YAMAL_LISTS; ++lstidx) {
      atomic_expect_or_init<size_t>(
          hdr->hdr[lstidx].prev,
          htoye64((mmnode_offs) & ((yamal_hdr_t *)0)->hdr[lstidx]));
    }
    if (!atomic_expect_or_init<FMC_CLOSABLE>(hdr->closable, closable)) {
      ytp_yamal_destroy(this, &error);
      std::string errormsg = "configured closable type ";
      errormsg +=
          (closable == FMC_CLOSABLE::CLOSABLE) ? "'closable'" : "'unclosable'";
      errormsg += " differs from file closable type in file";
      FMC_ERROR_REPORT(&error, errormsg.c_str());
      return;
    }
    mmlist_pages_allocation1(this, &error);
    if (enable_thread) {
      thread_ = std::thread([this]() {
        fmc_error_t *err;
        int *cpuid = _set_yamal_aux_thread_affinity(NULL, false);
        if (cpuid) {
          fmc_set_cur_affinity(*cpuid, &err);
        }
        while (!done_) {
          std::unique_lock<std::mutex> sl_(m_);
          if (cv_.wait_for(sl_, 10ms) != std::cv_status::timeout)
            break;

          mmlist_pages_allocation1(this, &err);
          mmlist_sync1(this, &err);
        }
      });
    }
  }
}

ytp_yamal::~ytp_yamal() {
  fmc_error_t *error;
  {
    std::lock_guard<std::mutex> sl_(m_);
    done_ = true;
  }
  if (thread_.joinable()) {
    cv_.notify_all();
    thread_.join();
  }
  for (unsigned long int i = 0; i < fm_mmlist_page_count; ++i) {
    if (fmc_fview_data(&pages[i])) {
      fmc_fview_destroy(&pages[i], fm_mmlist_page_sz, &error);
    }
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
  size_t old_reserve;
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
  auto *node_mem = mmnode_get1(yamal, ptr, error);
  if (*error) {
    FMC_ERROR_REPORT(error, "unable to initialize node in reserved memory");
    return nullptr;
  }

  auto *mem = new (node_mem) mmnode(sz);
  std::memset(mem->data, 0, sz);
  mem->prev = ptr;
  return mem->data;
}

ytp_iterator_t ytp_yamal_commit(ytp_yamal_t *yamal, void *data, size_t lstidx,
                                fmc_error_t **error) {
  auto *node = mmnode_node_from_data(data);
  auto offs = node->prev.load();

  auto *mem = mmnode_get1(yamal, offs, error);
  if (*error)
    return nullptr;
  auto *yamal_hdr = yamal->header(error);
  if (*error)
    return nullptr;
  auto *hdr = &yamal_hdr->hdr[lstidx];
  mmnode_offs closed = htoye64((mmnode_offs) & ((yamal_hdr_t *)0)->hdr[lstidx]);
  mmnode_offs last = hdr->prev;
  mmnode_offs next_ptr = last;
  do {
    node = mmnode_get1(yamal, next_ptr, error);
    if (*error)
      return nullptr;
    next_ptr = 0;
    while (node->next) {
      if (node->next == closed) {
        fmc_error_set2(error, FMC_ERROR_CLOSED);
        return nullptr;
      }
      last = node->next;
      node = mmnode_get1(yamal, last, error);
      if (*error)
        return nullptr;
    }
    mem->prev = last;
    mem->seqno = htoye64(ye64toh(node->seqno) + 1);
  } while (!atomic_compare_exchange_weak(&node->next, &next_ptr, offs));
  hdr->prev = offs;
  return &node->next;
}

void ytp_yamal_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, size_t *seqno,
                    size_t *size, const char **data, fmc_error_t **error) {
  auto offset = cast_iterator(iterator).load();

  auto *mmnode = mmnode_get1(yamal, offset, error);
  if (*error) {
    return;
  }

  *data = (const char *)mmnode->data;
  *size = ye64toh(mmnode->size);
  *seqno = ye64toh(mmnode->seqno);
}

ytp_iterator_t ytp_yamal_begin(ytp_yamal_t *yamal, size_t lstidx,
                               fmc_error_t **error) {
  fmc_error_clear(error);
  fmc_error_t *err_in;
  return (ytp_iterator_t)&yamal->header(&err_in)->hdr[lstidx].next;
}

ytp_iterator_t ytp_yamal_end(ytp_yamal_t *yamal, size_t lstidx,
                             fmc_error_t **error) {
  fmc_error_clear(error);
  if (auto &mmit = yamal->header(error)->hdr[lstidx].prev; !*error) {
    if (auto *node = mmnode_get1(yamal, mmit, error); !*error) {
      return (ytp_iterator_t)&node->next;
    }
  }

  return nullptr;
}

bool ytp_yamal_term(ytp_iterator_t iterator) {
  return ye64toh(cast_iterator(iterator)) < sizeof(yamal_hdr_t);
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
    /*atomic_compare_exchange_weak(&yamal->header(error)->hdr[lstidx].prev,
       &c_off, curr->prev.load());*/
  } else {
    prev->next = curr->next.load();
    mmnode *next = mmnode_next1(yamal, curr, error);
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
  auto *hdr = yamal->header(error);
  if (*error) {
    return {};
  }

  if (&hdr->hdr[0] <= iterator && iterator < &hdr->hdr[YTP_YAMAL_LISTS]) {
    return (uint64_t)iterator - offsetof(mmnode, next) - (uint64_t)hdr;
  }

  constexpr size_t diff = offsetof(mmnode, prev) - offsetof(mmnode, next);
  auto &prev_it = cast_iterator((char *)iterator + diff);
  if (auto *node = mmnode_get1(yamal, prev_it.load(), error); !*error) {
    return ye64toh(node->next.load());
  }
  return 0;
}

void ytp_yamal_close(ytp_yamal_t *yamal, fmc_error_t **error) {
  fmc_error_clear(error);

  // Validate file is not read only
  if (yamal->readonly_) {
    FMC_ERROR_REPORT(error, "unable to close using a readonly file descriptor");
    return;
  }

  // Validate that sequence is closable
  auto *hdr = (yamal_hdr_t *)mmnode_get1(yamal, 0, error);
  if (*error) {
    return;
  }
  if (hdr->closable.load() != FMC_CLOSABLE::CLOSABLE) {
    FMC_ERROR_REPORT(error, "unable to close a non closable sequence");
    return;
  }

  for (size_t lstidx = 0; lstidx < YTP_YAMAL_LISTS; ++lstidx) {
    // Find end and close sequence
    mmnode_offs closed =
        htoye64((mmnode_offs) & ((yamal_hdr_t *)0)->hdr[lstidx]);
    mmnode_offs last = hdr->hdr[lstidx].prev;
    mmnode_offs next_ptr = last;
    mmnode *node = nullptr;
    do {
      node = mmnode_get1(yamal, next_ptr, error);
      if (*error)
        return;
      next_ptr = 0;
      while (node->next) {
        last = node->next;
        if (last == closed)
          return;
        node = mmnode_get1(yamal, last, error);
        if (*error)
          return;
      }
    } while (!atomic_compare_exchange_weak(&node->next, &next_ptr,
                                           offsetof(yamal_hdr_t, hdr)));
  }
}

bool ytp_yamal_closed(ytp_yamal_t *yamal, std::size_t lstidx,
                      fmc_error_t **error) {
  auto *hdr = yamal->header(error);
  if (*error)
    return false;
  mmnode_offs last = hdr->hdr[lstidx].prev;
  mmnode *node = nullptr;
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
