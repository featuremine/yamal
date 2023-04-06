Reserves memory for data in the memory mapped list

```c
char *ytp_time_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error);
```

Commits the data to the memory mapped list

```c
ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, ytp_peer_t peer, ytp_channel_t channel, uint64_t time, void *data, fmc_error_t **error);
```

Reads a message on channel level

```c
void ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, ytp_peer_t *peer, ytp_channel_t *channel, uint64_t *time, size_t *sz, const char **data, fmc_error_t **error);
```
