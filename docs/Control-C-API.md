# control.h

File contains C declaration of control layer of YTP.
A control channel is used for communicating peer, channel, publisher and subscription control information. A publisher is a peer that may publish messages on a channel.

```c
#include <ytp/control.h>
```

## ytp_control_new

Allocates and initializes a ytp_control_t object. 

- fd: a yamal file descriptor
- error: out-parameter for error handling

**return value**: ytp_control_t object

```c
ytp_control_t * ytp_control_new(fmc_fd fd, fmc_error_t **error)
```

## ytp_control_init

Initializes a ytp_control_t object. 

- ctrl: the ytp_control_t object
- fd: a yamal file descriptor
- error: out-parameter for error handling

```c
void ytp_control_init(ytp_control_t *ctrl, fmc_fd fd, fmc_error_t **error)
```

## ytp_control_new_2

Allocates and initializes a ytp_control_t object. 

- fd: a yamal file descriptor
- enable_thread: enables the auxiliary thread
- error: out-parameter for error handling

**return value**: ytp_control_t object

```c
ytp_control_t * ytp_control_new_2(fmc_fd fd, bool enable_thread, fmc_error_t **error)
```

## ytp_control_init_2

Initializes a ytp_control_t object. 

- ctrl: the ytp_control_t object
- fd: a yamal file descriptor
- enable_thread: enables the auxiliary thread
- error: out-parameter for error handling

```c
void ytp_control_init_2(ytp_control_t *ctrl, fmc_fd fd, bool enable_thread, fmc_error_t **error)
```

## ytp_control_del

Destroys and deallocate a ytp_control_t object. 

- ctrl: the ytp_control_t object
- error: out-parameter for error handling

```c
void ytp_control_del(ytp_control_t *ctrl, fmc_error_t **error)
```

## ytp_control_destroy

Destroys a ytp_control_t object. 

- ctrl: the ytp_control_t object
- error: out-parameter for error handling

```c
void ytp_control_destroy(ytp_control_t *ctrl, fmc_error_t **error)
```

## ytp_control_reserve

Reserves memory for data in the memory mapped list to be used to write to the file. 

- ctrl: the ytp_control_t object
- sz: size of the buffer to hold the memory
- error: out-parameter for error handling

**return value**: buffer to hold the reserved memory

```c
char * ytp_control_reserve(ytp_control_t *ctrl, size_t sz, fmc_error_t **error)
```

## ytp_control_commit

Commits the data to the memory mapped list on the control level. 

- ctrl: the ytp_control_t object
- peer: the peer that publishes the data
- channel: the channel to publish the data
- ts: the time to publish the data
- data: the value returned by ytp_peer_reserve if the node is not a sublist. Otherwise the first_ptr returned by ytp_peer_sublist_commit
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_control_commit(ytp_control_t *ctrl, ytp_peer_t peer, ytp_channel_t channel, int64_t ts, void *data, fmc_error_t **error)
```

## ytp_control_sublist_commit

Commits a new data node to an existing sublist (first_ptr, last_ptr) that is not in the main memory mapped list. 

- ctrl: the ytp_control_t object
- peer: the peer that publishes the data
- channel: the channel to publish the data
- ts: the time to publish the data
- first_ptr: an zero initialized atomic pointer for the first node of the sublist
- last_ptr: an zero initialized atomic pointer for the last node of the sublist
- new_ptr: the value returned by ytp_peer_reserve for the node that is intended to insert
- error: out-parameter for error handling

```c
void ytp_control_sublist_commit(ytp_control_t *ctrl, ytp_peer_t peer, ytp_channel_t channel, int64_t ts, void **first_ptr, void **last_ptr, void *new_ptr, fmc_error_t **error)
```

## ytp_control_sublist_finalize

Commits a sublist to the memory mapped list on the control level. 

- ctrl: the ytp_control_t object
- first_ptr: the first node of the sublist
- error: out-parameter for error handling

**return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_control_sublist_finalize(ytp_control_t *ctrl, void *first_ptr, fmc_error_t **error)
```

## ytp_control_sub

Publishes a subscription message. 

Complexity: Constant on average, worst case linear in the size of the list.

- ctrl: the ytp_control_t object
- peer: the peer that publishes the subscription message
- ts: the time to publish the subscription message
- sz: size of the payload
- payload: a prefix or channel name
- error: out-parameter for error handling

```c
void ytp_control_sub(ytp_control_t *ctrl, ytp_peer_t peer, int64_t ts, size_t sz, const char *payload, fmc_error_t **error)
```

## ytp_control_dir

Publishes a directory message. 

Complexity: Constant on average, worst case linear in the size of the list.

- ctrl: the ytp_control_t object
- peer: the peer that publishes the directory message
- ts: the time to publish the directory message
- sz: size of the payload
- payload: a SCDP encoded string
- error: out-parameter for error handling

```c
void ytp_control_dir(ytp_control_t *ctrl, ytp_peer_t peer, int64_t ts, size_t sz, const char *payload, fmc_error_t **error)
```

## ytp_control_ch_name

Returns the name of the channel, given the channel reference. 

Complexity: Constant on average, worst case linear in the number of channels.

- ctrl: the ytp_control_t object
- channel: channel reference to obtain the name
- sz: size of the channel name
- name: name of the channel
- error: out-parameter for error handling

```c
void ytp_control_ch_name(ytp_control_t *ctrl, ytp_channel_t channel, size_t *sz, const char **name, fmc_error_t **error)
```

## ytp_control_ch_decl

Declares an existing/new channel. 

Complexity: Constant on average, worst case linear in the size of the list.

- ctrl: the ytp_control_t object
- peer: the peer that publishes the channel announcement
- ts: the time to publish the channel announcement
- sz: size of the channel name
- name: name of the channel
- error: out-parameter for error handling

**return value**: channel id

```c
ytp_channel_t ytp_control_ch_decl(ytp_control_t *ctrl, ytp_peer_t peer, int64_t ts, size_t sz, const char *name, fmc_error_t **error)
```

## ytp_control_peer_name

Returns the name of the peer, given the peer reference. 

Complexity: Constant on average, worst case linear in the number of peers.

- ctrl: the ytp_control_t object
- peer: peer reference to obtain the name
- sz: size of the peer name
- name: name of the peer
- error: out-parameter for error handling

```c
void ytp_control_peer_name(ytp_control_t *ctrl, ytp_peer_t peer, size_t *sz, const char **name, fmc_error_t **error)
```

## ytp_control_peer_decl

Declares an existing/new peer. 

Complexity: Constant on average, worst case linear in the size of the list.

- ctrl: the ytp_control_t object
- sz: size of the peer name
- name: name of the peer
- error: out-parameter for error handling

**return value**: peer id

```c
ytp_peer_t ytp_control_peer_decl(ytp_control_t *ctrl, size_t sz, const char *name, fmc_error_t **error)
```

## ytp_control_poll_until

Process announcement messages until the specified seqno. 

- ctrl: the ytp_control_t object
- seqno: seqno
- error: out-parameter for error handling

```c
void ytp_control_poll_until(ytp_control_t *ctrl, uint64_t seqno, fmc_error_t **error)
```

## ytp_control_end

Returns the iterator to the end of the list, the last node. Also moves control pointer to the end. 

- ctrl: the ytp_control_t object
- error: out-parameter for error handling

**return value**: iterator to the end of the list

```c
ytp_iterator_t ytp_control_end(ytp_control_t *ctrl, fmc_error_t **error)
```

## ytp_control_term

Checks if there are not more messages. 

- iterator: iterator to test

**return value**: true if there are not more messages, false otherwise

```c
bool ytp_control_term(ytp_iterator_t iterator)
```

## ytp_control_seek

Returns an iterator given a serializable ptr. Also moves control pointer to catch up with iterator. 

- ctrl: ytp_control_t object
- off: the serializable ptr offset
- error: out-parameter for error handling

**return value**: the iterator of the serializable ptr

```c
ytp_iterator_t ytp_control_seek(ytp_control_t *ctrl, ytp_mmnode_offs off, fmc_error_t **error)
```

## ytp_control_tell

Returns serializable offset given an iterator. 

- ctrl: ytp_control_t object
- iterator: the iterator of the serializable ptr
- error: out-parameter for error handling

**return value**: the serializable ptr offset

```c
ytp_mmnode_offs ytp_control_tell(ytp_control_t *ctrl, ytp_iterator_t iterator, fmc_error_t **error)
```

