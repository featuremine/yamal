# yamal.h

File contains C declaration of yamal layer of YTP.

```c
#include <ytp/yamal.h>
```

## ytp_yamal_init

Initializes a ytp_yamal_t object. 
- yamal
- fd: a yamal file descriptor 
- error: out-parameter for error handling

```c
void ytp_yamal_init(ytp_yamal_t *yamal, int fd, fmc_error_t **error)
```

## ytp_yamal_new

Allocates and initializes a ytp_yamal_t object. 
- fd: a yamal file descriptor 
- error: out-parameter for error handling 

- **return value**: ytp_yamal_t object

```c
ytp_yamal_t * ytp_yamal_new(int fd, fmc_error_t **error)
```

## ytp_yamal_set_aux_thread_affinity

Sets CPU affinity for the auxillary yamal thread. 
- cpuid: a CPU ID to use for the affinity

```c
void ytp_yamal_set_aux_thread_affinity(int cpuid)
```

## ytp_yamal_clear_aux_thread_affinity

Clears CPU affinity for the auxillary yamal thread. 


```c
void ytp_yamal_clear_aux_thread_affinity()
```

## ytp_yamal_init_2

Initializes a ytp_yamal_t object. 
- yamal
- fd: a yamal file descriptor 
- enable_thread: enable the preallocation and sync thread 
- error: out-parameter for error handling

```c
void ytp_yamal_init_2(ytp_yamal_t *yamal, int fd, bool enable_thread, fmc_error_t **error)
```

## ytp_yamal_new_2

Allocates and initializes a ytp_yamal_t object. 
- fd: a yamal file descriptor 
- enable_thread: enable the preallocation and sync thread 
- error: out-parameter for error handling 

- **return value**: ytp_yamal_t object

```c
ytp_yamal_t * ytp_yamal_new_2(int fd, bool enable_thread, fmc_error_t **error)
```

## ytp_yamal_init_3

Initializes a ytp_yamal_t object. 
- fd: a yamal file descriptor 
- enable_thread: enable the preallocation and sync thread 
- closable: closable mode 
- error: out-parameter for error handling 

- **return value**: ytp_yamal_t object

```c
void ytp_yamal_init_3(ytp_yamal_t *yamal, int fd, bool enable_thread, YTP_CLOSABLE_MODE closable, fmc_error_t **error)
```

## ytp_yamal_new_3

Allocates and initializes a ytp_yamal_t object. 
- fd: a yamal file descriptor 
- enable_thread: enable the preallocation and sync thread 
- closable: closable mode 
- error: out-parameter for error handling 

- **return value**: ytp_yamal_t object

```c
ytp_yamal_t * ytp_yamal_new_3(int fd, bool enable_thread, YTP_CLOSABLE_MODE closable, fmc_error_t **error)
```

## ytp_yamal_fd

Returns the file descriptor from a ytp_yamal_t object. 
- yamal

- **return value**: fmc_fd

```c
fmc_fd ytp_yamal_fd(ytp_yamal_t *yamal)
```

## ytp_yamal_reserve

Reserves memory for data in the memory mapped list. 
- yamal
- sz: the size of the data payload 
- error: out-parameter for error handling 

- **return value**: a writable pointer for data

```c
char * ytp_yamal_reserve(ytp_yamal_t *yamal, size_t sz, fmc_error_t **error)
```

## ytp_yamal_commit

Commits the data to the memory mapped list. 
- yamal
- data: the value returned by ytp_yamal_reserve if the node is not a sublist. Otherwise the first_ptr returned by ytp_yamal_sublist_commit 
- lstidx: the list index to commit to 
- error: out-parameter for error handling 

- **return value**: ytp_iterator_t for the message

```c
ytp_iterator_t ytp_yamal_commit(ytp_yamal_t *yamal, void *data, size_t lstidx, fmc_error_t **error)
```

## ytp_yamal_sublist_commit

Commits a new data node to an existing sublist (first_ptr, last_ptr) that is not in the main memory mapped list. 
- yamal
- first_ptr: an zero initialized atomic pointer for the first node of the sublist 
- last_ptr: an zero initialized atomic pointer for the last node of the sublist 
- new_ptr: the value returned by ytp_yamal_reserve for the node that is intended to insert 
- lstidx: the list index to commit to 
- error: out-parameter for error handling

```c
void ytp_yamal_sublist_commit(ytp_yamal_t *yamal, void **first_ptr, void **last_ptr, void *new_ptr, fmc_error_t **error)
```

## ytp_yamal_read

Reads a message on yamal level. 
- yamal
- iterator
- seqno
- sz
- data
- error: out-parameter for error handling

```c
void ytp_yamal_read(ytp_yamal_t *yamal, ytp_iterator_t iterator, uint64_t *seqno, size_t *sz, const char **data, fmc_error_t **error)
```

## ytp_yamal_destroy

Destroys a ytp_yamal_t object. 
- yamal
- error: out-parameter for error handling

```c
void ytp_yamal_destroy(ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_yamal_del

Destroys and deallocate a ytp_yamal_t object. 
- yamal
- error: out-parameter for error handling

```c
void ytp_yamal_del(ytp_yamal_t *yamal, fmc_error_t **error)
```

## ytp_yamal_begin

Returns an iterator to the beginning of the list. 
- yamal
- lstidx
- error: out-parameter for error handling 

- **return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_yamal_begin(ytp_yamal_t *yamal, size_t lstidx, fmc_error_t **error)
```

## ytp_yamal_end

Returns an iterator to the end of the list. 
- yamal
- lstidx
- error: out-parameter for error handling 

- **return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_yamal_end(ytp_yamal_t *yamal, size_t lstidx, fmc_error_t **error)
```

## ytp_yamal_term

Checks if there are not more messages. 
- iterator

- **return value**: true if there are not more messages, false otherwise

```c
bool ytp_yamal_term(ytp_iterator_t iterator)
```

## ytp_yamal_next

Returns the next iterator. 
- yamal
- iterator
- error: out-parameter for error handling 

- **return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_yamal_next(ytp_yamal_t *yamal, ytp_iterator_t iterator, fmc_error_t **error)
```

## ytp_yamal_prev

Returns the previous iterator. 
- yamal
- iterator
- error: out-parameter for error handling 

- **return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_yamal_prev(ytp_yamal_t *yamal, ytp_iterator_t iterator, fmc_error_t **error)
```

## ytp_yamal_seek

Returns an iterator given a serializable offset. 
- yamal
- ptr
- error: out-parameter for error handling 

- **return value**: ytp_iterator_t

```c
ytp_iterator_t ytp_yamal_seek(ytp_yamal_t *yamal, ytp_mmnode_offs ptr, fmc_error_t **error)
```

## ytp_yamal_tell

Returns serializable offset given an iterator. 
- yamal
- iterator
- error: out-parameter for error handling 

- **return value**: serializable

```c
ytp_mmnode_offs ytp_yamal_tell(ytp_yamal_t *yamal, ytp_iterator_t iterator, fmc_error_t **error)
```

## ytp_yamal_close

Closes the yamal lists. 
- yamal
- lstidx
- error: out-parameter for error handling

```c
void ytp_yamal_close(ytp_yamal_t *yamal, size_t lstidx, fmc_error_t **error)
```

## ytp_yamal_closed

Determines if a list is closed. 
- yamal
- lstidx
- error: out-parameter for error handling 

- **return value**: true if the list is closed, false otherwise

```c
bool ytp_yamal_closed(ytp_yamal_t *yamal, size_t lstidx, fmc_error_t **error)
```

## ytp_yamal_allocate_page

Allocates a specific page. 
- yamal
- page
- error: out-parameter for error handling

```c
void ytp_yamal_allocate_page(ytp_yamal_t *yamal, size_t page, fmc_error_t **error)
```

## ytp_yamal_reserved_size

Returns the reserved size. 
- yamal
- error: out-parameter for error handling 

- **return value**: yamal size

```c
size_t ytp_yamal_reserved_size(ytp_yamal_t *yamal, fmc_error_t **error)
```

