#include <fmc/alignment.h>
#include <fmc/endianness.h>
#include <fmc/error.h>
#include <ytp/yamal.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <thread>

#include "yamal.hpp"

#if !defined(YTP_USE_BIG_ENDIAN)
#define ye64toh(x) fmc_le64toh(x)
#define htoye64(x) fmc_htole64(x)
#if FMC_BYTE_ORDER == FMC_LITTLE_ENDIAN
#define DIRECT_BYTE_ORDER
#endif
#else
#define ye64toh(x) fmc_be64toh(x)
#define htoye64(x) fmc_htobe64(x)
#if FMC_BYTE_ORDER == FMC_BIG_ENDIAN
#define DIRECT_BYTE_ORDER
#endif
#endif

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

static_assert(sizeof(mmnode) == 24);

static const char magic_number[8] = {'Y', 'A', 'M', 'A', 'L', '0', '0', '0'};

static const size_t fm_mmlist_page_sz = 1024 * 1024 * 8;
static const size_t BYTES_PER_PERIOD = 1024 * 1024 * 2;

template <typename T>
static bool atomic_expect_or_init(std::atomic<T> &data, T desired) {
  T expected = 0;
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
    if (!page_ptr) {
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
  auto pred_yamal_size = yamal_size + BYTES_PER_PERIOD;
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
  return mmnode_get1(this, 0, err);
}

static std::atomic<mmnode_offs> &cast_iterator(ytp_iterator_t iterator) {
  auto &it = *(std::atomic<mmnode_offs> *)iterator;
  return it;
}

void ytp_yamal_init(ytp_yamal_t *yamal, int fd, fmc_error_t **error) {
  ytp_yamal_init_2(yamal, fd, true, error);
}

ytp_yamal_t *ytp_yamal_new(int fd, fmc_error_t **error) {
  return ytp_yamal_new_2(fd, true, error);
}

void ytp_yamal_init_2(ytp_yamal_t *yamal, int fd, bool enable_thread,
                      fmc_error_t **error) {
  fmc_error_clear(error);
  yamal->fd = fd;
  yamal->done_ = false;
  yamal->readonly_ = fmc_freadonly(fd);
  auto *hdr = yamal->header(error);
  if (hdr) {
    auto hdr_sz = sizeof(fm_mmnode_t) + sizeof(magic_number);
    auto &data_ptr = *(std::atomic<size_t> *)(&hdr->data);
    if (yamal->readonly_) {
      if (data_ptr.load() != *(uint64_t *)magic_number) {
        FMC_ERROR_REPORT(error, "invalid yamal file format");
        return;
      }
    } else {
      atomic_expect_or_init<size_t>(hdr->size, htoye64(hdr_sz));
      if (!atomic_expect_or_init<size_t>(data_ptr, *(uint64_t *)magic_number)) {
        FMC_ERROR_REPORT(error, "invalid yamal file format");
        return;
      }
      mmlist_pages_allocation1(yamal, error);
      if (enable_thread) {
        yamal->thread_ = std::thread([yamal]() {
          fmc_error_t *err;
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
}

ytp_yamal_t *ytp_yamal_new_2(int fd, bool enable_thread, fmc_error_t **error) {
  auto *yamal = new ytp_yamal_t;
  ytp_yamal_init_2(yamal, fd, enable_thread, error);
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
  return cast_iterator(iterator) == 0;
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
