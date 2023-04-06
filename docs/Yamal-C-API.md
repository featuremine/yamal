Initializes a ytp_yamal_t object

```c
void ytp_yamal_init(ytp_yamal_t *yamal, int fd, fmc_error_t **error);
```

Allocates and initializes a ytp_yamal_t object

```c
ytp_yamal_t *ytp_yamal_new(int fd, fmc_error_t **error);
```

Returns the file descriptor from a ytp_yamal_t object

```c
fmc_fd ytp_yamal_fd(ytp_yamal_t *yamal);
```

Reserves memory for data in the memory mapped list

```c
char *ytp_yamal_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error);
```

Commits the data to the memory mapped list

```c
ytp_iterator_t ytp_yamal_commit(ytp_yamal_t *yamal, void *data, fmc_error_t **error);
```

Reads a message on yamal level

```c
void ytp_yamal_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, size_t *sz, const char **data, fmc_error_t **error);
```

Destroys a ytp_yamal_t object

```c
void ytp_yamal_destroy(ytp_yamal_t *yamal, fmc_error_t **error);
```

Destroys and deallocate a ytp_yamal_t object

```c
void ytp_yamal_del(ytp_yamal_t *yamal, fmc_error_t **error);
```

Returns an iterator to the beginning of the list

```c
ytp_iterator_t ytp_yamal_begin(ytp_yamal_t *yamal, fmc_error_t **error);
```

Returns an iterator to the end of the list

```c
ytp_iterator_t ytp_yamal_end(ytp_yamal_t *yamal, fmc_error_t **error);
```

Checks if there are not more messages

```c
bool ytp_yamal_term(ytp_iterator_t iterator);
```

Returns the next iterator

```c
ytp_iterator_t ytp_yamal_next(ytp_yamal_t *yamal, ytp_iterator_t iterator, fmc_error_t **error);
```

Returns the previous iterator

```c
ytp_iterator_t ytp_yamal_prev(ytp_yamal_t *yamal, ytp_iterator_t iterator, fmc_error_t **error);
```

Removes a node from the list

```c
ytp_iterator_t ytp_yamal_remove(ytp_yamal_t *yamal, ytp_iterator_t iterator, fmc_error_t **error);
```

Returns an iterator given a serializable ptr

```c
ytp_iterator_t ytp_yamal_seek(ytp_yamal_t *yamal, size_t ptr, fmc_error_t **error);
```

Returns serializable ptr given an iterator

```c
size_t ytp_yamal_tell(ytp_yamal_t *yamal, ytp_iterator_t iterator, fmc_error_t **error);
```
