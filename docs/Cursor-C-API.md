# cursor.h

File contains C declaration of data API.

```c
#include <ytp/cursor.h>
```

## ytp_cursor_new

Allocates and initializes a ytp_cursor object. 
- yamal: the ytp_yamal_t object
- error: out-parameter for error handling

**return value**: ytp_cursor_t object

```c
ytp_cursor_t * ytp_cursor_new(ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_cursor_init

Initializes a ytp_cursor object. 
- cursor: the ytp_cursor_t object
- yamal: the ytp_yamal_t object
- error: out-parameter for error handling

**return value**: ytp_cursor_t object

```c
void ytp_cursor_init(ytp_cursor_t *cursor, ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_cursor_del

Destroys and deallocate a ytp_cursor_t object. 
- cursor: the ytp_cursor_t object
- error: out-parameter for error handling

```c
void ytp_cursor_del(ytp_cursor_t *cursor, fmc_error_t **error)
```

## ytp_cursor_destroy

Destroys a ytp_cursor_t object. 
- cursor: the ytp_cursor_t object
- error: out-parameter for error handling

```c
void ytp_cursor_destroy(ytp_cursor_t *cursor, fmc_error_t **error)
```

## ytp_cursor_ann_cb

Registers a stream announcement callback. 
- cursor: the ytp_cursor_t object
- cb: the callback pointer
- closure: the closure pointer
- error: out-parameter for error handling

```c
void ytp_cursor_ann_cb(ytp_cursor_t *cursor, ytp_cursor_ann_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_cursor_ann_cb_rm

Unregisters a stream announcement callback. 
- cursor: the ytp_cursor_t object
- cb: the callback pointer
- closure: the closure pointer
- error: out-parameter for error handling

```c
void ytp_cursor_ann_cb_rm(ytp_cursor_t *cursor, ytp_cursor_ann_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_cursor_data_cb

Registers a stream data callback by stream handler. 
- cursor: the ytp_cursor_t object
- stream: the stream id
- cb: the callback pointer
- closure: the closure pointer
- error: out-parameter for error handling

```c
void ytp_cursor_data_cb(ytp_cursor_t *cursor, ytp_mmnode_offs stream, ytp_cursor_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_cursor_data_cb_rm

Unregisters a stream data callback by stream handler. 
- cursor: the ytp_cursor_t object
- stream: the stream id
- cb: the callback pointer
- closure: the closure pointer
- error: out-parameter for error handling

```c
void ytp_cursor_data_cb_rm(ytp_cursor_t *cursor, ytp_mmnode_offs stream, ytp_cursor_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_cursor_poll

Reads one message and executes the callbacks that applies. 
- cursor: the ytp_cursor_t object
- error: out-parameter for error handling

**return value**: true if a message was processed, false otherwise

```c
bool ytp_cursor_poll(ytp_cursor_t *cursor, fmc_error_t **error)
```

## ytp_cursor_consume

Moves all of the callbacks of the source cursor into destination if both cursors have the same iterator. 
- dest: the destination ytp_cursor_t
- src: the source ytp_cursor_t
- error: out-parameter for error handling

**return value**: true if both cursors have the same iterator and all the callbacks were moved, false otherwise.

```c
bool ytp_cursor_consume(ytp_cursor_t *dest, ytp_cursor_t *src, fmc_error_t **error)
```

## ytp_cursor_all_cb_rm

Removes all of the callbacks of the cursor. 
- cursor: the ytp_cursor_t object

```c
void ytp_cursor_all_cb_rm(ytp_cursor_t *cursor)
```

## ytp_cursor_seek

Moves data pointer to the specified offset. 
- cursor: the ytp_cursor_t object
- offset: from the head of yamal
- error: out-parameter for error handling

```c
void ytp_cursor_seek(ytp_cursor_t *cursor, ytp_mmnode_offs offset, fmc_error_t **error)
```

## ytp_cursor_tell

Returns serializable offset of the current data iterator. 
- cursor: the ytp_cursor_t object
- error: out-parameter for error handling

**return value**: serializable

```c
ytp_mmnode_offs ytp_cursor_tell(ytp_cursor_t *cursor, fmc_error_t **error)
```

