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
#include <cstring>
#include <stdbool.h> 
#include <ytp/peer.h>
#include <ytp/yamal.h>
#include <ytp/errno.h> // ytp_status_t
#include "endianness.h"

struct ytp_peer_hdr {
  ytp_peer_t id;
};

struct ytp_peer_msg {
  ytp_peer_hdr hdr;
  char data[];
};

APR_DECLARE(ytp_status_t) ytp_peer_reserve(ytp_yamal_t *yamal, char **buf, apr_size_t size) {
  ytp_peer_msg *peer_msg = NULL;
  ytp_status_t rv = ytp_yamal_reserve(yamal, (char**)&peer_msg, size + sizeof(ytp_peer_hdr));
  if(rv) {
    return rv;
  }
  *buf = peer_msg->data;
  return rv;
}

APR_DECLARE(ytp_status_t) ytp_peer_commit(ytp_yamal_t *yamal, ytp_iterator_t *it, ytp_peer_t peer, void *data) {
  ytp_peer_msg *peer_msg = (ytp_peer_msg *)((char *)data - sizeof(ytp_peer_hdr));
  peer_msg->hdr.id = _htobe64(peer);
  return ytp_yamal_commit(yamal, it, peer_msg);
}

APR_DECLARE(ytp_status_t) ytp_peer_name(ytp_yamal_t *yamal, ytp_iterator_t *it, apr_size_t sz, const char *name) {
  char *dst = NULL;
  ytp_status_t rv = ytp_peer_reserve(yamal, &dst, sz);
  if(rv) {
    return rv;
  }
  memcpy(dst, name, sz);
  return ytp_peer_commit(yamal, it, 0, dst);
}

APR_DECLARE(ytp_status_t) ytp_peer_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                   ytp_peer_t *peer, apr_size_t *size, const char **data) {
  const ytp_peer_msg *peer_msg;
  ytp_status_t rv = ytp_yamal_read(yamal, iterator, size, (const char **)&peer_msg);
  if(rv) {
    return rv;
  }
  *peer = _be64toh(peer_msg->hdr.id);
  *data = peer_msg->data;
  *size -= sizeof(ytp_peer_hdr);
  return rv;
}

APR_DECLARE(bool) ytp_peer_ann(ytp_peer_t peer) { return peer == YTP_PEER_ANN; }
