# time.h

File contains C declaration of time layer of YTP.

```c
#include <ytp/time.h>
```

## ytp_time_reserve

Reserves memory for data in the memory mapped list. 
- yamal: the ytp_yamal_t object
- sz: the size of the data payload
- error: out-parameter for error handling

**return value**: a writable pointer for data

```c
char * ytp_time_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error)
```

## ytp_time_commit

Commits the data to the memory mapped list on the time level. 
- yamal: the ytp_yamal_t object
- ts: the time to publish the message
- data: the value returned by ytp_peer_reserve if the node is not a sublist. Otherwise the first_ptr returned by ytp_peer_sublist_commit
- lstidx: the list index to commit to
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_time_commit(ytp_yamal_t *yamal, int64_t ts, void *data, size_t listidx, fmc_error_t **error)
```

## ytp_time_sublist_commit

Commits a new data node to an existing sublist (first_ptr, last_ptr) that is not in the main memory mapped list. 
- yamal
- peer: the peer that publishes the data
- channel: the channel to publish the data
- ts: the time to publish the message
- first_ptr: an zero initialized atomic pointer for the first node of the sublist
- last_ptr: an zero initialized atomic pointer for the last node of the sublist
- new_ptr: the value returned by ytp_peer_reserve for the node that is intended to insert
- error: out-parameter for error handling

```c
void ytp_time_sublist_commit(ytp_yamal_t *yamal, int64_t ts, void **first_ptr, void **last_ptr, void *new_ptr, fmc_error_t **error)
```

## ytp_time_read

Reads a message on channel level. 
- yamal: the ytp_yamal_t object
- iterator
- seqno
- ts
- sz
- data
- error: out-parameter for error handling

```c
void ytp_time_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno, int64_t *ts, size_t *sz, const char **data, fmc_error_t **error)
```

