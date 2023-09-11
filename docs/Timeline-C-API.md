# timeline.h

File contains C declaration of timeline API of YTP.

```c
#include <ytp/timeline.h>
```

## ytp_timeline_new

Allocates and initializes a ytp_timeline object. 

- timeline
- error: out-parameter for error handling

**return value**: ytp_timeline_t object

```c
ytp_timeline_t * ytp_timeline_new(ytp_control_t *ctrl, fmc_error_t **error)
```

## ytp_timeline_init

Initializes a ytp_timeline object. 

- timeline
- error: out-parameter for error handling

**return value**: ytp_timeline_t object

```c
void ytp_timeline_init(ytp_timeline_t *timeline, ytp_control_t *ctrl, fmc_error_t **error)
```

## ytp_timeline_del

Destroys and deallocate a ytp_timeline_t object. 

- timeline
- error: out-parameter for error handling

```c
void ytp_timeline_del(ytp_timeline_t *timeline, fmc_error_t **error)
```

## ytp_timeline_destroy

Destroys a ytp_timeline_t object. 

- timeline: the ytp_timeline_t object
- error: out-parameter for error handling

```c
void ytp_timeline_destroy(ytp_timeline_t *timeline, fmc_error_t **error)
```

## ytp_timeline_ch_cb

Registers a channel announcement callback. 

Complexity: Linear with the number of registered callbacks.

- timeline
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_ch_cb(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_ch_cb_rm

Unregisters a channel announcement callback. 

Complexity: Linear with the number of registered callbacks.

- timeline
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_ch_cb_rm(ytp_timeline_t *timeline, ytp_timeline_ch_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_peer_cb

Registers a peer announcement callback. 

Complexity: Linear with the number of registered callbacks.

- timeline
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_peer_cb(ytp_timeline_t *timeline, ytp_timeline_peer_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_peer_cb_rm

Unregisters a peer announcement callback. 

Complexity: Linear with the number of registered callbacks.

- timeline
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_peer_cb_rm(ytp_timeline_t *timeline, ytp_timeline_peer_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_prfx_cb

Registers a channel data callback by channel name or prefix. 

Subscribes to data on any channel matching the prefix if chprfx terminates with '/' or exact channel name instead
Complexity: Linear with the number of channels.

- timeline
- sz
- prfx
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_prfx_cb(ytp_timeline_t *timeline, size_t sz, const char *prfx, ytp_timeline_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_prfx_cb_rm

Unregisters a channel data callback by channel name or prefix. 

Complexity: Linear with the number of channels.

- timeline
- sz
- prfx
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_prfx_cb_rm(ytp_timeline_t *timeline, size_t sz, const char *prfx, ytp_timeline_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_indx_cb

Registers a channel data callback by channel handler. 

Complexity: Linear with the number of callbacks on that channel.

- timeline
- channel
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_indx_cb(ytp_timeline_t *timeline, ytp_channel_t channel, ytp_timeline_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_indx_cb_rm

Unregisters a channel data callback by channel handler. 

Complexity: Linear with the number of callbacks on that channel.

- timeline
- channel
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_indx_cb_rm(ytp_timeline_t *timeline, ytp_channel_t channel, ytp_timeline_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_iter_get

Returns the current data iterator. 

- timeline

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_timeline_iter_get(ytp_timeline_t *timeline)
```

## ytp_timeline_iter_set

Sets the current data iterator. 

- timeline
- iterator

```c
void ytp_timeline_iter_set(ytp_timeline_t *timeline, ytp_iterator_t iterator)
```

## ytp_timeline_poll

Reads one message and executes the callbacks that applies. 

- timeline
- error: out-parameter for error handling

**return value**: true if a message was processed, false otherwise

```c
bool ytp_timeline_poll(ytp_timeline_t *timeline, fmc_error_t **error)
```

## ytp_timeline_poll_until

Reads one message and executes the callbacks that applies if timeline is behind src_timeline. 

- timeline
- src_timeline
- error: out-parameter for error handling

**return value**: true if a message was processed, false otherwise

```c
bool ytp_timeline_poll_until(ytp_timeline_t *timeline, const ytp_timeline_t *src_timeline, fmc_error_t **error)
```

## ytp_timeline_consume

Moves all of the callbacks of the source timeline into destination if both timelines have the same iterator. 

- dest
- src

**return value**: true if both timelines have the same iterator and all the callbacks were moved, false otherwise.

```c
bool ytp_timeline_consume(ytp_timeline_t *dest, ytp_timeline_t *src)
```

## ytp_timeline_cb_rm

Removes all of the callbacks of the timeline. 

- timeline

```c
void ytp_timeline_cb_rm(ytp_timeline_t *timeline)
```

## ytp_timeline_seek

Returns an iterator given a serializable offset. 

Moves control pointer to catch up with iterator. 
- timeline
- off
- error: out-parameter for error handling

**return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_timeline_seek(ytp_timeline_t *timeline, ytp_mmnode_offs off, fmc_error_t **error)
```

## ytp_timeline_tell

Returns serializable offset given an iterator. 

- timeline
- iterator
- error: out-parameter for error handling

**return value**: serializable

```c
ytp_mmnode_offs ytp_timeline_tell(ytp_timeline_t *timeline, ytp_iterator_t iterator, fmc_error_t **error)
```

## ytp_timeline_idle_cb

Registers an idle callback. 

- timeline
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_idle_cb(ytp_timeline_t *timeline, ytp_timeline_idle_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_timeline_idle_cb_rm

Unregisters an idle callback. 

- timeline
- cb
- closure
- error: out-parameter for error handling

```c
void ytp_timeline_idle_cb_rm(ytp_timeline_t *timeline, ytp_timeline_idle_cb_t cb, void *closure, fmc_error_t **error)
```

