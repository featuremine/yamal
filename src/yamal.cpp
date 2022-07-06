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

#include <stdbool.h> 
#include <apr.h> // apr_size_t APR_DECLARE
#include <apr_general.h> // apr_initialize() APR_ALIGN
#include <apr_pools.h> // apr_pool_t
#include <apr_file_io.h> // apr_file_t apr_file_info_get()
#include <apr_mmap.h> // apr_mmap_create()
#include <ytp/errno.h> // ytp_status_t
#include <ytp/yamal.h> // ytp_status_t
#include <atomic> // std::atomic_fetch_add
#include <chrono>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <cstring> // std::memset
#include "yamal.hpp" // struct ytp_yamal
#include "utils.hpp"
#include "endianness.h" // htoye64()

#if !defined(YTP_USE_BIG_ENDIAN)
#define ye64toh(x) _le64toh(x)
#define htoye64(x) _htole64(x)
#if _BYTE_ORDER == _LITTLE_ENDIAN
#define DIRECT_BYTE_ORDER
#endif
#else
#define ye64toh(x) _be64toh(x)
#define htoye64(x) _htobe64(x)
#if _BYTE_ORDER == _BIG_ENDIAN
#define DIRECT_BYTE_ORDER
#endif
#endif

using namespace std::chrono_literals;

static const apr_size_t fm_mmlist_page_count = YAMAL_PAGES;

typedef struct mmnode fm_mmnode_t;
typedef apr_size_t mmnode_offs;
typedef struct fm_mmlist fm_mmlist_t;

struct mmnode {
  mmnode(apr_size_t sz) : size(htoye64(sz)) {}
  std::atomic<apr_size_t> size;
  std::atomic<mmnode_offs> next = 0;
  std::atomic<mmnode_offs> prev = 0;
  char data[];
};

static_assert(sizeof(mmnode) == 24);

static const char magic_number[8] = {'Y', 'A', 'M', 'A', 'L', '0', '0', '0'};

static const apr_size_t fm_mmlist_page_sz = YTP_MMLIST_PAGE_SIZE;

static std::mutex mutex_init;

template <typename T>
static bool atomic_expect_or_init(std::atomic<T> &data, T desired) {
  T expected = 0;
  if (!atomic_compare_exchange_weak(&data, &expected, desired)) {
    return expected == desired;
  }
  return true;
}

static std::atomic<mmnode_offs> &cast_iterator(ytp_iterator_t iterator) {
  auto &it = *(std::atomic<mmnode_offs> *)iterator;
  return it;
}

static ytp_status_t allocate_page(ytp_yamal_t *yamal, apr_size_t page) {
  ytp_status_t rv = YTP_STATUS_OK;
  if (!yamal->pages[page]) {
    apr_size_t f_offset = page * fm_mmlist_page_sz;
    if (!yamal->readonly_) {
      apr_finfo_t finfo;
      rv = apr_file_info_get(&finfo, APR_FINFO_SIZE, yamal->f);
      if (rv) {
        return rv;
      }
      if((apr_size_t)finfo.size < f_offset + fm_mmlist_page_sz) {
        rv = apr_file_trunc(yamal->f, f_offset + fm_mmlist_page_sz);
        if (rv) {
          return rv;
        }
      }
    } else {
      apr_finfo_t finfo;
      rv = apr_file_info_get(&finfo, APR_FINFO_SIZE, yamal->f);
      if (rv) {
        return rv;
      }
      if ((apr_size_t)finfo.size < f_offset + fm_mmlist_page_sz) {
        return YTP_STATUS_EEOF;
      }
    }
    apr_int32_t flag = !yamal->readonly_ ? APR_MMAP_READ | APR_MMAP_WRITE : APR_MMAP_READ;
    rv = apr_mmap_create(&(yamal->pages[page]), yamal->f, f_offset, fm_mmlist_page_sz, 
		                     flag, yamal->pool_);
  }
  return rv;
}

static ytp_status_t get_mapped_memory(ytp_yamal_t *yamal, void **mem, mmnode_offs offs) {
  ytp_status_t rv = YTP_STATUS_OK;
  apr_size_t loffs = ye64toh(offs);
  apr_size_t page = loffs / fm_mmlist_page_sz;
  apr_size_t mem_offset = loffs % fm_mmlist_page_sz;
  std::lock_guard<std::mutex> lock(yamal->pa_mutex_);
  if (!yamal->pages[page]) {
    rv = allocate_page(yamal, page);
    if (rv) {
      return rv;
    }
  }
  *mem = (char *)yamal->pages[page]->mm + mem_offset;
  return rv;
}

static ytp_status_t mmnode_get1(ytp_yamal_t *yamal, fm_mmnode_t **node, mmnode_offs offs) {
  return get_mapped_memory(yamal, (void **)node, offs);
}

static ytp_status_t header(ytp_yamal_t *yamal, fm_mmnode_t **node) {
  return mmnode_get1(yamal, node, 0);
}

static ytp_status_t mmnode_it_back(ytp_yamal_t *yamal, std::atomic<mmnode_offs> **back) {
  fm_mmnode_t *hdr = nullptr;
  ytp_status_t rv = header(yamal, &hdr);
  if (rv) {
    return rv;
  }
  *back = &(hdr->prev);
  return rv;
}

static ytp_status_t mmnode_next1(ytp_yamal_t *yamal, fm_mmnode_t **next, fm_mmnode_t *node) {
  if (!node->next) {
    *next = nullptr;
    return YTP_STATUS_OK;
  }
  return mmnode_get1(yamal, next, node->next);
}

static fm_mmnode_t *mmnode_node_from_data(void *data) {
  return (fm_mmnode_t *)(((char *)data) - offsetof(fm_mmnode_t, data));
}

static ytp_status_t mmlist_pages_allocation1(ytp_yamal_t *yamal) {
  std::atomic<mmnode_offs> *back;
  ytp_status_t rv = mmnode_it_back(yamal, &back);
  if (rv) {
    return rv;
  }
  auto last_node_off = back->load();
  fm_mmnode_t *node = nullptr;
  rv = mmnode_get1(yamal, &node, last_node_off);
  if (rv) {
    return rv;
  }
  auto yamal_size = ye64toh(last_node_off) + ye64toh(node->size.load());
  auto pred_yamal_size = yamal_size + YTP_MMLIST_PREALLOC_SIZE;
  auto pred_page_idx = pred_yamal_size / fm_mmlist_page_sz;

  std::lock_guard<std::mutex> lock(yamal->pa_mutex_);
  if (!yamal->pages[pred_page_idx]) {
    auto page_idx = pred_page_idx;
    for (; !yamal->pages[page_idx]; page_idx--)
      ;
    for (++page_idx; page_idx <= pred_page_idx; page_idx++) {
      rv = allocate_page(yamal, page_idx);
      if (rv) {
        return rv;
      }
    }
  }
  return rv;
}

static ytp_status_t mmlist_sync1(ytp_yamal_t *yamal) {
  ytp_status_t rv = YTP_STATUS_OK;
  for (unsigned long int i = 0; i < fm_mmlist_page_count; ++i) {
    if (yamal->pages[i]) {
      rv = mmap_sync(yamal->pages[i], fm_mmlist_page_sz);
      if(rv) {
        return rv;
      }
    }
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_initialize(void) {
  std::lock_guard<std::mutex> lock(mutex_init);
  return apr_initialize();
}

APR_DECLARE(void) ytp_terminate(void) {
  std::lock_guard<std::mutex> lock(mutex_init);
  return apr_terminate();
}

APR_DECLARE(ytp_status_t) ytp_yamal_init(ytp_yamal_t *yamal, apr_file_t *f) {
  return ytp_yamal_init2(yamal, f, true);
}

APR_DECLARE(ytp_status_t) ytp_yamal_new(ytp_yamal_t **yamal, apr_file_t *f) {
  return ytp_yamal_new2(yamal, f, true);
}

APR_DECLARE(ytp_status_t) ytp_yamal_init2(ytp_yamal_t *yamal, apr_file_t *f, bool enable_thread) {
  ytp_status_t rv = apr_pool_create(&(yamal->pool_), NULL);
  if (rv) {
    return rv;
  }
  yamal->f = f;
  yamal->done_ = false;
  apr_int32_t flags = apr_file_flags_get(f);
  yamal->readonly_ = (flags & APR_FOPEN_WRITE) == 0;
  std::memset(yamal->pages, 0, sizeof(yamal->pages));
  fm_mmnode_t *hdr = nullptr;
  rv = header(yamal, &hdr);
  if (rv) {
    (void)ytp_yamal_destroy(yamal);
    return rv;
  }
  if (hdr) {
    auto hdr_sz = sizeof(fm_mmnode_t) + sizeof(magic_number);
    auto &data_ptr = *(std::atomic<apr_size_t> *)(&hdr->data);
    if (yamal->readonly_) {
      if (data_ptr.load() != *(uint64_t *)magic_number) {
        (void)ytp_yamal_destroy(yamal);
        return YTP_STATUS_EINVFORMAT;
      }
    } else {
      atomic_expect_or_init<apr_size_t>(hdr->size, htoye64(hdr_sz));
      if (!atomic_expect_or_init<apr_size_t>(data_ptr, *(uint64_t *)magic_number)) {
        (void)ytp_yamal_destroy(yamal);
        return YTP_STATUS_EINVFORMAT;
      }
      rv = mmlist_pages_allocation1(yamal);
      if (rv) {
        (void)ytp_yamal_destroy(yamal);
        return rv;
      }
      if (enable_thread) {
        yamal->thread_ = std::thread([yamal]() {
          while (!yamal->done_) {
            std::unique_lock<std::mutex> sl_(yamal->m_);
            if (yamal->cv_.wait_for(sl_, 10ms) != std::cv_status::timeout)
              break;

            (void)mmlist_pages_allocation1(yamal);
            (void)mmlist_sync1(yamal);
          }
        });
      }
    }
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_new2(ytp_yamal_t **yamal, apr_file_t *f, bool enable_thread) {
  *yamal = new ytp_yamal_t;
  ytp_status_t rv = ytp_yamal_init2(*yamal, f, enable_thread);
  if (rv) {
    delete *yamal;
    *yamal = nullptr;
  }
  return rv;
}

APR_DECLARE(apr_file_t *) ytp_yamal_file(ytp_yamal_t *yamal) {
  return yamal->f;
}

APR_DECLARE(ytp_status_t) ytp_yamal_reserve(ytp_yamal_t *yamal, char **buf, 
                                            apr_size_t bufsize) {
  if (bufsize == 0) {
    return YTP_STATUS_EINVSIZE;
  }
  if (yamal->readonly_) {
    return YTP_STATUS_EREADONLY;
  }

  apr_size_t node_size = APR_ALIGN(sizeof(mmnode) + bufsize, APR_SIZEOF_VOIDP);  
  fm_mmnode_t *hdr = nullptr;
  ytp_status_t rv = header(yamal, &hdr);
  if (rv) {
    return rv;
  }
  apr_size_t old_reserve = 0;
  do {
#ifdef DIRECT_BYTE_ORDER
    old_reserve = std::atomic_fetch_add(&hdr->size, node_size);
#else
    apr_size_t val;
    apr_size_t expected = hdr->size.load();
    do {
      old_reserve = ye64toh(expected);
      val = old_reserve + node_size;
    } while (
        !atomic_compare_exchange_weak(&hdr->size, &expected, htoye64(val)));
#endif
  } while (old_reserve % fm_mmlist_page_sz + node_size > fm_mmlist_page_sz);
  mmnode_offs ptr = htoye64(old_reserve);
  fm_mmnode_t *node_mem = nullptr;
  rv = mmnode_get1(yamal, &node_mem, ptr);
  if (rv) {
    return rv;
  }
  auto *mem = new (node_mem) fm_mmnode_t(bufsize);
  if(!mem) {
    return YTP_STATUS_EMEM;
  }
  std::memset(mem->data, 0, bufsize);
  mem->prev = ptr;
  *buf = mem->data;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_commit(ytp_yamal_t *yamal, ytp_iterator_t *it, void *data) {
  fm_mmnode_t *node = mmnode_node_from_data(data);
  auto offs = node->prev.load();
  fm_mmnode_t *mem = nullptr;
  ytp_status_t rv = mmnode_get1(yamal, &mem, offs);
  if (rv) {
    return rv;
  }
  fm_mmnode_t *hdr = nullptr;
  rv = header(yamal, &hdr);
  if (rv) {
    return rv;
  }
  mmnode_offs last = hdr->prev;
  mmnode_offs next_ptr = last;
  do {
    rv = mmnode_get1(yamal, &node, next_ptr);
    if (rv) {
      return rv;
    }
    while (node->next) {
      last = node->next;
      rv = mmnode_get1(yamal, &node, last);
      if (rv) {
        return rv;
      }
    }
    mem->prev = last;
    next_ptr = 0;
  } while (!atomic_compare_exchange_weak(&node->next, &next_ptr, offs));
  hdr->prev = offs;
  *it = &node->next;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                         apr_size_t *sz, const char **data) {
  auto offset = cast_iterator(iterator).load();
  fm_mmnode_t *node = nullptr;
  ytp_status_t rv = mmnode_get1(yamal, &node, offset);
  if (rv) {
    return rv;
  }
  *data = (const char *)node->data;
  *sz = ye64toh(node->size);
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_destroy(ytp_yamal_t *yamal) {
  ytp_status_t rv = YTP_STATUS_OK;
  if(yamal) {
    {
      std::lock_guard<std::mutex> sl_(yamal->m_);
      yamal->done_ = true;
    }
    if (yamal->thread_.joinable()) {
      yamal->cv_.notify_all();
      yamal->thread_.join();
    }
    // Commented out this code because of a possible bug on APR
    // memory is free now in apr_pool_destroy()
    // for (unsigned long int i = 0; i < fm_mmlist_page_count; ++i) {
    //   if (yamal->pages[i]) {
    //     ytp_status_t r = apr_mmap_delete(yamal->pages[i]);
    //     rv = r != YTP_STATUS_OK ? r : rv;
    //   }
    // }
    apr_pool_destroy(yamal->pool_);
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_del(ytp_yamal_t *yamal) {
  ytp_status_t rv = YTP_STATUS_OK;
  if(yamal) {
    rv = ytp_yamal_destroy(yamal);
    delete yamal;
  }
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_begin(ytp_yamal_t *yamal,
                                         ytp_iterator_t *iterator) {
  fm_mmnode_t *hdr = nullptr;
  ytp_status_t rv = header(yamal, &hdr);
  if (rv) {
    return rv;
  }
  *iterator = (ytp_iterator_t)&(hdr->next);
  return YTP_STATUS_OK;
}

APR_DECLARE(ytp_status_t) ytp_yamal_end(ytp_yamal_t *yamal, ytp_iterator_t *iterator) {
  fm_mmnode_t *hdr = nullptr;
  ytp_status_t rv = header(yamal, &hdr);
  if (rv) {
    return rv;
  }
  auto &mmit = hdr->prev;
  fm_mmnode_t *node = nullptr;
  rv = mmnode_get1(yamal, &node, mmit);
  if (rv) {
    return rv;
  }
  *iterator = (ytp_iterator_t)&(node->next);
  return rv;
}

APR_DECLARE(bool) ytp_yamal_term(ytp_iterator_t iterator) {
  return cast_iterator(iterator) == 0;
}

APR_DECLARE(ytp_status_t) ytp_yamal_next(ytp_yamal_t *yamal,
                                         ytp_iterator_t *it_ptr,
                                         ytp_iterator_t iterator) {
  fm_mmnode_t *node = nullptr;
  ytp_status_t rv = mmnode_get1(yamal, &node, cast_iterator(iterator));
  if (rv) {
    return rv;
  }
  *it_ptr = &node->next;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_prev(ytp_yamal_t *yamal,
                                         ytp_iterator_t *it_ptr,
                                         ytp_iterator_t iterator) {
  auto &it = cast_iterator(iterator);
  constexpr apr_size_t diff = offsetof(mmnode, prev) - offsetof(mmnode, next);
  auto &prev_it = *(std::atomic<mmnode_offs> *)((char *)&it + diff);
  fm_mmnode_t *prev = nullptr;
  ytp_status_t rv = mmnode_get1(yamal, &prev, prev_it.load());
  if (rv) {
    return rv;
  }
  *it_ptr = &prev->next;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_remove(ytp_yamal_t *yamal,
                                           ytp_iterator_t *it_ptr,
                                           ytp_iterator_t iterator) {
  auto c_off = cast_iterator(iterator).load();
  std::unique_lock<std::mutex> sl_(yamal->m_);
  if (!c_off) {
    return YTP_STATUS_EINVOFFSET;
  }
  if (yamal->readonly_) {
    return YTP_STATUS_EREADONLY;
  }
  fm_mmnode_t *curr = nullptr;
  ytp_status_t rv = mmnode_get1(yamal, &curr, c_off);
  if (rv) {
    return rv;
  }

  // Note: fm_mmnode_prev does not work because previous can be header
  // If previous is header, fm_mmnode_prev returns nullptr
  auto prev_off = curr->prev.load();
  fm_mmnode_t *prev = nullptr;
  rv = mmnode_get1(yamal, &prev, prev_off);
  if (rv) {
    return rv;
  }

  // if we fail to do the exchange, something else deleted me:
  if (!atomic_compare_exchange_weak(&prev->next, &c_off, curr->next.load())) {
    return YTP_STATUS_EMEM;
  }
  
  mmnode_offs end = 0UL;
  // if we fail to do the exchange, additional nodes where added to next
  if (atomic_compare_exchange_weak(&curr->next, &end, curr->prev.load())) {
    fm_mmnode_t *hdr = nullptr;
    rv = header(yamal, &hdr);
    if (rv) {
      return rv;
    }
    atomic_compare_exchange_weak(&(hdr->prev), &c_off,
                                 curr->prev.load());
  } else {
    prev->next = curr->next.load();
    fm_mmnode_t *next = nullptr;
    rv = mmnode_next1(yamal, &next, curr);
    if (rv) {
      return rv;
    }
    atomic_compare_exchange_weak(&(next->prev), &c_off, curr->prev.load());
  }
  *it_ptr = (ytp_iterator_t)&(prev->next);
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_seek(ytp_yamal_t *yamal,
                                         ytp_iterator_t *it_ptr,
                                         apr_size_t ptr) {
  fm_mmnode_t *node = nullptr;
  ytp_status_t rv = mmnode_get1(yamal, &node, htoye64(ptr));
  if (rv) {
    return rv;
  }
  *it_ptr = &node->next;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_tell(ytp_yamal_t *yamal,
                                         apr_size_t *ptr,
                                         ytp_iterator_t iterator) {
  fm_mmnode_t *node = nullptr;
  ytp_status_t rv = mmnode_get1(yamal, &node, cast_iterator(iterator).load());
  if (rv) {
    return rv;
  }
  *ptr = ye64toh(node->prev.load());
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_yamal_allocate_page(ytp_yamal_t *yamal, apr_size_t page) {
  return allocate_page(yamal, page);
}

APR_DECLARE(ytp_status_t) ytp_yamal_reserved_size(ytp_yamal_t *yamal, apr_size_t *size) {
  fm_mmnode_t *hdr = nullptr;
  ytp_status_t rv = header(yamal, &hdr);
  if (rv) {
    return rv;
  }
  *size = ye64toh(hdr->size.load());
  return rv;
}