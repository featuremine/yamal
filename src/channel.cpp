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
#include <ytp/channel.h>
#include <ytp/peer.h> // ytp_peer_t
#include <ytp/yamal.h> // ytp_yamal_t
#include <ytp/errno.h> // ytp_status_t
#include "endianness.h" // _htobe64

struct ytp_channel_hdr {
  ytp_channel_t id;
};

struct ytp_channel_msg {
  ytp_channel_hdr hdr;
  char data[];
};

APR_DECLARE(ytp_status_t) ytp_channel_reserve(ytp_yamal_t *yamal, char **buf, apr_size_t size) {
  ytp_channel_msg *channel_msg = NULL;
  ytp_status_t rv = ytp_peer_reserve(yamal, (char**)&channel_msg, size + sizeof(ytp_channel_hdr));
  if(rv) {
    return rv;
  }
  *buf = channel_msg->data;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_channel_commit(ytp_yamal_t *yamal, ytp_iterator_t *it, ytp_peer_t peer,
                                  ytp_channel_t channel, void *data) {
  ytp_channel_msg *channel_msg =
      (ytp_channel_msg *)((char *)data - sizeof(ytp_channel_hdr));
  channel_msg->hdr.id = _htobe64(channel);
  return ytp_peer_commit(yamal, it, peer, channel_msg);
}

APR_DECLARE(ytp_status_t) ytp_channel_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                      ytp_peer_t *peer, ytp_channel_t *channel, apr_size_t *size,
                      const char **data) {
  const ytp_channel_msg *channel_msg;
  ytp_status_t rv = ytp_peer_read(yamal, iterator, peer, size, (const char **)&channel_msg);
  if(rv) {
    return rv;
  }
  if (*peer == 0) {
    *channel = 0;
    *data = (const char *)channel_msg;
  } else {
    *channel = _be64toh(channel_msg->hdr.id);
    *data = channel_msg->data;
    *size -= sizeof(ytp_channel_hdr);
  }
  return rv;
}
