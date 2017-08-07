#ifndef FILE_H
#define FILE_H

#include <stdint.h>

#include <ipnoise-common/log_common.h>
#include "libevent.h"

int64_t file_load(
    const char      *a_path,
    struct evbuffer *a_out
);

int32_t file_load_as_base64(
    const char      *a_path,
    struct evbuffer *a_out
);

#endif

