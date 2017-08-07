#ifndef WEBSOCKET_VIDEO_SCREEN_H
#define WEBSOCKET_VIDEO_SCREEN_H

#include <stdint.h>

#include "libevent.h"
#include "websocket.h"

void websocket_frame_ready_video_screen(
    Websocket           *a_websock,
    struct evbuffer     *a_frame,
    void                *a_ctx
);

#endif

