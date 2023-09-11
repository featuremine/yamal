/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include "cursor.h"
#include "atomic.h"

#include <fmc/alignment.h>
#include <ytp/cursor.h>
#include <ytp/data.h>
#include <ytp/streams.h>
#include <ytp/subscription.h>

#include <uthash/utarray.h>
#include <uthash/uthash.h>

#include <stdlib.h>

#undef utarray_oom
#define utarray_oom()                                                          \
  do {                                                                         \
    fmc_error_set2(error, FMC_ERROR_MEMORY);                                   \
  } while (0)

#undef uthash_fatal
#undef HASH_RECORD_OOM
#define HASH_RECORD_OOM(oomed) fmc_error_set2(error, FMC_ERROR_MEMORY)

static void cb_ann_icd_init(void *elt) {}
static void cb_ann_icd_copy(void *dst, const void *src) {
  memcpy(dst, src, sizeof(struct ytp_cursor_ann_cb_cl_t));
}
static void cb_ann_icd_dtor(void *elt) {}

const UT_icd cb_ann_icd = {
    sizeof(struct ytp_cursor_ann_cb_cl_t),
    cb_ann_icd_init,
    cb_ann_icd_copy,
    cb_ann_icd_dtor,
};

static void cb_data_icd_init(void *elt) {}
static void cb_data_icd_copy(void *dst, const void *src) {
  memcpy(dst, src, sizeof(struct ytp_cursor_data_cb_cl_t));
}
static void cb_data_icd_dtor(void *elt) {}

const UT_icd cb_data_icd = {
    sizeof(struct ytp_cursor_data_cb_cl_t),
    cb_data_icd_init,
    cb_data_icd_copy,
    cb_data_icd_dtor,
};

static unsigned streams_data_hash(ytp_mmnode_offs key) {
  unsigned ret;
  HASH_VALUE(&key, sizeof(ytp_mmnode_offs), ret);
  return ret;
}

static struct ytp_cursor_streams_data_item_t *
streams_data_addhash(struct ytp_cursor_streams_data_item_t **m,
                     ytp_mmnode_offs key, unsigned hash, fmc_error_t **error) {
  fmc_error_clear(error);
  struct ytp_cursor_streams_data_item_t *item =
      malloc(sizeof(struct ytp_cursor_streams_data_item_t));
  if (!item) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return NULL;
  }
  item->stream = key;
  HASH_ADD_KEYPTR_BYHASHVALUE(hh, (*m), &item->stream, sizeof(ytp_mmnode_offs),
                              hash, item);
  if (*error) {
    free(item);
    return NULL;
  }
  return item;
}

static struct ytp_cursor_streams_data_item_t *
streams_data_gethash(struct ytp_cursor_streams_data_item_t *m,
                     ytp_mmnode_offs key, unsigned hash) {
  struct ytp_cursor_streams_data_item_t *item;
  HASH_FIND_BYHASHVALUE(hh, m, &key, sizeof(ytp_mmnode_offs), hash, item);
  return item;
}

struct ytp_cursor_streams_data_item_t *
streams_data_get(struct ytp_cursor_streams_data_item_t *m,
                 ytp_mmnode_offs key) {
  return streams_data_gethash(m, key, streams_data_hash(key));
}

struct ytp_cursor_streams_data_item_t *
streams_data_emplace(struct ytp_cursor_streams_data_item_t **m,
                     ytp_mmnode_offs key, fmc_error_t **error) {
  fmc_error_clear(error);
  unsigned hash = streams_data_hash(key);
  struct ytp_cursor_streams_data_item_t *item =
      streams_data_gethash(*m, key, hash);
  if (item == NULL) {
    item = streams_data_addhash(m, key, hash, error);
    if (*error) {
      return NULL;
    }

    utarray_init(&item->cb_data, &cb_data_icd);
    item->cb_data_locked = 0;
  }
  return item;
}

ytp_cursor_t *ytp_cursor_new(ytp_yamal_t *yamal, fmc_error_t **error) {
  ytp_cursor_t *cursor = (ytp_cursor_t *)malloc(sizeof(ytp_cursor_t));
  if (!cursor) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return NULL;
  }

  ytp_cursor_init(cursor, yamal, error);
  if (*error) {
    free(cursor);
    return NULL;
  }

  return cursor;
}

void ytp_cursor_del(ytp_cursor_t *cursor, fmc_error_t **error) {
  ytp_cursor_destroy(cursor, error);
  if (error) {
    return;
  }

  free(cursor);
}

void ytp_cursor_init(ytp_cursor_t *cursor, ytp_yamal_t *yamal,
                     fmc_error_t **error) {
  cursor->it_data = ytp_data_begin(yamal, error);
  if (*error) {
    return;
  }

  cursor->it_ann = ytp_announcement_begin(yamal, error);
  if (*error) {
    return;
  }

  cursor->ann_processed = 0;
  utarray_init(&cursor->cb_ann, &cb_ann_icd);
  cursor->cb_data = NULL;
  cursor->yamal = yamal;
}

void ytp_cursor_destroy(ytp_cursor_t *cursor, fmc_error_t **error) {
  fmc_error_clear(error);
  utarray_done(&cursor->cb_ann);

  struct ytp_cursor_streams_data_item_t *item;
  struct ytp_cursor_streams_data_item_t *tmp;
  HASH_ITER(hh, cursor->cb_data, item, tmp) {
    HASH_DEL(cursor->cb_data, item);
    utarray_done(&item->cb_data);
    free(item);
  }
}

void ytp_cursor_ann_cb(ytp_cursor_t *cursor, ytp_cursor_ann_cb_t cb,
                       void *closure, fmc_error_t **error) {
  fmc_error_clear(error);

  for (size_t i = utarray_len(&cursor->cb_ann); i-- > 0;) {
    struct ytp_cursor_ann_cb_cl_t *p;
    p = _utarray_eltptr(&cursor->cb_ann, i);
    if (p->cb == cb && p->cl == closure) {
      return;
    }
  }

  struct ytp_cursor_ann_cb_cl_t item;
  item.cb = cb;
  item.cl = closure;
  utarray_push_back(&cursor->cb_ann, &item);
  if (*error) {
    return;
  }
}

void ytp_cursor_ann_cb_rm(ytp_cursor_t *cursor, ytp_cursor_ann_cb_t cb,
                          void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  struct ytp_cursor_ann_cb_cl_t *p;
  size_t new_size = utarray_len(&cursor->cb_ann);
  for (size_t i = utarray_len(&cursor->cb_ann); i-- > 0;) {
    p = _utarray_eltptr(&cursor->cb_ann, i);
    if (p->cb == cb && p->cl == closure) {
      if (cursor->cb_ann_locked == 0) {
        --new_size;
        ut_swap(_utarray_eltptr(&cursor->cb_ann, i),
                _utarray_eltptr(&cursor->cb_ann, new_size),
                sizeof(struct ytp_cursor_ann_cb_cl_t));
      } else {
        p->cb = NULL;
      }
    }
  }
  utarray_resize(&cursor->cb_ann, new_size);
  if (*error) {
    return;
  }
}

void ytp_cursor_data_cb(ytp_cursor_t *cursor, ytp_mmnode_offs stream,
                        ytp_cursor_data_cb_t cb, void *closure,
                        fmc_error_t **error) {
  fmc_error_clear(error);

  struct ytp_cursor_streams_data_item_t *map_item =
      streams_data_emplace(&cursor->cb_data, stream, error);
  if (*error) {
    return;
  }

  for (size_t i = utarray_len(&map_item->cb_data); i-- > 0;) {
    struct ytp_cursor_data_cb_cl_t *p;
    p = _utarray_eltptr(&map_item->cb_data, i);
    if (p->cb == cb && p->cl == closure) {
      return;
    }
  }

  struct ytp_cursor_data_cb_cl_t arr_item;
  arr_item.cb = cb;
  arr_item.cl = closure;
  utarray_push_back(&map_item->cb_data, &arr_item);
  if (*error) {
    return;
  }

  if (!cursor->yamal->readonly_) {
    ytp_subscription_commit(cursor->yamal, stream, error);
    if (*error) {
      goto rollback;
    }
  }
  return;

rollback:
  utarray_pop_back(&map_item->cb_data);
  return;
}

void ytp_cursor_data_cb_rm(ytp_cursor_t *cursor, ytp_mmnode_offs stream,
                           ytp_cursor_data_cb_t cb, void *closure,
                           fmc_error_t **error) {
  fmc_error_clear(error);

  struct ytp_cursor_streams_data_item_t *map_item =
      streams_data_get(cursor->cb_data, stream);

  struct ytp_cursor_data_cb_cl_t *p;
  size_t new_size = utarray_len(&map_item->cb_data);
  for (size_t i = utarray_len(&map_item->cb_data); i-- > 0;) {
    p = _utarray_eltptr(&map_item->cb_data, i);
    if (p->cb == cb && p->cl == closure) {
      if (map_item->cb_data_locked == 0) {
        --new_size;
        ut_swap(_utarray_eltptr(&map_item->cb_data, i),
                _utarray_eltptr(&map_item->cb_data, new_size),
                sizeof(struct ytp_cursor_ann_cb_cl_t));
      } else {
        p->cb = NULL;
      }
    }
  }
  utarray_resize(&map_item->cb_data, new_size);
  if (*error) {
    return;
  }
}

static bool ytp_cursor_poll_ann(ytp_cursor_t *cursor, fmc_error_t **error) {
  uint64_t seqno;
  ytp_mmnode_offs stream;
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
  size_t esz;
  const char *encoding;

  ytp_mmnode_offs *original_ptr;
  ytp_mmnode_offs *subscribed_ptr;
  bool polled = ytp_announcement_next(
      cursor->yamal, &cursor->it_ann, &seqno, &stream, &psz, &peer, &csz,
      &channel, &esz, &encoding, &original_ptr, &subscribed_ptr, error);
  if (*error || !polled) {
    return false;
  }

  cursor->ann_processed = seqno;

  ytp_mmnode_offs original = atomic_load_cast(original_ptr);
  ytp_mmnode_offs subscribed = atomic_load_cast(subscribed_ptr);

  ++cursor->cb_ann_locked;
  for (size_t i = utarray_len(&cursor->cb_ann); i-- > 0;) {
    struct ytp_cursor_ann_cb_cl_t *p;
    p = _utarray_eltptr(&cursor->cb_ann, i);
    if (p->cb == NULL) {
      continue;
    }
    p->cb(p->cl, seqno, original, psz, peer, csz, channel, esz, encoding,
          subscribed != 0);
  }
  if (--cursor->cb_ann_locked == 0) {
    size_t new_size = utarray_len(&cursor->cb_ann);
    for (size_t i = utarray_len(&cursor->cb_ann); i-- > 0;) {
      struct ytp_cursor_ann_cb_cl_t *p;
      p = _utarray_eltptr(&cursor->cb_ann, i);
      if (p->cb == NULL) {
        --new_size;
        ut_swap(_utarray_eltptr(&cursor->cb_ann, i),
                _utarray_eltptr(&cursor->cb_ann, new_size),
                sizeof(struct ytp_cursor_ann_cb_cl_t));
      }
    }
    utarray_resize(&cursor->cb_ann, new_size);
  }

  return true;
}

bool ytp_cursor_poll(ytp_cursor_t *cursor, fmc_error_t **error) {
  {
    bool polled = ytp_cursor_poll_ann(cursor, error);
    if (polled || *error) {
      return polled;
    }
  }

  if (ytp_yamal_term(cursor->it_data)) {
    return false;
  }

  uint64_t seqno;
  int64_t ts;
  ytp_mmnode_offs stream;
  size_t sz;
  const char *data;
  ytp_data_read(cursor->yamal, cursor->it_data, &seqno, &ts, &stream, &sz,
                &data, error);
  if (*error) {
    return false;
  }

  uint64_t stream_seqno;
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
  size_t esz;
  const char *encoding;
  ytp_mmnode_offs *original;
  ytp_mmnode_offs *subscribed;
  ytp_announcement_lookup(cursor->yamal, stream, &stream_seqno, &psz, &peer,
                          &csz, &channel, &esz, &encoding, &original,
                          &subscribed, error);
  if (cursor->ann_processed < stream_seqno) {
    bool polled = ytp_cursor_poll_ann(cursor, error);
    if (!*error && !polled) {
      fmc_error_set(error, "data message is using an invalid stream id");
    }
    return polled;
  }

  struct ytp_cursor_streams_data_item_t *map_item =
      streams_data_get(cursor->cb_data, stream);

  if (map_item != NULL) {
    ++map_item->cb_data_locked;
    for (size_t i = utarray_len(&map_item->cb_data); i-- > 0;) {
      struct ytp_cursor_data_cb_cl_t *p;
      p = _utarray_eltptr(&map_item->cb_data, i);
      if (p->cb == NULL) {
        continue;
      }
      p->cb(p->cl, seqno, ts, *original, sz, data);
    }
    if (--map_item->cb_data_locked == 0) {
      size_t new_size = utarray_len(&map_item->cb_data);
      for (size_t i = utarray_len(&map_item->cb_data); i-- > 0;) {
        struct ytp_cursor_data_cb_cl_t *p;
        p = _utarray_eltptr(&map_item->cb_data, i);
        if (p->cb == NULL) {
          --new_size;
          ut_swap(_utarray_eltptr(&map_item->cb_data, i),
                  _utarray_eltptr(&map_item->cb_data, new_size),
                  sizeof(struct ytp_cursor_data_cb_cl_t));
        }
      }
      utarray_resize(&map_item->cb_data, new_size);
    }
  }

  ytp_iterator_t next_it =
      ytp_yamal_next(cursor->yamal, cursor->it_data, error);
  if (*error) {
    return false;
  }

  cursor->it_data = next_it;
  return true;
}

struct rollback_history_t {
  UT_array *arr;
  size_t old_sz;
};

bool ytp_cursor_consume(ytp_cursor_t *dest, ytp_cursor_t *src,
                        fmc_error_t **error) {
  fmc_error_clear(error);

  if (src->it_data != dest->it_data || src->it_ann != dest->it_ann) {
    return false;
  }

  size_t src_ann_sz = utarray_len(&src->cb_ann);
  size_t old_ann_sz = utarray_len(&dest->cb_ann);
  size_t new_ann_sz = old_ann_sz + src_ann_sz;

  struct rollback_history_t rollback_history[HASH_CNT(hh, src->cb_data)];
  memset(rollback_history, 0, sizeof(rollback_history));

  struct rollback_history_t *h = &rollback_history[0];

  struct ytp_cursor_streams_data_item_t *map_item;
  struct ytp_cursor_streams_data_item_t *tmp;
  HASH_ITER(hh, src->cb_data, map_item, tmp) {
    struct ytp_cursor_streams_data_item_t *dest_map_item = streams_data_emplace(
        &dest->cb_data, (*(ytp_mmnode_offs *)map_item->hh.key), error);
    if (*error) {
      goto rollback;
    }

    size_t src_data_sz = utarray_len(&map_item->cb_data);
    size_t old_data_sz = utarray_len(&dest_map_item->cb_data);
    size_t new_data_sz = old_data_sz + src_data_sz;
    utarray_resize(&dest_map_item->cb_data, new_data_sz);
    if (*error) {
      goto rollback;
    }
    h->arr = &dest_map_item->cb_data;
    h->old_sz = old_data_sz;
    ++h;

    size_t i = old_data_sz;
    for (size_t j = 0; j < src_data_sz; ++j) {
      struct ytp_cursor_ann_cb_cl_t *s, *d;
      s = _utarray_eltptr(&map_item->cb_data, j);
      if (s->cb == NULL) {
        --new_data_sz;
        continue;
      }
      d = _utarray_eltptr(&dest_map_item->cb_data, i++);
      *d = *s;
    }
    utarray_resize(&dest_map_item->cb_data, new_data_sz);
  }

  {
    utarray_resize(&dest->cb_ann, new_ann_sz);
    if (*error) {
      goto rollback;
    }

    size_t i = old_ann_sz;
    for (size_t j = 0; j < src_ann_sz; ++j) {
      struct ytp_cursor_ann_cb_cl_t *s, *d;
      s = _utarray_eltptr(&src->cb_ann, j);
      if (s->cb == NULL) {
        --new_ann_sz;
        continue;
      }
      d = _utarray_eltptr(&dest->cb_ann, i++);
      *d = *s;
    }
    utarray_resize(&dest->cb_ann, new_ann_sz);
  }

  HASH_ITER(hh, src->cb_data, map_item, tmp) {
    struct ytp_cursor_streams_data_item_t *dest_map_item =
        streams_data_get(dest->cb_data, (*(ytp_mmnode_offs *)map_item->hh.key));
    if (dest_map_item == NULL) {
      continue;
    }

    if (map_item->cb_data_locked == 0) {
      utarray_clear(&map_item->cb_data);
    } else {
      for (size_t i = 0; i < utarray_len(&map_item->cb_data); ++i) {
        struct ytp_cursor_ann_cb_cl_t *s;
        s = _utarray_eltptr(&map_item->cb_data, i);
        s->cb = NULL;
      }
    }
  }

  if (src->cb_ann_locked == 0) {
    utarray_clear(&src->cb_ann);
  } else {
    for (size_t i = 0; i < utarray_len(&src->cb_ann); ++i) {
      struct ytp_cursor_ann_cb_cl_t *s;
      s = _utarray_eltptr(&src->cb_ann, i);
      s->cb = NULL;
    }
  }

  return true;

rollback:
  while (h != &rollback_history[0]) {
    --h;
    utarray_resize(h->arr, h->old_sz);
  }

  utarray_resize(&dest->cb_ann, old_ann_sz);

  fmc_error_set2(error, FMC_ERROR_MEMORY);
  return false;
}

void ytp_cursor_all_cb_rm(ytp_cursor_t *cursor) {
  if (cursor->cb_ann_locked == 0) {
    utarray_clear(&cursor->cb_ann);
  } else {
    for (size_t i = utarray_len(&cursor->cb_ann); i-- > 0;) {
      struct ytp_cursor_ann_cb_cl_t *p;
      p = _utarray_eltptr(&cursor->cb_ann, i);
      p->cb = NULL;
    }
  }

  struct ytp_cursor_streams_data_item_t *map_item;
  struct ytp_cursor_streams_data_item_t *tmp;
  HASH_ITER(hh, cursor->cb_data, map_item, tmp) {
    if (map_item->cb_data_locked == 0) {
      utarray_clear(&map_item->cb_data);
    } else {
      for (size_t i = utarray_len(&map_item->cb_data); i-- > 0;) {
        struct ytp_cursor_data_cb_cl_t *p;
        p = _utarray_eltptr(&map_item->cb_data, i);
        p->cb = NULL;
      }
    }
  }
}

void ytp_cursor_seek(ytp_cursor_t *cursor, ytp_mmnode_offs offset,
                     fmc_error_t **error) {
  ytp_iterator_t new_it = ytp_yamal_seek(cursor->yamal, offset, error);
  if (*error) {
    return;
  }

  cursor->it_data = new_it;
}

ytp_mmnode_offs ytp_cursor_tell(ytp_cursor_t *cursor, fmc_error_t **error) {
  return ytp_yamal_tell(cursor->yamal, cursor->it_data, error);
}
