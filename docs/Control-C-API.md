Allocates and initializes a ytp_control_t object

```c
ytp_control_t *ytp_control_new(fmc_fd fd, fmc_error_t **error);
```

Destroys and deallocate a ytp_control_t object

```c
void ytp_control_del(ytp_control_t *ctrl, fmc_error_t **error);
```

Initializes a ytp_control_t object

```c
void ytp_control_init(ytp_control_t *ctrl, fmc_fd fd, fmc_error_t **error);
```

Destroys a ytp_control_t object

```c
void ytp_control_destroy(ytp_control_t *ctrl, fmc_error_t **error);
```

Reserves memory for data in the memory mapped list

```c
char *ytp_control_reserve(ytp_control_t *ctrl, size_t sz, fmc_error_t **error);
```

Commits the data to the memory mapped list

```c
ytp_iterator_t ytp_control_commit(ytp_control_t *ctrl, ytp_peer_t peer, ytp_channel_t channel, uint64_t time, void *data, fmc_error_t **error);
```

Publishes a subscription message

```c
void ytp_control_sub(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time, size_t sz, const char *payload, fmc_error_t **error);
```

Publishes a directory message

```c
void ytp_control_dir(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time, size_t sz, const char *payload, fmc_error_t **error);
```

Returns the name of the channel, given the channel reference

```c
void ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel, size_t *sz, const char **name, fmc_error_t **error);
```

Declares an existing/new channel

```c
ytp_channel_t ytp_control_ch_decl(ytp_control_t *ctrl, ytp_peer_t peer, uint64_t time, size_t sz, const char *name, fmc_error_t **error);
```

Returns the name of the peer, given the peer reference

```c
void ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer, size_t *sz, const char **name, fmc_error_t **error);
```

Declares an existing/new peer

```c
ytp_peer_t ytp_control_peer_decl(ytp_control_t *ctrl, size_t sz, const char *name, fmc_error_t **error);
```

Checks if there are not more messages

```c
bool ytp_control_term(ytp_control_t *ctrl);
```

Reads a message on control level

```c
bool ytp_control_read(ytp_control_t *ctrl, ytp_peer_t *peer, ytp_channel_t *channel, uint64_t *time, size_t *sz, const char **data, fmc_error_t **error);
```

Returns the iterator to the end of yamal

```c
ytp_iterator_t ytp_control_end(ytp_control_t *ctrl, fmc_error_t **error);
```

Returns the current iterator

```c
ytp_iterator_t ytp_control_cur(ytp_control_t *ctrl);
```
