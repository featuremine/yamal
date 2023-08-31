/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include "endianess.h"

#include <fmc/error.h>

#include <ytp/index.h>
#include <ytp/streams.h>

#include <string.h>

struct idx_msg_t {
  uint64_t stream;
  uint64_t offset;
  char payload[];
};

ytp_iterator_t ytp_index_write(ytp_yamal_t *yamal, ytp_mmnode_offs stream,
                               ytp_mmnode_offs offset, size_t sz,
                               const void *payload, fmc_error_t **error) {
  fmc_error_clear(error);

  struct idx_msg_t *msg = (struct idx_msg_t *)ytp_yamal_reserve(
      yamal, sz + sizeof(struct idx_msg_t), error);
  if (*error) {
    return NULL;
  }

  msg->stream = htoye64(stream);
  msg->offset = htoye64(offset);
  memcpy(msg->payload, payload, sz);
  return ytp_yamal_commit(yamal, (char *)msg, YTP_STREAM_LIST_INDX, error);
}

void ytp_index_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                    uint64_t *seqno, ytp_mmnode_offs *stream,
                    ytp_mmnode_offs *offset, size_t *sz, const char **payload,
                    fmc_error_t **error) {
  size_t msg_sz;
  struct idx_msg_t *msg;
  ytp_yamal_read(yamal, iterator, seqno, &msg_sz, (const char **)&msg, error);
  if (*error) {
    return;
  }

  if (msg_sz < sizeof(struct idx_msg_t)) {
    fmc_error_set(error, "invalid index message");
    return;
  }

  *stream = ye64toh(msg->stream);
  *offset = ye64toh(msg->offset);
  *sz = msg_sz - sizeof(struct idx_msg_t);
  *payload = msg->payload;
}

void ytp_index_lookup(ytp_yamal_t *yamal, ytp_mmnode_offs offset,
                      uint64_t *seqno, ytp_mmnode_offs *stream,
                      ytp_mmnode_offs *data_offset, size_t *sz,
                      const char **payload, fmc_error_t **error) {
  ytp_iterator_t iterator = ytp_yamal_seek(yamal, offset, error);
  if (*error) {
    return;
  }

  ytp_index_read(yamal, iterator, seqno, stream, data_offset, sz, payload,
                 error);
}
