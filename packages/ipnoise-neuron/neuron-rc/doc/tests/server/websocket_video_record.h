#ifndef WEBSOCKET_VIDEO_RECORD_H
#define WEBSOCKET_VIDEO_RECORD_H

#include <stdint.h>

#include "libevent.h"
#include "websocket.h"

void websocket_frame_ready_video_record(
    Websocket           *a_websock,
    struct evbuffer     *a_frame_data,
    void                *a_ctx
);

#endif

