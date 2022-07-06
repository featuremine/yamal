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

#include <apr.h> // apr_size_t APR_DECLARE
#include <ytp/time.h>
#include <ytp/channel.h>
#include <ytp/peer.h> // ytp_peer_t
#include <ytp/yamal.h> // ytp_yamal_t
#include <ytp/errno.h> // ytp_status_t
#include "endianness.h" // _htobe64


struct ytp_time_hdr {
  uint64_t time;
};

struct ytp_time_msg {
  ytp_time_hdr hdr;
  char data[];
};

APR_DECLARE(ytp_status_t) ytp_time_reserve(ytp_yamal_t *yamal, char **buf, apr_size_t size) {
  ytp_time_msg *time_msg = NULL;
  ytp_status_t rv = ytp_channel_reserve(yamal, (char**)&time_msg, size + sizeof(ytp_time_hdr));
  if(rv) {
    return rv;
  }
  *buf = time_msg->data;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_time_commit(ytp_yamal_t *yamal, ytp_iterator_t *it, ytp_peer_t peer,
                               ytp_channel_t channel, uint64_t time, void *data) {
  ytp_time_msg *time_msg = (ytp_time_msg *)((char *)data - sizeof(ytp_time_hdr));
  time_msg->hdr.time = _htobe64(time);
  return ytp_channel_commit(yamal, it, peer, channel, time_msg);
}

APR_DECLARE(ytp_status_t) ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                                        ytp_peer_t *peer, ytp_channel_t *channel,
                                        uint64_t *time, apr_size_t *size, const char **data) {
  const ytp_time_msg *time_msg;
  ytp_status_t rv = ytp_channel_read(yamal, iterator, peer, channel, size, (const char **)&time_msg);
  if(rv) {
    return rv;
  }
  if (*peer == 0) {
    *time = 0;
    *data = (const char *)time_msg;
  } else {
    *time = _be64toh(time_msg->hdr.time);
    *data = time_msg->data;
    *size -= sizeof(ytp_time_hdr);
  }
  return rv;
}
