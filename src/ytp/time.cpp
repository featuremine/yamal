/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include "endianess.h"

#include <ytp/time.h>
#include <ytp/yamal.h>

struct ytp_time_msg {
  int64_t ts;
  char data[];
};

char *ytp_time_reserve(ytp_yamal_t *yamal, size_t size, fmc_error_t **error) {
  fmc_error_clear(error);
  struct ytp_time_msg *time_msg = (struct ytp_time_msg *)ytp_yamal_reserve(
      yamal, size + sizeof(struct ytp_time_msg), error);
  if (*error) {
    return nullptr;
  }

  return time_msg->data;
}

ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, int64_t ts, void *data,
                               size_t listidx, fmc_error_t **error) {
  struct ytp_time_msg *time_msg =
      (struct ytp_time_msg *)((char *)data - sizeof(struct ytp_time_msg));
  time_msg->ts = htoye64(ts);
  return ytp_yamal_commit(yamal, time_msg, listidx, error);
}

void ytp_time_sublist_commit(ytp_yamal_t *yamal, int64_t ts, void **first_ptr,
                             void **last_ptr, void *new_ptr,
                             fmc_error_t **error) {
  auto *time_msg = (ytp_time_msg *)((char *)new_ptr - sizeof(ytp_time_msg));
  time_msg->ts = htoye64(ts);
  return ytp_yamal_sublist_commit(yamal, first_ptr, last_ptr, time_msg, error);
}

void ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno,
                   int64_t *ts, size_t *size, const char **data,
                   fmc_error_t **error) {
  const ytp_time_msg *time_msg;
  ytp_yamal_read(yamal, iterator, seqno, size, (const char **)&time_msg, error);
  if (*error) {
    return;
  }

  *ts = ye64toh(time_msg->ts);
  *data = time_msg->data;
  *size -= sizeof(struct ytp_time_msg);
}
