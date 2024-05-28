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

#include <cstring>
#include <fmc/endianness.h>
#include <fmc/error.h>
#include <ytp/peer.h>
#include <ytp/yamal.h>

struct ytp_peer_hdr {
  ytp_peer_t id;
};

struct ytp_peer_msg {
  ytp_peer_hdr hdr;
  char data[];
};

char *ytp_peer_reserve(ytp_yamal_t *yamal, size_t size, fmc_error_t **error) {
  if (auto *peer_msg = (ytp_peer_msg *)ytp_yamal_reserve(
          yamal, size + sizeof(ytp_peer_hdr), error);
      peer_msg) {
    return peer_msg->data;
  }

  return nullptr;
}

ytp_iterator_t ytp_peer_commit(ytp_yamal_t *yamal, ytp_peer_t peer, void *data,
                               fmc_error_t **error) {
  auto *peer_msg = (ytp_peer_msg *)((char *)data - sizeof(ytp_peer_hdr));
  peer_msg->hdr.id = fmc_htobe64(peer);
  return ytp_yamal_commit(yamal, peer_msg, error);
}

void ytp_peer_sublist_commit(ytp_yamal_t *yamal, ytp_peer_t peer,
                             void **first_ptr, void **last_ptr, void *new_ptr,
                             fmc_error_t **error) {
  auto *peer_msg = (ytp_peer_msg *)((char *)new_ptr - sizeof(ytp_peer_hdr));
  peer_msg->hdr.id = fmc_htobe64(peer);
  return ytp_yamal_sublist_commit(yamal, first_ptr, last_ptr, peer_msg, error);
}

ytp_iterator_t ytp_peer_name(ytp_yamal_t *yamal, size_t sz, const char *name,
                             fmc_error_t **error) {
  if (auto *dst = ytp_peer_reserve(yamal, sz, error); dst) {
    memcpy(dst, name, sz);
    return ytp_peer_commit(yamal, 0, dst, error);
  }

  return nullptr;
}

void ytp_peer_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                   ytp_peer_t *peer, size_t *size, const char **data,
                   fmc_error_t **error) {
  const ytp_peer_msg *peer_msg;
  ytp_yamal_read(yamal, iterator, size, (const char **)&peer_msg, error);
  if (!*error) {
    *peer = fmc_be64toh(peer_msg->hdr.id);
    *data = peer_msg->data;
    *size -= sizeof(ytp_peer_hdr);
  }
}

bool ytp_peer_ann(ytp_peer_t peer) { return peer == YTP_PEER_ANN; }