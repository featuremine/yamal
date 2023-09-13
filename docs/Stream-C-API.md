# stream.h

File contains C declaration of stream layer of YTP.

```c
#include <ytp/stream.h>
```

## ytp_stream_close

Closes all stream level lists. 

- yamal
- error: out-parameter for error handling

```c
void ytp_stream_close(ytp_yamal_t *yamal, fmc_error_t **error)
```

