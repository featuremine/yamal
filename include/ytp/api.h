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
typedef void (*ytp_sequence_peer_cb_t)(void *closure, ytp_peer_t peer,
                                       size_t sz, const char *name);
typedef void (*ytp_sequence_ch_cb_t)(void *closure, ytp_peer_t peer,
                                     ytp_channel_t channel, uint64_t time,
                                     size_t sz, const char *name);
typedef void (*ytp_sequence_data_cb_t)(void *closure, ytp_peer_t peer,
                                       ytp_channel_t channel, uint64_t time,
                                       size_t sz, const char *data);

struct ytp_sequence_api_v1 {
  // Do we need new/init/del method?

  // Reserves memory for data in the memory mapped list
  char *(*sequence_reserve)(shared_sequence *, size_t, fmc_error_t **);
  // Commits the data to the memory mapped list
  ytp_iterator_t (*sequence_commit)(shared_sequence *, ytp_peer_t, ytp_channel_t, uint64_t,
                                        void *, fmc_error_t **);
  // Publishes a subscription message
  void (*sequence_sub)(shared_sequence *, ytp_peer_t ,
                       uint64_t, size_t, const char *,
                       fmc_error_t **);
  // Publishes a directory message
  void (*sequence_dir)(shared_sequence *, ytp_peer_t ,
                       uint64_t , size_t , const char *,
                       fmc_error_t **);
  // Returns the name of the channel, given the channel reference
  void (*sequence_ch_name)(shared_sequence *, ytp_channel_t,
                                    size_t *, const char **,
                                    fmc_error_t **);
  // Declares an existing/new channel
  ytp_channel_t (*sequence_ch_decl)(shared_sequence *,
                                    ytp_peer_t , uint64_t ,
                                    size_t , const char *,
                                    fmc_error_t **);
  // Registers a channel announcement callback
  void (*sequence_ch_cb)(shared_sequence *, ytp_sequence_ch_cb_t,
                         void *, fmc_error_t **);
  // Unregisters a channel announcement callback
  void (*sequence_ch_cb_rm)(shared_sequence *,
                            ytp_sequence_ch_cb_t , void *,
                            fmc_error_t **);
  // Returns the name of the peer, given the peer reference
  void (*sequence_peer_name)(shared_sequence *, ytp_peer_t ,
                             size_t *, const char **,
                             fmc_error_t **);
  // Declares an existing/new peer
  ytp_peer_t (*sequence_peer_decl)(shared_sequence *, size_t ,
                                   const char *,
                                   fmc_error_t **);
  // Registers a peer announcement callback
  void (*sequence_peer_cb)(shared_sequence *seq,
                                    ytp_sequence_peer_cb_t cb, void *closure,
                                    fmc_error_t **error);
  // Unregisters a peer announcement callback
  void (*sequence_peer_cb_rm)(shared_sequence *seq,
                                       ytp_sequence_peer_cb_t cb, void *closure,
                                       fmc_error_t **error);
  // Registers a channel data callback by channel name or prefix
  void (*sequence_prfx_cb)(shared_sequence *seq, size_t sz,
                                    const char *prfx, ytp_sequence_data_cb_t cb,
                                    void *closure, fmc_error_t **error);
  // Unregisters a channel data callback by channel name or prefix
  void (*sequence_prfx_cb_rm)(shared_sequence *seq, size_t sz,
                                       const char *prfx,
                                       ytp_sequence_data_cb_t cb, void *closure,
                                       fmc_error_t **error);
  // Registers a channel data callback by channel handler
  void (*sequence_indx_cb)(shared_sequence *, ytp_channel_t ,
                           ytp_sequence_data_cb_t, void *,
                           fmc_error_t **);
  // Unregisters a channel data callback by channel handler
  void (*sequence_indx_cb_rm)(shared_sequence *,
                               ytp_channel_t,
                               ytp_sequence_data_cb_t, void *,
                               fmc_error_t **);
  // Reads one message and executes the callbacks that applies.
  bool (*sequence_poll)(shared_sequence*, fmc_error_t**);
  // Checks if there are not more messages
  bool (*sequence_term)(shared_sequence*);
  // Returns the iterator to the end of yamal
  ytp_iterator_t (*sequence_end)(shared_sequence*, fmc_error_t**);
  // Returns the current data iterator
  ytp_iterator_t (*sequence_cur)(shared_sequence*);
  // Returns the current data iterator
  ytp_iterator_t (*sequence_get_it)(shared_sequence *);
  // Sets the current data iterator
  void (*sequence_set_it)(shared_sequence *,
                          ytp_iterator_t);
  // Returns an iterator given a serializable offset
  ytp_iterator_t (*sequence_seek)(shared_sequence *seq, size_t off,
                                           fmc_error_t **error);
  // Returns serializable offset given an iterator
  size_t (*sequence_tell)(shared_sequence *seq, ytp_iterator_t iterator,
                                   fmc_error_t **error);
  // Increases the reference counter
  void (*sequence_shared_inc)(shared_sequence*);
  // Decreases the reference counter and call ytp_sequence_del the sequence
  void (*sequence_shared_dec)(shared_sequence*, fmc_error_t **);
};

// function that you can call to return the actual sequence api structure pointer
ytp_sequence_api_v1* ytp_sequence_api_v1_get();

#ifdef __cplusplus
}
#endif
