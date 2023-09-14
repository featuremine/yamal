# streams.h

File contains C declaration of streams API.

```c
#include <ytp/streams.h>
```

## ytp_streams_new

Allocates and initializes a ytp_streams_t object. 

- yamal
- error: out-parameter for error handling

**return value**: ytp_streams_t object

```c
ytp_streams_t * ytp_streams_new(ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_streams_del

Destroys and deallocate a ytp_streams_t object. 

- streams
- error: out-parameter for error handling

```c
void ytp_streams_del(ytp_streams_t *streams, fmc_error_t **error)
```

## ytp_streams_announce

Announces a stream in adherence to stream protocol. 

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
ytp_mmnode_offs ytp_streams_announce(ytp_streams_t *streams, size_t psz, const char *peer, size_t csz, const char *channel, size_t esz, const char *encoding, fmc_error_t **error)
```

## ytp_streams_lookup

Lookups a stream in adherence to stream protocol. 

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
ytp_mmnode_offs ytp_streams_lookup(ytp_streams_t *streams, size_t psz, const char *peer, size_t csz, const char *channel, size_t *esz, const char **encoding, fmc_error_t **error)
```

