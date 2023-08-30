# glob.h

File contains C declaration of glob API.

```c
#include <ytp/glob.h>
```

## ytp_glob_new

Allocates and initializes a ytp_glob object. 
- cursor: the ytp_cursor_t object 
- error: out-parameter for error handling 

- **return value**: ytp_glob_t object

```c
ytp_glob_t * ytp_glob_new(ytp_cursor_t *cursor, fmc_error_t **error)
```

## ytp_glob_del

Destroys and deallocate a ytp_glob_t object. 
- glob: the ytp_glob_t object 
- error: out-parameter for error handling

```c
void ytp_glob_del(ytp_glob_t *glob, fmc_error_t **error)
```

## ytp_glob_prefix_cb

Registers a stream data callback by channel name prefix. 
- glob: the ytp_glob_t object 
- sz
- prfx
- cb: the callback pointer 
- closure: the closure pointer 
- error: out-parameter for error handling

```c
void ytp_glob_prefix_cb(ytp_glob_t *glob, size_t sz, const char *prfx, ytp_cursor_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_glob_prefix_cb_rm

Unregisters a stream data callback by channel name prefix. 
- glob: the ytp_glob_t object 
- sz
- prfx
- cb: the callback pointer 
- closure: the closure pointer 
- error: out-parameter for error handling

```c
void ytp_glob_prefix_cb_rm(ytp_glob_t *glob, size_t sz, const char *prfx, ytp_cursor_data_cb_t cb, void *closure, fmc_error_t **error)
```

## ytp_glob_consume

Moves all of the callbacks of the source glob into destination. 
- dest: the destination ytp_glob_t 
- src: the source ytp_glob_t 
- error: out-parameter for error handling

```c
void ytp_glob_consume(ytp_glob_t *dest, ytp_glob_t *src, fmc_error_t **error)
```

