Allocates and initializes a ytp_sequence_t object

```c
ytp_sequence_t *ytp_sequence_new(fmc_fd fd, fmc_error_t **error);
```

Destroys and deallocate a ytp_sequence_t object

```c
void ytp_sequence_del(ytp_sequence_t *seq, fmc_error_t **error);
```

Initializes a ytp_sequence_t object

```c
void ytp_sequence_init(ytp_sequence_t *seq, fmc_fd fd, fmc_error_t **error);
```

Destroys a ytp_sequence_t object

```c
void ytp_sequence_destroy(ytp_sequence_t *seq, fmc_error_t **error);
```

Reserves memory for data in the memory mapped list

```c
char *ytp_sequence_reserve(ytp_sequence_t *seq, size_t sz, fmc_error_t **error);
```

Commits the data to the memory mapped list

```c
ytp_iterator_t ytp_sequence_commit(ytp_sequence_t *seq, ytp_peer_t peer, ytp_channel_t channel, uint64_t time, void *data, fmc_error_t **error);
```

Publishes a subscription message

```c
void ytp_sequence_sub(ytp_sequence_t *seq, ytp_peer_t peer, uint64_t time, size_t sz, const char *payload, fmc_error_t **error);
```

Publishes a directory message

```c
void ytp_sequence_dir(ytp_sequence_t *seq, ytp_peer_t peer, uint64_t time, size_t sz, const char *payload, fmc_error_t **error);
```

Returns the name of the channel, given the channel reference

```c
void ytp_sequence_ch_name(ytp_sequence_t *seq, ytp_channel_t channel, size_t *sz, const char **name, fmc_error_t **error);
```

Declares an existing/new channel

```c
ytp_channel_t ytp_sequence_ch_decl(ytp_sequence_t *seq, ytp_peer_t peer, uint64_t time, size_t sz, const char *name, fmc_error_t **error);
```

Registers a channel announcement callback

```c
void ytp_sequence_ch_cb(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb, void *closure, fmc_error_t **error);
```

Unregisters a channel announcement callback

```c
void ytp_sequence_ch_cb_rm(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb, void *closure, fmc_error_t **error);
```

Returns the name of the peer, given the peer reference

```c
void ytp_sequence_peer_name(ytp_sequence_t *seq, ytp_peer_t peer, size_t *sz, const char **name, fmc_error_t **error);
```

Declares an existing/new peer

```c
ytp_peer_t ytp_sequence_peer_decl(ytp_sequence_t *seq, size_t sz, const char *name, fmc_error_t **error);
```

Registers a peer announcement callback

```c
void ytp_sequence_peer_cb(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb, void *closure, fmc_error_t **error);
```

Unregisters a peer announcement callback

```c
void ytp_sequence_peer_cb_rm(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb, void *closure, fmc_error_t **error);
```

Registers a channel data callback by channel name or prefix

```c
void ytp_sequence_prfx_cb(ytp_sequence_t *seq, size_t sz, const char *prfx, ytp_sequence_data_cb_t cb, void *closure, fmc_error_t **error);
```

Unregisters a channel data callback by channel name or prefix

```c
void ytp_sequence_prfx_cb_rm(ytp_sequence_t *seq, size_t sz, const char *prfx, ytp_sequence_data_cb_t cb, void *closure, fmc_error_t **error);
```

Registers a channel data callback by channel handler

```c
void ytp_sequence_indx_cb(ytp_sequence_t *seq, ytp_channel_t channel, ytp_sequence_data_cb_t cb, void *closure, fmc_error_t **error);
```

Unregisters a channel data callback by channel handler

```c
void ytp_sequence_indx_cb_rm(ytp_sequence_t *seq, ytp_channel_t channel, ytp_sequence_data_cb_t cb, void *closure, fmc_error_t **error);
```

Reads one message and executes the callbacks that applies.

```c
bool ytp_sequence_poll(ytp_sequence_t *seq, fmc_error_t **error);
```

Checks if there are not more messages

```c
bool ytp_sequence_term(ytp_sequence_t *seq);
```

Returns the iterator to the end of yamal

```c
ytp_iterator_t ytp_sequence_end(ytp_sequence_t *seq, fmc_error_t **error);
```

Returns the current iterator

```c
ytp_iterator_t ytp_sequence_cur(ytp_sequence_t *seq);
```
