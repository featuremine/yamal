# subscription.h

File contains C declaration of subscription API.

```c
#include <ytp/subscription.h>
```

## ytp_subscription_write

Writes a raw stream subscription message. 

- yamal
- stream
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_subscription_write(ytp_yamal_t *yamal, ytp_mmnode_offs stream, fmc_error_t **error)
```

## ytp_subscription_commit

Writes and commits a stream subscription message. 

- yamal
- stream
- error: out-parameter for error handling

**return value**: true if the message subscription was successfully committed, false otherwise

```c
bool ytp_subscription_commit(ytp_yamal_t *yamal, ytp_mmnode_offs stream, fmc_error_t **error)
```

## ytp_subscription_read

Reads a raw stream subscription message. 

- yamal
- iterator
- seqno
- stream
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
void ytp_subscription_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno, ytp_mmnode_offs *stream, fmc_error_t **error)
```

## ytp_subscription_lookup

Lookups a raw stream subscription message. 

- yamal
- offset
- seqno
- stream
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
void ytp_subscription_lookup(ytp_yamal_t *yamal, ytp_mmnode_offs offset, uint64_t *seqno, ytp_mmnode_offs *stream, fmc_error_t **error)
```

## ytp_subscription_next

Lookups a raw stream subscription message. 

- yamal
- iterator
- stream
- error: out-parameter for error handling

**return value**: bool true if a message was read, false otherwise

```c
bool ytp_subscription_next(ytp_yamal_t *yamal, ytp_iterator_t *iterator, ytp_mmnode_offs *stream, fmc_error_t **error)
```

## ytp_subscription_begin

Returns an iterator to the beginning of the list. 

- yamal
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_subscription_begin(ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_subscription_end

Returns an iterator to the end of the list. 

- yamal
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_subscription_end(ytp_yamal_t *yamal, fmc_error_t **error)
```

