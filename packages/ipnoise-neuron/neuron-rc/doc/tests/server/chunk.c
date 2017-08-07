#include "chunk.h"

void add_chunk_type(
    struct evbuffer     *a_out,
    const ChunkType     a_val)
{
    uint16_t type = a_val;

    evbuffer_add(a_out, &type, sizeof(type));
}

void add_chunk_uint16(
    struct evbuffer *a_out,
    const uint16_t  a_val)
{
    uint16_t type = CHUNK_TYPE_PROP_UINT16;

    evbuffer_add(a_out, &type,  sizeof(type));
    evbuffer_add(a_out, &a_val, sizeof(a_val));
}

void add_chunk_string(
    struct evbuffer *a_out,
    const char      *a_val)
{
    uint16_t type = CHUNK_TYPE_PROP_STRING;

    evbuffer_add(a_out, &type,  sizeof(type));
    evbuffer_add_printf(a_out, "%s%c", a_val, 0);
}

void add_chunk_data_from_event(
    struct evbuffer *a_out,
    struct evbuffer *a_data)
{
    uint16_t type = CHUNK_TYPE_PROP_DATA;

    evbuffer_add(a_out, &type,  sizeof(type));
    evbuffer_add_buffer(a_out, a_data);
}

void add_chunk_error(
    struct evbuffer     *a_out,
    const Error         a_err_code,
    const char          *a_err_msg,
    struct evbuffer     *a_frame_orig)
{
    add_chunk_type(a_out,   CHUNK_TYPE_ERROR);
    add_chunk_uint16(a_out, a_err_code);
    add_chunk_string(a_out, a_err_msg);
    add_chunk_data_from_event(a_out, a_frame_orig);
}

