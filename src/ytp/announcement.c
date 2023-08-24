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

#include <fmc/error.h>
#include <ytp/announcement.h>

#include <string.h>

struct ann_msg_t {
  ytp_mmnode_offs original;
  ytp_mmnode_offs subscribed;
  uint32_t peer_name_sz;
  uint32_t channel_name_sz;
  char payload[];
};

ytp_iterator_t ytp_announcement_write(ytp_yamal_t *yamal, size_t psz,
                                      const char *peer, size_t csz,
                                      const char *channel, size_t esz,
                                      const char *encoding,
                                      fmc_error_t **error) {
  if (psz > INT32_MAX) {
    fmc_error_set(error, "peer name is too long");
    return NULL;
  }

  if (csz > INT32_MAX) {
    fmc_error_set(error, "channel name is too long");
    return NULL;
  }

  struct ann_msg_t *msg = (struct ann_msg_t *)ytp_yamal_reserve(
      yamal, psz + csz + esz + sizeof(struct ann_msg_t), error);
  if (*error) {
    return NULL;
  }

  msg->peer_name_sz = htoye32(psz);
  msg->channel_name_sz = htoye32(csz);
  memcpy(msg->payload, peer, psz);
  memcpy(msg->payload + psz, channel, csz);
  memcpy(msg->payload + psz + csz, encoding, esz);
  return ytp_yamal_commit(yamal, (char *)msg, YTP_STREAM_LIST_ANNS, error);
}

void ytp_announcement_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                           uint64_t *seqno, size_t *psz, const char **peer,
                           size_t *csz, const char **channel, size_t *esz,
                           const char **encoding, ytp_mmnode_offs **original,
                           ytp_mmnode_offs **subscribed, fmc_error_t **error) {
  size_t sz;
  struct ann_msg_t *msg;
  ytp_yamal_read(yamal, iterator, seqno, &sz, (const char **)&msg, error);
  if (*error) {
    return;
  }

  *psz = ye32toh(msg->peer_name_sz);
  *csz = ye32toh(msg->channel_name_sz);

  if (*psz + *csz > sz) {
    fmc_error_set(error, "invalid announcement message");
    return;
  }

  *esz = sz - (*psz + *csz + sizeof(struct ann_msg_t));

  *peer = msg->payload;
  *channel = *peer + *psz;
  *encoding = *channel + *csz;
  *original = &msg->original;
  *subscribed = &msg->subscribed;
}

void ytp_announcement_lookup(ytp_yamal_t *yamal, ytp_mmnode_offs stream,
                             uint64_t *seqno, size_t *psz, const char **peer,
                             size_t *csz, const char **channel, size_t *esz,
                             const char **encoding, ytp_mmnode_offs **original,
                             ytp_mmnode_offs **subscribed,
                             fmc_error_t **error) {
  ytp_iterator_t iterator = ytp_yamal_seek(yamal, stream, error);
  if (*error) {
    return;
  }

  ytp_announcement_read(yamal, iterator, seqno, psz, peer, csz, channel, esz,
                        encoding, original, subscribed, error);
}

ytp_iterator_t ytp_announcement_begin(ytp_yamal_t *yamal, fmc_error_t **error) {
  return ytp_yamal_begin(yamal, YTP_STREAM_LIST_ANNS, error);
}

bool ytp_announcement_term(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                           fmc_error_t **error) {
  fmc_error_clear(error);
  while (!ytp_yamal_term(iterator)) {
    uint64_t seqno;
    size_t psz;
    const char *peer;
    size_t csz;
    const char *channel;
    size_t esz;
    const char *encoding;
    ytp_mmnode_offs *original;
    ytp_mmnode_offs *subscribed;
    ytp_announcement_read(yamal, iterator, &seqno, &psz, &peer, &csz, &channel,
                          &esz, &encoding, &original, &subscribed, error);
    if (*error) {
      return true;
    }

    ytp_mmnode_offs original_val = atomic_load_cast(original);
    if (original_val == 0) {
      return true;
    }

    ytp_mmnode_offs stream = ytp_yamal_tell(yamal, iterator, error);
    if (*error) {
      return true;
    }

    iterator = ytp_yamal_next(yamal, iterator, error);
    if (*error) {
      return true;
    }

    if (original_val == stream) {
      return false;
    }
  }
  return true;
}

bool ytp_announcement_next(ytp_yamal_t *yamal, ytp_iterator_t *iterator,
                           uint64_t *seqno, ytp_mmnode_offs *stream,
                           size_t *psz, const char **peer, size_t *csz,
                           const char **channel, size_t *esz,
                           const char **encoding, ytp_mmnode_offs **original,
                           ytp_mmnode_offs **subscribed, fmc_error_t **error) {
  fmc_error_clear(error);
  while (!ytp_yamal_term(*iterator)) {
    ytp_announcement_read(yamal, *iterator, seqno, psz, peer, csz, channel, esz,
                          encoding, original, subscribed, error);
    if (*error) {
      return false;
    }

    ytp_mmnode_offs original_val = atomic_load_cast(*original);
    if (original_val == 0) {
      return false;
    }

    *stream = ytp_yamal_tell(yamal, *iterator, error);
    if (*error) {
      return false;
    }

    *iterator = ytp_yamal_next(yamal, *iterator, error);
    if (*error) {
      return false;
    }

    if (original_val == *stream) {
      return true;
    }
  }
  return false;
}
