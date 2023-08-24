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

#include <ytp/streams.h>
#include <ytp/subscription.h>

#include <string.h>

struct sub_msg_t {
  uint64_t stream;
};

ytp_iterator_t ytp_subscription_write(ytp_yamal_t *yamal,
                                      ytp_mmnode_offs stream,
                                      fmc_error_t **error) {
  fmc_error_clear(error);

  struct sub_msg_t *msg = (struct sub_msg_t *)ytp_yamal_reserve(
      yamal, sizeof(struct sub_msg_t), error);
  if (*error) {
    return NULL;
  }

  msg->stream = htoye64(stream);
  return ytp_yamal_commit(yamal, (char *)msg, YTP_STREAM_LIST_SUBS, error);
}

bool ytp_subscription_commit(ytp_yamal_t *yamal, ytp_mmnode_offs stream,
                             fmc_error_t **error) {
  if (yamal->readonly_) {
    fmc_error_set(error, "trying to commit a subscription to a read-only file");
    return false;
  }

  uint64_t stream_seqno;
  size_t psz;
  const char *peer;
  size_t csz;
  const char *channel;
  size_t esz;
  const char *encoding;
  ytp_mmnode_offs *original_atomic;
  ytp_mmnode_offs *subscribed_atomic;
  ytp_announcement_lookup(yamal, stream, &stream_seqno, &psz, &peer, &csz,
                          &channel, &esz, &encoding, &original_atomic,
                          &subscribed_atomic, error);
  if (*error) {
    return false;
  }

  if (atomic_load_cast(subscribed_atomic) == 0) {
    ytp_iterator_t sub_it = ytp_subscription_write(yamal, stream, error);
    if (*error) {
      return false;
    }

    ytp_mmnode_offs sub_offs = ytp_yamal_tell(yamal, sub_it, error);
    if (*error) {
      return false;
    }

    ytp_mmnode_offs expected = 0;
    if (atomic_compare_exchange_weak_check(subscribed_atomic, &expected,
                                           sub_offs)) {
      return true;
    }
  }

  return false;
}

void ytp_subscription_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                           uint64_t *seqno, ytp_mmnode_offs *stream,
                           fmc_error_t **error) {
  size_t sz;
  struct sub_msg_t *msg;
  ytp_yamal_read(yamal, iterator, seqno, &sz, (const char **)&msg, error);
  if (*error) {
    return;
  }

  if (sz != sizeof(struct sub_msg_t)) {
    fmc_error_set(error, "invalid subscription message");
    return;
  }

  *stream = ye64toh(msg->stream);
}

void ytp_subscription_lookup(ytp_yamal_t *yamal, ytp_mmnode_offs offset,
                             uint64_t *seqno, ytp_mmnode_offs *stream,
                             fmc_error_t **error) {
  ytp_iterator_t iterator = ytp_yamal_seek(yamal, offset, error);
  if (*error) {
    return;
  }

  ytp_subscription_read(yamal, iterator, seqno, stream, error);
}

bool ytp_subscription_next(ytp_yamal_t *yamal, ytp_iterator_t *iterator,
                           ytp_mmnode_offs *stream, fmc_error_t **error) {
  fmc_error_clear(error);

  if (yamal->readonly_) {
    fmc_error_set(error, "yamal file descriptor must have write access");
    return false;
  }

  while (true) {
    if (ytp_yamal_term(*iterator)) {
      return false;
    }

    uint64_t seqno;
    ytp_mmnode_offs s;
    ytp_subscription_read(yamal, *iterator, &seqno, &s, error);
    if (*error) {
      return false;
    }

    size_t psz;
    const char *peer;
    size_t csz;
    const char *channel;
    size_t esz;
    const char *encoding;

    ytp_mmnode_offs *original;
    ytp_mmnode_offs *subscribed;
    ytp_announcement_lookup(yamal, s, &seqno, &psz, &peer, &csz, &channel, &esz,
                            &encoding, &original, &subscribed, error);
    if (*error) {
      return false;
    }

    ytp_mmnode_offs sub_offs = ytp_yamal_tell(yamal, *iterator, error);
    if (*error) {
      return false;
    }

    ytp_iterator_t next = ytp_yamal_next(yamal, *iterator, error);
    if (*error) {
      return false;
    }
    *iterator = next;

    ytp_mmnode_offs expected = 0;
    if (atomic_compare_exchange_weak_check(subscribed, &expected, sub_offs)) {
      *stream = s;
      return true;
    }
  }
}
