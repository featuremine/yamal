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

#include <ytp/time.h>
#include <ytp/yamal.h>

struct ytp_time_hdr {
  int64_t ts;
};

struct ytp_time_msg {
  ytp_time_hdr hdr;
  char data[];
};

char *ytp_time_reserve(ytp_yamal_t *yamal, size_t size, fmc_error_t **error) {
  fmc_error_clear(error);
  auto *time_msg = (ytp_time_msg *)ytp_yamal_reserve(
      yamal, size + sizeof(ytp_time_hdr), error);
  if (*error) {
    return nullptr;
  }

  return time_msg->data;
}

ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, int64_t ts, void *data,
                               size_t listidx, fmc_error_t **error) {
  auto *time_msg = (ytp_time_msg *)((char *)data - sizeof(ytp_time_hdr));
  time_msg->hdr.ts = htoye64(ts);
  return ytp_yamal_commit(yamal, time_msg, listidx, error);
}

void ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno,
                   int64_t *ts, size_t *size, const char **data,
                   fmc_error_t **error) {
  const ytp_time_msg *time_msg;
  ytp_yamal_read(yamal, iterator, seqno, size, (const char **)&time_msg, error);
  if (*error) {
    return;
  }

  *ts = ye64toh(time_msg->hdr.ts);
  *data = time_msg->data;
  *size -= sizeof(ytp_time_hdr);
}
