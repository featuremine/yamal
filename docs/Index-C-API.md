# index.h

File contains C declaration of index API.

```c
#include <ytp/index.h>
```

## ytp_index_write

Writes a raw stream index message. 
- yamal
- stream
- offset
- sz: index payload size
- payload: index payload
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_index_write(ytp_yamal_t *yamal, ytp_mmnode_offs stream, ytp_mmnode_offs offset, size_t sz, const void *payload, fmc_error_t **error)
```

## ytp_index_read

Reads a raw stream index message. 
- yamal
- iterator
- seqno
- stream
- data_offset
- sz: index payload size
- payload: index payload
- error: out-parameter for error handling

```c
void ytp_index_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno, ytp_mmnode_offs *stream, ytp_mmnode_offs *data_offset, size_t *sz, const char **payload, fmc_error_t **error)
```

## ytp_index_lookup

Lookups a raw stream index message. 
- yamal
- offset
- seqno
- stream
- data_offset
- sz: index payload size
- payload: index payload
- error: out-parameter for error handling

```c
void ytp_index_lookup(ytp_yamal_t *yamal, ytp_mmnode_offs offset, uint64_t *seqno, ytp_mmnode_offs *stream, ytp_mmnode_offs *data_offset, size_t *sz, const char **payload, fmc_error_t **error)
```

## ytp_index_begin

Returns an iterator to the beginning of the list. 
- yamal
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_index_begin(ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_index_end

Returns an iterator to the end of the list. 
- yamal
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_index_end(ytp_yamal_t *yamal, fmc_error_t **error)
```

