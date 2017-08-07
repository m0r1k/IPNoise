#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#include "libevent.h"
#include "http_request.h"

typedef enum
{
    ERROR_UNKNOWN                           = 0,
    ERROR_INTERNAL_ERROR,
    ERROR_CHUNK_TYPE_EMPTY,
    ERROR_CHUNK_TYPE_UNSUPPORTED,
} Error;

typedef enum
{
    CHUNK_TYPE_UNKNOWN                      = 0,

    // errors
    CHUNK_TYPE_ERROR                        = 1,

    // commands
    CHUNK_TYPE_ACTION_RESET                 = 100,
    CHUNK_TYPE_ACTION_KEYBOARD_RESET,
    CHUNK_TYPE_ACTION_KEYBOARD_READY,
    CHUNK_TYPE_ACTION_WEBGL,
    CHUNK_TYPE_ACTION_WEBGL_READY,

    // data with variable length
    CHUNK_TYPE_PROP_DATA    = 200,

    // null terminated string
    CHUNK_TYPE_PROP_STRING,

    // uint
    CHUNK_TYPE_PROP_UINT8,
    CHUNK_TYPE_PROP_UINT16,
    CHUNK_TYPE_PROP_UINT32,
    CHUNK_TYPE_PROP_UINT64,

    // int
    CHUNK_TYPE_PROP_INT8,
    CHUNK_TYPE_PROP_INT16,
    CHUNK_TYPE_PROP_INT32,
    CHUNK_TYPE_PROP_INT64
} ChunkType;


void add_chunk_type(
    struct evbuffer     *a_out,
    const ChunkType     a_val
);

void add_chunk_uint16(
    struct evbuffer *a_out,
    const uint16_t  a_val
);

void add_chunk_string(
    struct evbuffer *a_out,
    const char      *a_val
);

void add_chunk_data_from_event(
    struct evbuffer *a_out,
    struct evbuffer *a_data
);

void add_chunk_error(
    struct evbuffer     *a_out,
    const Error         a_err_code,
    const char          *a_err_msg,
    struct evbuffer     *a_frame_orig
);

#endif

