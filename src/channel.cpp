#include <fmc/endianness.h>
#include <ytp/channel.h>
#include <ytp/peer.h>
#include <ytp/yamal.h>

struct ytp_channel_hdr {
  ytp_channel_t id;
};

struct ytp_channel_msg {
  ytp_channel_hdr hdr;
  char data[];
};

char *ytp_channel_reserve(ytp_yamal_t *yamal, size_t size,
                          fmc_error_t **error) {
  if (auto *channel_msg = (ytp_channel_msg *)ytp_peer_reserve(
          yamal, size + sizeof(ytp_channel_hdr), error);
      channel_msg) {
    return channel_msg->data;
  }

  return nullptr;
}

ytp_iterator_t ytp_channel_commit(ytp_yamal_t *yamal, ytp_peer_t peer,
                                  ytp_channel_t channel, void *data,
                                  fmc_error_t **error) {
  auto *channel_msg =
      (ytp_channel_msg *)((char *)data - sizeof(ytp_channel_hdr));
  channel_msg->hdr.id = fmc_htobe64(channel);
  return ytp_peer_commit(yamal, peer, channel_msg, error);
}

void ytp_channel_read(ytp_yamal_t *yamal, ytp_iterator_t iterator,
                      ytp_peer_t *peer, ytp_channel_t *channel, size_t *size,
                      const char **data, fmc_error_t **error) {
  const ytp_channel_msg *channel_msg;
  ytp_peer_read(yamal, iterator, peer, size, (const char **)&channel_msg,
                error);
  if (!*error) {
    if (*peer == 0) {
      *channel = 0;
      *data = (const char *)channel_msg;
    } else {
      *channel = fmc_be64toh(channel_msg->hdr.id);
      *data = channel_msg->data;
      *size -= sizeof(ytp_channel_hdr);
    }
  }
}
