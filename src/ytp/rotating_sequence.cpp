#include <ytp/sequence.h>
#include "sequence.hpp"

struct ytp_rotating_sequence;
typedef struct ytp_rotating_sequence ytp_rotating_sequence_t;

struct ytp_rotating_sequence {
  ytp_sequence_t seq;
  std::string pattern;
  size_t size;
};

ytp_rotating_sequence_t *ytp_rotating_sequence_new(const char* pattern, size_t maxsize, fmc_error_t **error) {
    // Open sequence using first file compliant with pattern
}

void ytp_rotating_sequence_del(ytp_rotating_sequence_t *seq, fmc_error_t **error) {

}

char *ytp_rotating_sequence_reserve(ytp_rotating_sequence_t *seq, size_t sz,
                                    fmc_error_t **error) {
  auto currsz = ytp_yamal_reserved_size(&seq->seq.ctrl.yamal, error);
  if (*error) return nullptr;
  if (currsz + sz > seq->size) {
    // open new file according to pattern
    // point internal sequence to new sequence, close old sequence
    // reserve and rotate there
  }
  return ytp_sequence_reserve(&seq->seq, sz, error);
}

ytp_iterator_t ytp_rotating_sequence_commit(ytp_rotating_sequence_t *seq, ytp_peer_t peer,
                                            ytp_channel_t channel, uint64_t time,
                                            void *data, fmc_error_t **error) {
  auto it = ytp_control_commit(&seq->seq.ctrl, peer, channel, time, data, error);
  if (*error && (*error)->code == FMC_ERROR_FILE_END) {
    // open new file according to pattern
    // point internal sequence to new sequence, close old sequence
    // reserve and commit and rotate there
  }
  return it;
}

void ytp_rotating_sequence_dir(ytp_rotating_sequence_t *seq, ytp_peer_t peer,
                                uint64_t time, size_t sz, const char *payload,
                                fmc_error_t **error) {}

void ytp_rotating_sequence_ch_name(ytp_rotating_sequence_t *seq, ytp_channel_t channel,
                                    size_t *sz, const char **name,
                                    fmc_error_t **error) {}

ytp_channel_t ytp_rotating_sequence_ch_decl(ytp_rotating_sequence_t *seq,
                                             ytp_peer_t peer, uint64_t time,
                                             size_t sz, const char *name,
                                             fmc_error_t **error) {}

void ytp_rotating_sequence_ch_cb(ytp_rotating_sequence_t *seq, ytp_sequence_ch_cb_t cb,
                                  void *closure, fmc_error_t **error) {}

void ytp_rotating_sequence_ch_cb_rm(ytp_rotating_sequence_t *seq,
                                     ytp_sequence_ch_cb_t cb, void *closure,
                                     fmc_error_t **error) {}

void ytp_rotating_sequence_peer_name(ytp_rotating_sequence_t *seq, ytp_peer_t peer,
                                      size_t *sz, const char **name,
                                      fmc_error_t **error) {}

ytp_peer_t ytp_rotating_sequence_peer_decl(ytp_rotating_sequence_t *seq, size_t sz,
                                            const char *name,
                                            fmc_error_t **error) {}

void ytp_rotating_sequence_peer_cb(ytp_rotating_sequence_t *seq,
                                    ytp_sequence_peer_cb_t cb, void *closure,
                                    fmc_error_t **error) {}

void ytp_rotating_sequence_peer_cb_rm(ytp_rotating_sequence_t *seq,
                                       ytp_sequence_peer_cb_t cb, void *closure,
                                       fmc_error_t **error) {}


void ytp_rotating_sequence_prfx_cb(ytp_rotating_sequence_t *seq, size_t sz,
                                    const char *prfx, ytp_sequence_data_cb_t cb,
                                    void *closure, fmc_error_t **error) {}

void ytp_rotating_sequence_prfx_cb_rm(ytp_rotating_sequence_t *seq, size_t sz,
                                       const char *prfx,
                                       ytp_sequence_data_cb_t cb, void *closure,
                                       fmc_error_t **error) {}

void ytp_rotating_sequence_indx_cb(ytp_rotating_sequence_t *seq, ytp_channel_t channel,
                                    ytp_sequence_data_cb_t cb, void *closure,
                                    fmc_error_t **error) {}

void ytp_rotating_sequence_indx_cb_rm(ytp_rotating_sequence_t *seq,
                                       ytp_channel_t channel,
                                       ytp_sequence_data_cb_t cb, void *closure,
                                       fmc_error_t **error) {}

bool ytp_rotating_sequence_poll(ytp_rotating_sequence_t *seq, fmc_error_t **error) {}

bool ytp_rotating_sequence_term(ytp_rotating_sequence_t *seq) {}

ytp_iterator_t ytp_rotating_sequence_end(ytp_rotating_sequence_t *seq,
                                          fmc_error_t **error) {}

ytp_iterator_t ytp_rotating_sequence_cur(ytp_rotating_sequence_t *seq) {}

ytp_iterator_t ytp_rotating_sequence_get_it(ytp_rotating_sequence_t *seq) {}

void ytp_rotating_sequence_set_it(ytp_rotating_sequence_t *seq,
                                   ytp_iterator_t iterator) {}

void ytp_rotating_sequence_cb_rm(ytp_rotating_sequence_t *seq) {}

ytp_iterator_t ytp_rotating_sequence_seek(ytp_rotating_sequence_t *seq, size_t off,
                                           fmc_error_t **error) {}

size_t ytp_rotating_sequence_tell(ytp_rotating_sequence_t *seq, ytp_iterator_t iterator,
                                   fmc_error_t **error) {}
