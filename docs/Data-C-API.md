# data.h

File contains C declaration of data API.

```c
#include <ytp/data.h>
```

## ytp_data_reserve

Reserves memory for data in the memory mapped list. 
- yamal
- sz: the size of the data payload
- error: out-parameter for error handling

**return value**: a writable pointer for data

```c
char * ytp_data_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error)
```

## ytp_data_commit

Commits the data to the memory mapped list. 
- yamal
- ts
- stream
- data: the value returned by ytp_data_reserve
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_data_commit(ytp_yamal_t *yamal, int64_t ts, ytp_mmnode_offs stream, void *data, fmc_error_t **error)
```

## ytp_data_sublist_commit

Commits a new data node to an existing sublist (first_ptr, last_ptr) that is not in the main memory mapped list. 
- yamal
- ts
- stream
- first_ptr: an zero initialized atomic pointer for the first node of the sublist
- last_ptr: an zero initialized atomic pointer for the last node of the sublist
- data: the value returned by ytp_data_reserve for the node that is intended to insert
- error: out-parameter for error handling

```c
void ytp_data_sublist_commit(ytp_yamal_t *yamal, int64_t ts, ytp_mmnode_offs stream, void **first_ptr, void **last_ptr, void *data, fmc_error_t **error)
```

## ytp_data_sublist_finalize

Commits a data sublist to the memory mapped list. 
- yamal
- first_ptr: the first node of the sublist
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the first message

```c
ytp_iterator_t ytp_data_sublist_finalize(ytp_yamal_t *yamal, void *first_ptr, fmc_error_t **error)
```

## ytp_data_read

Reads a message on data level. 
- yamal
- iterator
- seqno
- ts
- stream
- sz
- data
- error: out-parameter for error handling

```c
void ytp_data_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno, int64_t *ts, ytp_mmnode_offs *stream, size_t *sz, const char **data, fmc_error_t **error)
```

## ytp_data_begin

Returns an iterator to the beginning of the list. 
- yamal
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_data_begin(ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_data_end

Returns an iterator to the end of the list. 
- yamal
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_data_end(ytp_yamal_t *yamal, fmc_error_t **error)
```

