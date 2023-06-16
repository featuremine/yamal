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

#include "cursor.h"

#include <ytp/cursor.h>
#include <ytp/data.h>
#include <ytp/streams.h>

#include <uthash/utarray.h>
#include <uthash/uthash.h>

#include <stdlib.h>

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

struct ytp_cursor_streams_data_item_t {
  UT_hash_handle hh;
  UT_array *cb_data;
  int cb_data_locked;
};

static struct ytp_cursor_streams_data_item_t *
hashmap_add(struct ytp_cursor_streams_data_item_t **m,
            const ytp_mmnode_offs *key) {
  struct ytp_cursor_streams_data_item_t *item =
      aligned_alloc(_Alignof(struct ytp_cursor_streams_data_item_t),
                    sizeof(struct ytp_cursor_streams_data_item_t));
  HASH_ADD_KEYPTR(hh, (*m), key, sizeof(ytp_mmnode_offs), item);
  return item;
}

static struct ytp_cursor_streams_data_item_t *
hashmap_get(struct ytp_cursor_streams_data_item_t *m,
            const ytp_mmnode_offs *key) {
  struct ytp_cursor_streams_data_item_t *item;
  HASH_FIND(hh, m, key, sizeof(ytp_mmnode_offs), item);
  return item;
}

static struct ytp_cursor_streams_data_item_t *
hashmap_emplace(struct ytp_cursor_streams_data_item_t **m,
                ytp_mmnode_offs key) {
  struct ytp_cursor_streams_data_item_t *item = hashmap_get(*m, &key);
  if (item == NULL) {
    item = hashmap_add(m, &key);
    utarray_init(item->cb_data, &cb_data_icd);
    item->cb_data_locked = 0;
  }
  return item;
}

ytp_cursor_t *ytp_cursor_new(ytp_yamal_t *yamal, fmc_error_t **error) {
  ytp_cursor_t *cursor = (ytp_cursor_t *)aligned_alloc(_Alignof(ytp_cursor_t),
                                                       sizeof(ytp_cursor_t));
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
  utarray_init(cursor->cb_ann, &cb_ann_icd);
  cursor->cb_data = NULL;
}

void ytp_cursor_destroy(ytp_cursor_t *cursor, fmc_error_t **error) {
  fmc_error_clear(error);
  utarray_free(cursor->cb_ann);

  struct ytp_cursor_streams_data_item_t *item;
  struct ytp_cursor_streams_data_item_t *tmp;
  HASH_ITER(hh, cursor->cb_data, item, tmp) {
    HASH_DEL(cursor->cb_data, item);
    utarray_free(item->cb_data);
    free(item);
  }
}

void ytp_cursor_ann_cb(ytp_cursor_t *cursor, ytp_cursor_ann_cb_t cb,
                       void *closure, fmc_error_t **error) {
  fmc_error_clear(error);

  for (size_t i = utarray_len(cursor->cb_ann); i-- > 0;) {
    struct ytp_cursor_ann_cb_cl_t *p;
    p = _utarray_eltptr(cursor->cb_ann, i);
    if (p->cb == cb && p->cl == closure) {
      return;
    }
  }

  struct ytp_cursor_ann_cb_cl_t item;
  item.cb = cb;
  item.cl = closure;
  utarray_push_back(cursor->cb_ann, &item);
}

void ytp_cursor_ann_cb_rm(ytp_cursor_t *cursor, ytp_cursor_ann_cb_t cb,
                          void *closure, fmc_error_t **error) {
  fmc_error_clear(error);
  struct ytp_cursor_ann_cb_cl_t *p;
  size_t new_size = utarray_len(cursor->cb_ann);
  for (size_t i = utarray_len(cursor->cb_ann); i-- > 0;) {
    p = _utarray_eltptr(cursor->cb_ann, i);
    if (p->cb == cb && p->cl == closure) {
      if (cursor->cb_ann_locked == 0) {
        --new_size;
        ut_swap(_utarray_eltptr(cursor->cb_ann, i),
                _utarray_eltptr(cursor->cb_ann, new_size),
                sizeof(struct ytp_cursor_ann_cb_cl_t));
      } else {
        p->cb = NULL;
      }
    }
  }
  utarray_resize(cursor->cb_ann, new_size);
}

void ytp_cursor_data_cb(ytp_cursor_t *cursor, ytp_mmnode_offs stream,
                        ytp_cursor_data_cb_t cb, void *closure,
                        fmc_error_t **error) {
  fmc_error_clear(error);

  struct ytp_cursor_streams_data_item_t *map_item =
      hashmap_emplace(&cursor->cb_data, stream);

  for (size_t i = utarray_len(map_item->cb_data); i-- > 0;) {
    struct ytp_cursor_data_cb_cl_t *p;
    p = _utarray_eltptr(map_item->cb_data, i);
    if (p->cb == cb && p->cl == closure) {
      return;
    }
  }

  struct ytp_cursor_data_cb_cl_t arr_item;
  arr_item.cb = cb;
  arr_item.cl = closure;
  utarray_push_back(map_item->cb_data, &arr_item);
}

void ytp_cursor_data_cb_rm(ytp_cursor_t *cursor, ytp_mmnode_offs stream,
                           ytp_cursor_data_cb_t cb, void *closure,
                           fmc_error_t **error) {
  fmc_error_clear(error);

  struct ytp_cursor_streams_data_item_t *map_item =
      hashmap_get(cursor->cb_data, &stream);

  struct ytp_cursor_data_cb_cl_t *p;
  size_t new_size = utarray_len(map_item->cb_data);
  for (size_t i = utarray_len(map_item->cb_data); i-- > 0;) {
    p = _utarray_eltptr(map_item->cb_data, i);
    if (p->cb == cb && p->cl == closure) {
      if (map_item->cb_data_locked == 0) {
        --new_size;
        ut_swap(_utarray_eltptr(map_item->cb_data, i),
                _utarray_eltptr(map_item->cb_data, new_size),
                sizeof(struct ytp_cursor_ann_cb_cl_t));
      } else {
        p->cb = NULL;
      }
    }
  }
  utarray_resize(map_item->cb_data, new_size);
}

bool ytp_cursor_term(ytp_cursor_t *cursor, fmc_error_t **error) {
  return ytp_yamal_term(cursor->it_data) &&
         ytp_announcement_term(cursor->yamal, cursor->it_ann, error);
}

static bool ytp_cursor_poll_ann(ytp_cursor_t *cursor, fmc_error_t **error) {
  bool ann_term = ytp_announcement_term(cursor->yamal, cursor->it_ann, error);
  if (*error || ann_term) {
    return false;
  }

  uint64_t seqno;
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
  size_t esz;
  const char *encoding;

  ytp_mmnode_offs *original;
  ytp_mmnode_offs *subscribed;
  ytp_announcement_read(cursor->yamal, cursor->it_ann, &seqno, &psz, &peer,
                        &csz, &channel, &esz, &encoding, &original, &subscribed,
                        error);
  cursor->ann_processed = seqno;

  ++cursor->cb_ann_locked;
  for (size_t i = utarray_len(cursor->cb_ann); i-- > 0;) {
    struct ytp_cursor_ann_cb_cl_t *p;
    p = _utarray_eltptr(cursor->cb_ann, i);
    if (p->cb == NULL) {
      continue;
    }
    p->cb(p->cl, seqno, *original, psz, peer, csz, channel, esz, encoding);
  }
  if (--cursor->cb_ann_locked == 0) {
    size_t new_size = utarray_len(cursor->cb_ann);
    for (size_t i = utarray_len(cursor->cb_ann); i-- > 0;) {
      struct ytp_cursor_ann_cb_cl_t *p;
      p = _utarray_eltptr(cursor->cb_ann, i);
      if (p->cb == NULL) {
        --new_size;
        ut_swap(_utarray_eltptr(cursor->cb_ann, i),
                _utarray_eltptr(cursor->cb_ann, new_size),
                sizeof(struct ytp_cursor_ann_cb_cl_t));
      }
    }
    utarray_resize(cursor->cb_ann, new_size);
  }

  ytp_iterator_t next_it =
      ytp_announcement_next(cursor->yamal, cursor->it_ann, error);
  if (*error) {
    return false;
  }

  cursor->it_ann = next_it;
  return true;
}

bool ytp_cursor_poll(ytp_cursor_t *cursor, fmc_error_t **error) {
  {
    bool polled = ytp_cursor_poll_ann(cursor, error);
    if (polled || *error) {
      return polled;
    }
  }

  if (ytp_yamal_term(cursor->yamal)) {
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
      hashmap_get(cursor->cb_data, &stream);

  ++map_item->cb_data_locked;
  for (size_t i = utarray_len(map_item->cb_data); i-- > 0;) {
    struct ytp_cursor_data_cb_cl_t *p;
    p = _utarray_eltptr(map_item->cb_data, i);
    if (p->cb == NULL) {
      continue;
    }
    p->cb(p->cl, seqno, ts, *original, sz, data);
  }
  if (--map_item->cb_data_locked == 0) {
    size_t new_size = utarray_len(map_item->cb_data);
    for (size_t i = utarray_len(map_item->cb_data); i-- > 0;) {
      struct ytp_cursor_data_cb_cl_t *p;
      p = _utarray_eltptr(map_item->cb_data, i);
      if (p->cb == NULL) {
        --new_size;
        ut_swap(_utarray_eltptr(map_item->cb_data, i),
                _utarray_eltptr(map_item->cb_data, new_size),
                sizeof(struct ytp_cursor_data_cb_cl_t));
      }
    }
    utarray_resize(map_item->cb_data, new_size);
  }

  ytp_iterator_t next_it =
      ytp_announcement_next(cursor->yamal, cursor->it_data, error);
  if (*error) {
    return false;
  }

  cursor->it_data = next_it;
  return true;
}

bool ytp_cursor_consume(ytp_cursor_t *dest, ytp_cursor_t *src) {
  if (src->it_data != dest->it_data || src->it_ann != dest->it_ann) {
    return false;
  }

  struct ytp_cursor_streams_data_item_t *map_item;
  struct ytp_cursor_streams_data_item_t *tmp;
  HASH_ITER(hh, src->cb_data, map_item, tmp) {
    struct ytp_cursor_streams_data_item_t *dest_map_item =
        hashmap_emplace(&dest->cb_data, (*(ytp_mmnode_offs *)map_item->hh.key));
    for (size_t i = utarray_len(map_item->cb_data); i-- > 0;) {
      struct ytp_cursor_ann_cb_cl_t *p;
      p = _utarray_eltptr(map_item->cb_data, i);
      if (p->cb == NULL) {
        continue;
      }
      utarray_push_back(dest_map_item->cb_data, p);
      p->cb = NULL;
    }
    if (map_item->cb_data_locked == 0) {
      utarray_clear(map_item->cb_data);
    }
  }

  for (size_t i = utarray_len(src->cb_ann); i-- > 0;) {
    struct ytp_cursor_ann_cb_cl_t *p;
    p = _utarray_eltptr(src->cb_ann, i);
    if (p->cb == NULL) {
      continue;
    }
    utarray_push_back(dest->cb_ann, p);
    p->cb = NULL;
  }
  if (src->cb_ann_locked == 0) {
    utarray_clear(src->cb_ann);
  }

  return true;
}

void ytp_cursor_all_cb_rm(ytp_cursor_t *cursor) {
  if (cursor->cb_ann_locked == 0) {
    utarray_clear(cursor->cb_ann);
  } else {
    for (size_t i = utarray_len(cursor->cb_ann); i-- > 0;) {
      struct ytp_cursor_ann_cb_cl_t *p;
      p = _utarray_eltptr(cursor->cb_ann, i);
      p->cb = NULL;
    }
  }

  struct ytp_cursor_streams_data_item_t *map_item;
  struct ytp_cursor_streams_data_item_t *tmp;
  HASH_ITER(hh, cursor->cb_data, map_item, tmp) {
    if (map_item->cb_data_locked == 0) {
      utarray_clear(map_item->cb_data);
    } else {
      for (size_t i = utarray_len(map_item->cb_data); i-- > 0;) {
        struct ytp_cursor_data_cb_cl_t *p;
        p = _utarray_eltptr(map_item->cb_data, i);
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
