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

#include <fmc/endianness.h>
#include <ytp/channel.h>
#include <ytp/time.h>
#include <ytp/yamal.h>

struct ytp_time_hdr {
  uint64_t time;
};

struct ytp_time_msg {
  ytp_time_hdr hdr;
  char data[];
};

char *ytp_time_reserve(ytp_yamal_t *yamal, size_t size, fmc_error_t **error) {
  fmc_error_clear(error);
  if (auto *time_msg = (ytp_time_msg *)ytp_channel_reserve(
          yamal, size + sizeof(ytp_time_hdr), error);
      time_msg) {
    return time_msg->data;
  }

  return nullptr;
}

ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, ytp_peer_t peer,
                               ytp_channel_t channel, uint64_t time, void *data,
                               fmc_error_t **error) {
  auto *time_msg = (ytp_time_msg *)((char *)data - sizeof(ytp_time_hdr));
  time_msg->hdr.time = fmc_htobe64(time);
  return ytp_channel_commit(yamal, peer, channel, time_msg, error);
}

void ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                   ytp_peer_t *peer, ytp_channel_t *channel, uint64_t *time,
                   size_t *size, const char **data, fmc_error_t **error) {
  const ytp_time_msg *time_msg;
  ytp_channel_read(yamal, iterator, peer, channel, size,
                   (const char **)&time_msg, error);
  if (!*error) {
    if (*peer == 0) {
      *time = 0;
      *data = (const char *)time_msg;
    } else {
      *time = fmc_be64toh(time_msg->hdr.time);
      *data = time_msg->data;
      *size -= sizeof(ytp_time_hdr);
    }
  }
}
