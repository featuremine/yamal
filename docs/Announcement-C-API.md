# announcement.h

File contains C declaration of announcement API.

```c
#include <ytp/announcement.h>
```

## ytp_announcement_write

Writes a raw stream announcement message. 
- yamal
- psz: peer size
- peer: peer name
- csz: channel size
- channel: channel name
- esz: encoding size
- encoding: encoding metadata
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_announcement_write(ytp_yamal_t *yamal, size_t psz, const char *peer, size_t csz, const char *channel, size_t esz, const char *encoding, fmc_error_t **error)
```

## ytp_announcement_read

Read an announcement message. 
- yamal
- iterator
- seqno
- psz: peer size
- peer: peer name
- csz: channel size
- channel: channel name
- esz: encoding size
- encoding: encoding metadata
- original: offset of the original announcement, zero if uninitialized
- subscribed: offset of the first subscribe message
- error: out-parameter for error handling

```c
void ytp_announcement_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno, size_t *psz, const char **peer, size_t *csz, const char **channel, size_t *esz, const char **encoding, ytp_mmnode_offs **original, ytp_mmnode_offs **subscribed, fmc_error_t **error)
```

## ytp_announcement_lookup

Look up an announcement message. 
- yamal
- stream
- seqno
- psz: peer size
- peer: peer name
- csz: channel size
- channel: channel name
- esz: encoding size
- encoding: encoding metadata
- original: offset of the original announcement, zero if uninitialized
- subscribed: offset of the first subscribe message
- error: out-parameter for error handling

```c
void ytp_announcement_lookup(ytp_yamal_t *yamal, ytp_mmnode_offs stream, uint64_t *seqno, size_t *psz, const char **peer, size_t *csz, const char **channel, size_t *esz, const char **encoding, ytp_mmnode_offs **original, ytp_mmnode_offs **subscribed, fmc_error_t **error)
```

## ytp_announcement_begin

Returns an iterator to the beginning of the list. 
- yamal
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_announcement_begin(ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_announcement_next

Returns iterator for the next announcement message. 
- yamal
- iterator
- seqno
- psz: peer size
- peer: peer name
- csz: channel size
- channel: channel name
- esz: encoding size
- encoding: encoding metadata
- original: offset of the original announcement, zero if uninitialized
- subscribed: offset of the first subscribe message
- error: out-parameter for error handling

**return value**: true if an announcement has been read, false otherwise

```c
bool ytp_announcement_next(ytp_yamal_t *yamal, ytp_iterator_t *iterator, uint64_t *seqno, ytp_mmnode_offs *stream, size_t *psz, const char **peer, size_t *csz, const char **channel, size_t *esz, const char **encoding, ytp_mmnode_offs **original, ytp_mmnode_offs **subscribed, fmc_error_t **error)
```

