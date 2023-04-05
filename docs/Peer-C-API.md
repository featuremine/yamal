Reserves memory for data in the memory mapped list

```c
char *ytp_peer_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error);
```

Commits the data to the memory mapped list

```c
ytp_iterator_t ytp_peer_commit(ytp_yamal_t *yamal, ytp_peer_t peer, void *data, fmc_error_t **error);
```

Declares an existing/new peer

```c
ytp_iterator_t ytp_peer_name(ytp_yamal_t *yamal, size_t sz, const char *name, fmc_error_t **error);
```

Reads a message on peer level

```c
void ytp_peer_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, ytp_peer_t *peer, size_t *sz, const char **data, fmc_error_t **error);
```

Checks if if peer is announcement peer

```c
bool ytp_peer_ann(ytp_peer_t peer);
```
