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

#include "endianess.h"

#include <fmc/error.h>

#include <ytp/data.h>
#include <ytp/stream.h>
#include <ytp/time.h>

struct data_msg_t {
  ytp_mmnode_offs stream;
  char data[];
};

char *ytp_data_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error) {
  struct data_msg_t *msg = (struct data_msg_t *)ytp_time_reserve(
      yamal, sz + sizeof(struct data_msg_t), error);
  return msg->data;
}

ytp_iterator_t ytp_data_commit(ytp_yamal_t *yamal, int64_t ts,
                               ytp_mmnode_offs stream, void *data,
                               fmc_error_t **error) {
  struct data_msg_t *msg =
      (struct data_msg_t *)((char *)data - sizeof(struct data_msg_t));
  msg->stream = htoye64(stream);
  return ytp_time_commit(yamal, ts, (char *)msg, YTP_STREAM_LIST_DATA, error);
}

void ytp_data_sublist_commit(ytp_yamal_t *yamal, int64_t ts,
                             ytp_mmnode_offs stream, void **first_ptr,
                             void **last_ptr, void *data, fmc_error_t **error) {
  struct data_msg_t *msg =
      (struct data_msg_t *)((char *)data - sizeof(struct data_msg_t));
  msg->stream = htoye64(stream);
  ytp_time_sublist_commit(yamal, ts, first_ptr, last_ptr, (char *)msg, error);
}

ytp_iterator_t ytp_data_sublist_finalize(ytp_yamal_t *yamal, void *first_ptr,
                                         fmc_error_t **error) {
  return ytp_yamal_commit(yamal, first_ptr, YTP_STREAM_LIST_DATA, error);
}

void ytp_data_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno,
                   int64_t *ts, ytp_mmnode_offs *stream, size_t *sz,
                   const char **data, fmc_error_t **error) {
  size_t read_sz;
  const struct data_msg_t *msg;
  ytp_time_read(yamal, iterator, seqno, ts, &read_sz, (const char **)&msg,
                error);
  if (*error) {
    return;
  }

  *stream = ye64toh(msg->stream);
  *sz = read_sz - offsetof(struct data_msg_t, data);
  *data = msg->data;
}
