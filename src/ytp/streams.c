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

#include <stdatomic.h>
#include <stdlib.h>

#include <uthash/uthash.h>

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

#undef HASH_KEYCMP
#define HASH_KEYCMP(a, b, n) hashmap_cmp(a, b)
static int hashmap_cmp(const struct ytp_streams_reverse_map_key_t *a, const struct ytp_streams_reverse_map_key_t *b) {
  if (a->psz != b->psz || a->csz != b->csz) {
    return -1;
  }
  if (memcmp(a->peer, b->peer, a->psz) != 0) {
    return -1;
  }
  return memcmp(a->channel, b->channel, a->csz);
}

static unsigned hashmap_hash(const struct ytp_streams_reverse_map_key_t *key) {
  size_t hash_peer;
  size_t hash_channel;
  HASH_JEN(key->peer, key->psz, hash_peer);
  HASH_JEN(key->channel, key->csz, hash_channel);
  return fmc_hash_combine(hash_peer, hash_channel);
}

static struct ytp_streams_reverse_map_t *hashmap_add(struct ytp_streams_reverse_map_t **m, const struct ytp_streams_reverse_map_key_t *key, struct ytp_streams_reverse_map_t *item_arg) {
  struct ytp_streams_reverse_map_t *item = aligned_alloc(_Alignof(struct ytp_streams_reverse_map_t), sizeof(struct ytp_streams_reverse_map_t));
  memcpy(item, item_arg, sizeof(struct ytp_streams_reverse_map_t));
  unsigned hash = hashmap_hash(key);
  HASH_ADD_KEYPTR_BYHASHVALUE(hh, (*m), key, sizeof(struct ytp_streams_reverse_map_key_t), hash, item);
  return item;
}

static struct ytp_streams_reverse_map_t *hashmap_get(struct ytp_streams_reverse_map_t *m, const struct ytp_streams_reverse_map_key_t *key) {
  struct ytp_streams_reverse_map_t *item;
  unsigned hash = hashmap_hash(key);
  HASH_FIND_BYHASHVALUE(hh, m, key, sizeof(struct ytp_streams_reverse_map_key_t), hash, item);
  return item;
}

static struct ytp_streams_reverse_map_t *hashmap_emplace(struct ytp_streams_reverse_map_t **m, size_t psz, const char *peer, size_t csz, const char *channel, size_t esz, const char *encoding, ytp_mmnode_offs stream) {
  struct ytp_streams_reverse_map_key_t key = {psz, peer, csz, channel};
  struct ytp_streams_reverse_map_t *item = hashmap_get(*m, &key);
  if (item == NULL) {
    struct ytp_streams_reverse_map_t item_arg = {
        {}, esz, encoding, stream,
    };
    hashmap_add(m, &key, &item_arg);
  }
  return item;
}

static void ytp_streams_init(ytp_streams_t *streams, ytp_yamal_t *yamal, fmc_error_t **error) {
  streams->reverse_map = NULL;
  streams->yamal = yamal;
  streams->iterator = ytp_yamal_begin(yamal, YTP_STREAM_LIST_ANNS, error);
}

static void ytp_streams_destroy(ytp_streams_t *streams, fmc_error_t **error) {
  fmc_error_clear(error);
}

ytp_streams_t *ytp_streams_new(ytp_yamal_t *yamal, fmc_error_t **error) {
  ytp_streams_t *streams =
      (ytp_streams_t *)aligned_alloc(_Alignof(ytp_streams_t), sizeof(ytp_streams_t));
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

ytp_mmnode_offs ytp_streams_announce(ytp_streams_t *streams,
                                     size_t psz, const char *peer,
                                     size_t csz, const char *channel,
                                     size_t esz, const char *encoding,
                                     fmc_error_t **error) {
  fmc_error_clear(error);

  struct ytp_streams_reverse_map_key_t key = {psz, peer, csz, channel};
  struct ytp_streams_reverse_map_t *item = hashmap_get(streams->reverse_map, &key);
  if (item != NULL) {
    if (item->esz != esz || memcmp(item->encoding, encoding, esz) != 0) {
      fmc_error_set(error, "encoding doesn't match");
      return 0;
    }

    return item->stream;
  }

  while (!ytp_yamal_term(streams->iterator)) {
    uint64_t seqno;
    size_t read_psz;
    const char *read_peer;
    size_t read_csz;
    const char *read_channel;
    size_t read_esz;
    const char *read_encoding;
    ytp_mmnode_offs *original;
    ytp_mmnode_offs *subscribed;

    ytp_announcement_read(streams->yamal, streams->iterator, &seqno, &read_psz, &read_peer, &read_csz, &read_channel, &read_esz, &read_encoding, &original, &subscribed, error);
    if (*error) {
      return 0;
    }

    ytp_mmnode_offs stream = ytp_yamal_tell(streams->yamal, streams->iterator, error);
    if (*error) {
      return 0;
    }

    atomic_compare_exchange_weak_check((a), &expected, desired);               \

    ytp_iterator_t next = ytp_yamal_next(streams->yamal, streams->iterator, error);
    if (*error) {
      return 0;
    }
    streams->iterator = next;
  }

  uint64_t seqno;
  size_t sz;
  const char *data;
  ytp_yamal_read(streams->yamal, streams->iterator, &seqno, &sz, &data, error);
  if (*error) {
    return 0;
  }

  ytp_yamal_next(streams->yamal, streams->iterator, error);
  if (*error) {
    return 0;
  }
}

ytp_mmnode_offs ytp_streams_lookup(ytp_streams_t *streams,
                                   size_t psz, char *peer,
                                   size_t csz, char *channel,
                                   size_t *esz, char **encoding,
                                   fmc_error_t **error) {
}
