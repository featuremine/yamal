# sequence.h

File contains C declaration of sequence API of YTP.

```c
#include <ytp/sequence.h>
```

## ytp_sequence_new

Allocates and initializes a ytp_sequence_t object. 

- fd: a yamal file descriptor
- error: out-parameter for error handling

**return value**: ytp_sequence_t object

```c
ytp_sequence_t * ytp_sequence_new(fmc_fd fd, fmc_error_t **error)
```

## ytp_sequence_init

Initializes a ytp_sequence_t object. 

- seq: the ytp_sequence_t object
- fd: the yamal file descriptor
- error: out-parameter for error handling

```c
void ytp_sequence_init(ytp_sequence_t *seq, fmc_fd fd, fmc_error_t **error)
```

## ytp_sequence_new_2

Allocates and initializes a ytp_sequence_t object. 

- fd: a yamal file descriptor
- enable_thread: enables the auxiliary thread
- error: out-parameter for error handling

**return value**: ytp_sequence_t object

```c
ytp_sequence_t * ytp_sequence_new_2(fmc_fd fd, bool enable_thread, fmc_error_t **error)
```

## ytp_sequence_init_2

Initializes a ytp_sequence_t object. 

- seq: the ytp_sequence_t object
- fd: the yamal file descriptor
- enable_thread: enables the auxiliary thread
- error: out-parameter for error handling

```c
void ytp_sequence_init_2(ytp_sequence_t *seq, fmc_fd fd, bool enable_thread, fmc_error_t **error)
```

## ytp_sequence_del

Destroys and deallocate a ytp_sequence_t object. 

- seq: the ytp_sequence_t object
- error: out-parameter for error handling

```c
void ytp_sequence_del(ytp_sequence_t *seq, fmc_error_t **error)
```

## ytp_sequence_destroy

Destroys a ytp_sequence_t object. 

- seq: the ytp_sequence_t object
- error: out-parameter for error handling

```c
void ytp_sequence_destroy(ytp_sequence_t *seq, fmc_error_t **error)
```

## ytp_sequence_reserve

Reserves memory for data in the memory mapped list. 

- seq: the ytp_sequence_t object
- sz: the size of the data payload
- error: out-parameter for error handling

**return value**: buffer to hold the reserved memory

```c
char * ytp_sequence_reserve(ytp_sequence_t *seq, size_t sz, fmc_error_t **error)
```

## ytp_sequence_commit

Commits the data to the memory mapped list on the sequence level. 

- seq: the ytp_sequence_t object
- peer: the peer that publishes the data
- channel: the channel to publish the data
- msgtime: the time to publish the message
- data: the value returned by ytp_peer_reserve if the node is not a sublist. Otherwise the first_ptr returned by ytp_peer_sublist_commit
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_sequence_commit(ytp_sequence_t *seq, ytp_peer_t peer, ytp_channel_t channel, int64_t ts, void *data, fmc_error_t **error)
```

## ytp_sequence_sublist_commit

Commits a new data node to an existing sublist (first_ptr, last_ptr) that is not in the main memory mapped list. 

- seq
- peer: the peer that publishes the data
- channel: the channel to publish the data
- ts
- first_ptr: an zero initialized atomic pointer for the first node of the sublist
- last_ptr: an zero initialized atomic pointer for the last node of the sublist
- new_ptr: the value returned by ytp_peer_reserve for the node that is intended to insert
- error: out-parameter for error handling

```c
void ytp_sequence_sublist_commit(ytp_sequence_t *seq, ytp_peer_t peer, ytp_channel_t channel, int64_t ts, void **first_ptr, void **last_ptr, void *new_ptr, fmc_error_t **error)
```

## ytp_sequence_sublist_finalize

Commits the sublist to the memory mapped list on the sequence level. 

- seq
- first_ptr: the first node of the sublist
- error: out-parameter for error handling

```c
ytp_iterator_t ytp_sequence_sublist_finalize(ytp_sequence_t *seq, void *first_ptr, fmc_error_t **error)
```

## ytp_sequence_sub

Publishes a subscription message. 

Publishes a subscription message if it is not already published.
Complexity: Constant on average, worst case linear in the size of the list.

- seq
- peer: the peer that publishes the subscription
- ts
- sz
- payload
- error

```c
void ytp_sequence_sub(ytp_sequence_t *seq, ytp_peer_t peer, int64_t ts, size_t sz, const char *payload, fmc_error_t **error)
```

## ytp_sequence_dir

Publishes a directory message. 

Publishes a directory message if it is not already published.
Complexity: Constant on average, worst case linear in the size of the list.

- seq
- peer: the peer that publishes the message
- ts
- sz
- payload
- error

```c
void ytp_sequence_dir(ytp_sequence_t *seq, ytp_peer_t peer, int64_t ts, size_t sz, const char *payload, fmc_error_t **error)
```

## ytp_sequence_ch_name

Returns the name of the channel, given the channel reference. 

Complexity: Constant on average, worst case linear in the number of channels.

- seq: the ytp_sequence_t object
- channel: channel reference to obtain the name
- sz: size of the channel name
- name: name of the channel
- error: out-parameter for error handling

```c
void ytp_sequence_ch_name(ytp_sequence_t *seq, ytp_channel_t channel, size_t *sz, const char **name, fmc_error_t **error)
```

## ytp_sequence_ch_decl

Declares an existing/new channel. 

Complexity: Constant on average, worst case linear in the size of the list.

- seq: the ytp_sequence_t object
- peer: the peer that publishes the channel announcement
- msgtime: the time to publish the channel announcement
- sz: size of the channel name
- name: name of the channel
- error: out-parameter for error handling

**return value**: channel reference

```c
ytp_channel_t ytp_sequence_ch_decl(ytp_sequence_t *seq, ytp_peer_t peer, int64_t ts, size_t sz, const char *name, fmc_error_t **error)
```

## ytp_sequence_ch_cb

Registers a channel announcement callback. 

Complexity: Linear with the number of registered callbacks.

- seq: the ytp_sequence_t object
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_sequence_ch_cb(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_sequence_ch_cb_rm

Unregisters a channel announcement callback. 

Complexity: Linear with the number of registered callbacks.

- seq: the ytp_sequence_t object
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_sequence_ch_cb_rm(ytp_sequence_t *seq, ytp_sequence_ch_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_sequence_peer_name

Returns the name of the peer, given the peer reference. 

Complexity: Constant on average, worst case linear in the number of peers.

- seq: the ytp_sequence_t object
- peer: peer reference to obtain the name
- sz: size of the peer name
- name: name of the peer
- error: out-parameter for error handling

```c
void ytp_sequence_peer_name(ytp_sequence_t *seq, ytp_peer_t peer, size_t *sz, const char **name, fmc_error_t **error)
```

## ytp_sequence_peer_decl

Declares an existing/new peer. 

Complexity: Constant on average, worst case linear in the size of the list.

- seq: the ytp_sequence_t object
- sz: size of the peer name
- name: name of the peer
- error: out-parameter for error handling

**return value**: peer reference

```c
ytp_peer_t ytp_sequence_peer_decl(ytp_sequence_t *seq, size_t sz, const char *name, fmc_error_t **error)
```

## ytp_sequence_peer_cb

Registers a peer announcement callback. 

Complexity: Linear with the number of registered callbacks.

- seq: the ytp_sequence_t object
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_sequence_peer_cb(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_sequence_peer_cb_rm

Unregisters a peer announcement callback. 

Complexity: Linear with the number of registered callbacks.

- seq: the ytp_sequence_t object
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_sequence_peer_cb_rm(ytp_sequence_t *seq, ytp_sequence_peer_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_sequence_prfx_cb

Registers a channel data callback by channel name or prefix. 

Subscribes to data on any channel matching the prefix if chprfx terminates with '/' or exact channel name instead
Complexity: Linear with the number of channels.

- seq: the ytp_sequence_t object
- sz
- prfx
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_sequence_prfx_cb(ytp_sequence_t *seq, size_t sz, const char *prfx, ytp_sequence_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_sequence_prfx_cb_rm

Unregisters a channel data callback by channel name or prefix. 

Complexity: Linear with the number of channels.

- seq: the ytp_sequence_t object
- sz
- prfx
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_sequence_prfx_cb_rm(ytp_sequence_t *seq, size_t sz, const char *prfx, ytp_sequence_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_sequence_indx_cb

Registers a channel data callback by channel handler. 

Complexity: Linear with the number of callbacks on that channel.

- seq: the ytp_sequence_t object
- channel
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_sequence_indx_cb(ytp_sequence_t *seq, ytp_channel_t channel, ytp_sequence_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_sequence_indx_cb_rm

Unregisters a channel data callback by channel handler. 

Complexity: Linear with the number of callbacks on that channel.

- seq: the ytp_sequence_t object
- channel
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_sequence_indx_cb_rm(ytp_sequence_t *seq, ytp_channel_t channel, ytp_sequence_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_sequence_poll

Reads one message and executes the callbacks that applies. 

- seq: the ytp_sequence_t object
- error: out-parameter for error handling

**return value**: true if a message was processed, false otherwise

```c
bool ytp_sequence_poll(ytp_sequence_t *seq, fmc_error_t **error)
```

## ytp_sequence_end

Returns the iterator to the end of yamal. 

Complexity: Constant.

- seq: the ytp_sequence_t object
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_sequence_end(ytp_sequence_t *seq, fmc_error_t **error)
```

## ytp_sequence_cur

Returns the current data iterator. 

Complexity: Constant.

- seq: the ytp_sequence_t object

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_sequence_cur(ytp_sequence_t *seq)
```

## ytp_sequence_get_it

Returns the current data iterator. 

- seq: the ytp_sequence_t object

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_sequence_get_it(ytp_sequence_t *seq)
```

## ytp_sequence_set_it

Sets the current data iterator. 

- seq: the ytp_sequence_t object
- iterator

```c
void ytp_sequence_set_it(ytp_sequence_t *seq, ytp_iterator_t iterator)
```

## ytp_sequence_cb_rm

Removes all of the callbacks of the sequence. 

- sequence

```c
void ytp_sequence_cb_rm(ytp_sequence_t *seq)
```

## ytp_sequence_seek

Returns an iterator given a serializable offset. 

- seq: the ytp_sequence_t object
- offset: from the head of yamal
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_sequence_seek(ytp_sequence_t *seq, ytp_mmnode_offs offset, fmc_error_t **error)
```

## ytp_sequence_tell

Returns serializable offset given an iterator. 

- seq: the ytp_sequence_t object
- iterator
- error: out-parameter for error handling

**return value**: offset from the head of yamal

```c
ytp_mmnode_offs ytp_sequence_tell(ytp_sequence_t *seq, ytp_iterator_t iterator, fmc_error_t **error)
```

## ytp_sequence_shared_new

Allocates and initializes a ytp_sequence_shared object with a reference counter equal to one. 

- filename: a yamal file path
- error: out-parameter for error handling

**return value**: ytp_sequence_shared_t object

```c
ytp_sequence_shared_t * ytp_sequence_shared_new(const char *filename, fmc_fmode mode, fmc_error_t **error)
```

## ytp_sequence_shared_inc

Increases the reference counter. 

- shared_seq

```c
void ytp_sequence_shared_inc(ytp_sequence_shared_t *shared_seq)
```

## ytp_sequence_shared_dec

Decreases the reference counter and call ytp_sequence_del the sequence if the reference counter is zero. 

- shared_seq
- error: out-parameter for error handling

```c
void ytp_sequence_shared_dec(ytp_sequence_shared_t *shared_seq, fmc_error_t **error)
```

## ytp_sequence_shared_get

Returns the ytp_sequence_t object. 

- shared_seq

**return value**: ytp_sequence_t object

```c
ytp_sequence_t * ytp_sequence_shared_get(ytp_sequence_shared_t *shared_seq)
```

