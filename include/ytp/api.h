#include <fmc/error.h>
#include <fmc/files.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void shared_sequence;
typedef uint64_t ytp_peer_t;
typedef uint64_t ytp_channel_t;
typedef void *ytp_iterator_t;
typedef void (*ytp_sequence_data_cb_t)(void *closure, ytp_peer_t peer,
                                       ytp_channel_t channel, uint64_t time,
                                       size_t sz, const char *data);
typedef void mml;

struct ytp_sequence_api_v1 {
  // Sequence api
  bool (*sequence_poll)(shared_sequence*, fmc_error_t**);
  char *(*sequence_reserve)(shared_sequence *, size_t, fmc_error_t **);
  ytp_iterator_t (*sequence_commit)(shared_sequence *, ytp_peer_t, ytp_channel_t, uint64_t,
                                        void *, fmc_error_t **);
  void (*sequence_shared_inc)(shared_sequence*);
  void (*sequence_shared_dec)(shared_sequence*, fmc_error_t **);
  ytp_iterator_t (*sequence_get_it)(shared_sequence *);
  void (*sequence_indx_cb)(shared_sequence *, ytp_channel_t ,
                           ytp_sequence_data_cb_t, void *,
                           fmc_error_t **);
  void (*sequence_indx_cb_rm)(shared_sequence *,
                               ytp_channel_t,
                               ytp_sequence_data_cb_t, void *,
                               fmc_error_t **);
  void (*sequence_set_it)(shared_sequence *,
                          ytp_iterator_t);

  // Peer api
  mml* (*yamal_new)(fmc_fd, fmc_error_t**);
  ytp_iterator_t (*yamal_begin)(mml*, fmc_error_t **);
  void (*yamal_del)(mml*, fmc_error_t **);
  void (*peer_read)(mml *, ytp_iterator_t,
                    ytp_peer_t *, size_t *, const char **,
                    fmc_error_t **);
  ytp_iterator_t (*yamal_next)(mml *, ytp_iterator_t,
                               fmc_error_t **error);
  bool (*yamal_term)(ytp_iterator_t);
};

// function that you can call to return the actual sequence api structure pointer
ytp_sequence_api_v1* ytp_sequence_api_v1_get();

#ifdef __cplusplus
}
#endif
