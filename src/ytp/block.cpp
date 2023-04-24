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

#include <ytp/block.h>
#include <ytp/sequence.h>
#include <fmc/files.h>
#include "sequence.hpp"

struct ytp_block {
  ytp_sequence_t seq;
  std::string pattern;
  fmc_fmode mode;
  size_t maxsize;
  size_t curridx;
  fmc_fd currfd;
  // Callback related structures to be able to move them from one sequence to the next
};

std::string file_name(std::string prefix, uint32_t idx) {
  char str[6];
  snprintf(str, sizeof(str), ".%04u", idx);
  return prefix + str;
}

ytp_block_t *ytp_block_new(const char* pattern, size_t maxsize, fmc_fmode mode, fmc_error_t **error) {
    auto *seq = new ytp_block_t();
    seq->pattern = pattern;
    seq->mode = mode;
    seq->maxsize = maxsize;
    seq->curridx = 1;
    seq->currfd = fmc_fopen(file_name(seq->pattern, seq->curridx).c_str(), seq->mode, error);
    if (error) goto cleanup;
    ytp_sequence_init(&seq->seq, seq->currfd, error);
    if (error) goto cleanup;
    return seq;
cleanup:
  if (fmc_fvalid(seq->currfd)) {
    fmc_fclose(seq->currfd, error);
  }
  delete seq;
  return NULL;
}

void ytp_block_del(ytp_block_t *seq, fmc_error_t **error) {
  ytp_sequence_destroy(&seq->seq, error);
  fmc_fclose(seq->currfd, error);
  delete seq;
}

char *ytp_block_reserve(ytp_block_t *seq, size_t sz,
                                    fmc_error_t **error) {
//   // TODO: Handle errors correctly
//   if (sz > seq->maxsize) {
//     fmc_error_set(error, "Requested size is larger than maximum file size");
//     return nullptr;
//   }
//   auto currsz = ytp_yamal_reserved_size(&seq->seq.ctrl.yamal, error);
//   if (currsz + sz > seq->maxsize) {
//     ytp_sequence_close(&seq->seq, error);
//     // initialize new sequence
//     fmc_fd oldfd = seq->currfd;
//     ytp_sequence_t oldseq = seq->seq;

//     seq->currfd = fmc_fopen(file_name(seq->pattern, ++seq->curridx).c_str(), seq->mode, error);

//     // change control
//     // timeline read it points to begining of new file

//     ytp_sequence_init(&seq->seq, seq->currfd, error);

//     //TODO: migrate callbacks

//     // close old sequence
//     ytp_sequence_destroy(&oldseq, error);
//     fmc_fclose(oldfd, error);

//     return ytp_block_reserve(seq, sz, error);
//   }
//   return ytp_sequence_reserve(&seq->seq, sz, error);
}

ytp_iterator_t ytp_block_commit(ytp_block_t *seq, ytp_peer_t peer,
                                            ytp_channel_t channel, uint64_t time,
                                            void *data, fmc_error_t **error) {
//   // TODO: Handle errors correctly
//   auto it = ytp_control_commit(&seq->seq.ctrl, peer, channel, time, data, error);
//   if (*error && (*error)->code == FMC_ERROR_FILE_END) {
//     // open new file according to pattern
//     fmc_fd oldfd = seq->currfd;
//     ytp_sequence_t oldseq = seq->seq;

//     seq->currfd = fmc_fopen(file_name(seq->pattern, ++seq->curridx).c_str(), seq->mode, error);
//     ytp_sequence_init(&seq->seq, seq->currfd, error);

//     // reserve and commit data in new sequence
//     auto *node = mmnode_node_from_data(data);
//     auto sz = node->size.value();
//     auto *buff = ytp_block_reserve(&seq->seq, sz, error) {
//     memcpy(buff, data, sz);
//     it = ytp_block_commit(seq, peer, channel, time, buff, error);

//     //TODO: migrate callbacks

//     // clean up old sequence
//     ytp_sequence_destroy(&oldseq, error);
//     fmc_fclose(oldfd, error);
//   }
//   return it;
}

void ytp_block_dir(ytp_block_t *seq, ytp_peer_t peer,
                                uint64_t time, size_t sz, const char *payload,
                                fmc_error_t **error) {}

void ytp_block_ch_name(ytp_block_t *seq, ytp_channel_t channel,
                                    size_t *sz, const char **name,
                                    fmc_error_t **error) {}

ytp_channel_t ytp_block_ch_decl(ytp_block_t *seq,
                                             ytp_peer_t peer, uint64_t time,
                                             size_t sz, const char *name,
                                             fmc_error_t **error) {}

void ytp_block_ch_cb(ytp_block_t *seq, ytp_sequence_ch_cb_t cb,
                                  void *closure, fmc_error_t **error) {}

void ytp_block_ch_cb_rm(ytp_block_t *seq,
                                     ytp_sequence_ch_cb_t cb, void *closure,
                                     fmc_error_t **error) {}

void ytp_block_peer_name(ytp_block_t *seq, ytp_peer_t peer,
                                      size_t *sz, const char **name,
                                      fmc_error_t **error) {}

ytp_peer_t ytp_block_peer_decl(ytp_block_t *seq, size_t sz,
                                            const char *name,
                                            fmc_error_t **error) {}

void ytp_block_peer_cb(ytp_block_t *seq,
                                    ytp_sequence_peer_cb_t cb, void *closure,
                                    fmc_error_t **error) {}

void ytp_block_peer_cb_rm(ytp_block_t *seq,
                                       ytp_sequence_peer_cb_t cb, void *closure,
                                       fmc_error_t **error) {}


void ytp_block_prfx_cb(ytp_block_t *seq, size_t sz,
                                    const char *prfx, ytp_sequence_data_cb_t cb,
                                    void *closure, fmc_error_t **error) {}

void ytp_block_prfx_cb_rm(ytp_block_t *seq, size_t sz,
                                       const char *prfx,
                                       ytp_sequence_data_cb_t cb, void *closure,
                                       fmc_error_t **error) {}

void ytp_block_indx_cb(ytp_block_t *seq, ytp_channel_t channel,
                                    ytp_sequence_data_cb_t cb, void *closure,
                                    fmc_error_t **error) {}

void ytp_block_indx_cb_rm(ytp_block_t *seq,
                                       ytp_channel_t channel,
                                       ytp_sequence_data_cb_t cb, void *closure,
                                       fmc_error_t **error) {}

bool ytp_block_poll(ytp_block_t *seq, fmc_error_t **error) {}

bool ytp_block_term(ytp_block_t *seq) {}

ytp_iterator_t ytp_block_end(ytp_block_t *seq,
                                          fmc_error_t **error) {}

ytp_iterator_t ytp_block_cur(ytp_block_t *seq) {}

ytp_iterator_t ytp_block_get_it(ytp_block_t *seq) {}

void ytp_block_set_it(ytp_block_t *seq,
                                   ytp_iterator_t iterator) {}

void ytp_block_cb_rm(ytp_block_t *seq) {}

ytp_iterator_t ytp_block_seek(ytp_block_t *seq, size_t off,
                                           fmc_error_t **error) {}

size_t ytp_block_tell(ytp_block_t *seq, ytp_iterator_t iterator,
                                   fmc_error_t **error) {}
