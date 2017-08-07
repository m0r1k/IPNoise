#ifndef WEBSOCKET_API_H
#define WEBSOCKET_API_H

#include <stdint.h>

#include "libevent.h"
#include "websocket.h"
#include "chunk.h"

void process_chunks(
    struct evbuffer *a_in,
    struct evbuffer *a_out
);

void websocket_frame_ready_api(
    Websocket           *a_websock,
    struct evbuffer     *a_frame_data,
    void                *a_ctx
);

#endif

