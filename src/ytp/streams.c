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
#include <fmc/error.h>

#include <ytp/streams.h>

#include <stdlib.h>

#include <uthash/uthash.h>

#include "atomic.h"

/* TODO: add error handling for uthash */

struct ytp_streams_reverse_map_key_t {
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
};

struct ytp_streams_reverse_map_t {
  UT_hash_handle hh;

  size_t esz;
  const char *encoding;
  ytp_mmnode_offs stream;
};

struct ytp_streams {
  ytp_yamal_t *yamal;
  ytp_iterator_t iterator;
  struct ytp_streams_reverse_map_t *reverse_map;
};

static bool key_equal(size_t psz1, const char *peer1, size_t csz1,
                      const char *channel1, size_t psz2, const char *peer2,
                      size_t csz2, const char *channel2) {
  if (psz1 != psz2 || csz1 != csz2) {
    return false;
  }
  if (memcmp(peer1, peer2, psz1) != 0) {
    return false;
  }
  return memcmp(channel1, channel2, csz1) == 0;
}

#undef HASH_KEYCMP
#define HASH_KEYCMP(a, b, n) hashmap_cmp(a, b)
static int hashmap_cmp(const struct ytp_streams_reverse_map_key_t *a,
                       const struct ytp_streams_reverse_map_key_t *b) {
  return key_equal(a->psz, a->peer, a->csz, a->channel, b->psz, b->peer, b->csz,
                   b->channel) -
         1;
}

static unsigned hashmap_hash(const struct ytp_streams_reverse_map_key_t *key) {
  size_t hash_peer;
  size_t hash_channel;
  HASH_JEN(key->peer, key->psz, hash_peer);
  HASH_JEN(key->channel, key->csz, hash_channel);
  return fmc_hash_combine(hash_peer, hash_channel);
}

static struct ytp_streams_reverse_map_t *
hashmap_add(struct ytp_streams_reverse_map_t **m,
            const struct ytp_streams_reverse_map_key_t *key) {
  struct ytp_streams_reverse_map_t *item =
      aligned_alloc(_Alignof(struct ytp_streams_reverse_map_t),
                    sizeof(struct ytp_streams_reverse_map_t));
  unsigned hash = hashmap_hash(key);
  HASH_ADD_KEYPTR_BYHASHVALUE(
      hh, (*m), key, sizeof(struct ytp_streams_reverse_map_key_t), hash, item);
  return item;
}

static struct ytp_streams_reverse_map_t *
hashmap_get(struct ytp_streams_reverse_map_t *m,
            const struct ytp_streams_reverse_map_key_t *key) {
  struct ytp_streams_reverse_map_t *item;
  unsigned hash = hashmap_hash(key);
  HASH_FIND_BYHASHVALUE(
      hh, m, key, sizeof(struct ytp_streams_reverse_map_key_t), hash, item);
  return item;
}

static struct ytp_streams_reverse_map_t *
hashmap_emplace(struct ytp_streams_reverse_map_t **m, size_t psz,
                const char *peer, size_t csz, const char *channel, size_t esz,
                const char *encoding, ytp_mmnode_offs stream) {
  struct ytp_streams_reverse_map_key_t key = {psz, peer, csz, channel};
  struct ytp_streams_reverse_map_t *item = hashmap_get(*m, &key);
  if (item == NULL) {
    item = hashmap_add(m, &key);
    item->esz = esz;
    item->encoding = encoding;
    item->stream = stream;
  }
  return item;
}

static void ytp_streams_init(ytp_streams_t *streams, ytp_yamal_t *yamal,
                             fmc_error_t **error) {
  streams->reverse_map = NULL;
  streams->yamal = yamal;
  streams->iterator = ytp_yamal_begin(yamal, YTP_STREAM_LIST_ANNS, error);
}

static void ytp_streams_destroy(ytp_streams_t *streams, fmc_error_t **error) {
  fmc_error_clear(error);

  struct ytp_streams_reverse_map_t *item;
  struct ytp_streams_reverse_map_t *tmp;
  HASH_ITER(hh, streams->reverse_map, item, tmp) {
    HASH_DEL(streams->reverse_map, item);
    free(item);
  }
}

ytp_streams_t *ytp_streams_new(ytp_yamal_t *yamal, fmc_error_t **error) {
  ytp_streams_t *streams = (ytp_streams_t *)aligned_alloc(
      _Alignof(ytp_streams_t), sizeof(ytp_streams_t));
  if (!streams) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return NULL;
  }

  ytp_streams_init(streams, yamal, error);
  if (*error) {
    free(streams);
    return NULL;
  }

  return streams;
}

void ytp_streams_del(ytp_streams_t *streams, fmc_error_t **error) {
  ytp_streams_destroy(streams, error);
  free(streams);
}

void ytp_streams_search_ann(ytp_yamal_t *yamal, ytp_iterator_t *iterator,
                            enum ytp_streams_pred_result (*predicate)(
                                void *closure,
                                const struct ytp_streams_anndata_t *ann),
                            void *closure, fmc_error_t **error) {
  fmc_error_clear(error);

  while (!ytp_yamal_term(*iterator)) {
    struct ytp_streams_anndata_t ann;
    ytp_announcement_read(yamal, *iterator, &ann.seqno, &ann.psz, &ann.peer,
                          &ann.csz, &ann.channel, &ann.esz, &ann.encoding,
                          &ann.original, &ann.subscribed, error);
    if (*error) {
      return;
    }

    ann.stream = ytp_yamal_tell(yamal, *iterator, error);
    if (*error) {
      return;
    }

    enum ytp_streams_pred_result res = predicate(closure, &ann);

    if (res == YTP_STREAMS_PRED_ROLLBACK) {
      return;
    }

    ytp_iterator_t next = ytp_yamal_next(yamal, *iterator, error);
    if (*error) {
      return;
    }
    *iterator = next;

    if (res == YTP_STREAMS_PRED_DONE) {
      return;
    }
  }
}

struct ytp_streams_pred_cl_t {
  ytp_streams_t *streams;
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
  size_t esz;
  const char *encoding;
  struct ytp_streams_reverse_map_t *item;
};

static enum ytp_streams_pred_result
ytp_streams_pred(void *closure, const struct ytp_streams_anndata_t *ann) {
  struct ytp_streams_pred_cl_t *cl = (struct ytp_streams_pred_cl_t *)closure;

  struct ytp_streams_reverse_map_t *item =
      hashmap_emplace(&cl->streams->reverse_map, ann->psz, ann->peer, ann->csz,
                      ann->channel, ann->esz, ann->encoding, ann->stream);
  ytp_mmnode_offs original = atomic_load_cast(ann->original);
  if (original != item->stream) {
    if (original != 0) {
      return YTP_STREAMS_PRED_CONTINUE;
    }
    if (cl->streams->yamal->readonly_) {
      return YTP_STREAMS_PRED_ROLLBACK;
    }
    *ann->original = item->stream;
  }

  if (key_equal(ann->psz, ann->peer, ann->csz, ann->channel, cl->psz, cl->peer,
                cl->csz, cl->channel)) {
    cl->item = item;
    cl->esz = ann->esz;
    cl->encoding = ann->encoding;
    return YTP_STREAMS_PRED_DONE;
  }

  return YTP_STREAMS_PRED_CONTINUE;
}

/* TODO: announce and lookup share code, can be refactored */
ytp_mmnode_offs ytp_streams_announce(ytp_streams_t *streams, size_t psz,
                                     const char *peer, size_t csz,
                                     const char *channel, size_t esz,
                                     const char *encoding,
                                     fmc_error_t **error) {
  struct ytp_streams_pred_cl_t cl = {streams, psz, peer, csz,
                                     channel, 0,   NULL, NULL};

  ytp_mmnode_offs stream = ytp_streams_lookup(streams, psz, peer, csz, channel,
                                              &cl.esz, &cl.encoding, error);
  if (*error) {
    return 0;
  }
  if (stream != 0) {
    if (cl.item->esz != esz || memcmp(cl.item->encoding, encoding, esz) != 0) {
      fmc_error_set(error, "encoding doesn't match");
      return 0;
    }
    return stream;
  }

  if (streams->yamal->readonly_) {
    fmc_error_set(error, "unable to announce stream when the file is readonly");
    return 0;
  }
  ytp_announcement_write(streams->yamal, psz, peer, csz, channel, esz, encoding,
                         error);
  if (*error) {
    return 0;
  }

  ytp_streams_search_ann(streams->yamal, &streams->iterator, ytp_streams_pred,
                         (void *)&cl, error);
  if (*error) {
    return 0;
  }

  if (cl.item->esz != esz || memcmp(cl.item->encoding, encoding, esz) != 0) {
    fmc_error_set(error, "encoding doesn't match");
    return 0;
  }

  return cl.item->stream;
}

ytp_mmnode_offs ytp_streams_lookup(ytp_streams_t *streams, size_t psz,
                                   const char *peer, size_t csz,
                                   const char *channel, size_t *esz,
                                   const char **encoding, fmc_error_t **error) {
  fmc_error_clear(error);

  struct ytp_streams_reverse_map_key_t key = {psz, peer, csz, channel};
  struct ytp_streams_reverse_map_t *item =
      hashmap_get(streams->reverse_map, &key);
  if (item != NULL) {
    *esz = item->esz;
    *encoding = item->encoding;
    return item->stream;
  }

  struct ytp_streams_pred_cl_t cl = {streams, psz, peer, csz,
                                     channel, 0,   NULL, NULL};
  ytp_streams_search_ann(streams->yamal, &streams->iterator, ytp_streams_pred,
                         (void *)&cl, error);
  if (*error) {
    return 0;
  }
  if (*error) {
    return 0;
  }
  if (cl.item == NULL) {
    return 0;
  }
  *esz = cl.item->esz;
  *encoding = cl.item->encoding;
  return cl.item->stream;
}
