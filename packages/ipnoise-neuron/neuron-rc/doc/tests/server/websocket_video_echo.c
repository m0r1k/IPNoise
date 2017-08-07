#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "http_request.h"

#include "websocket_video_echo.h"

void websocket_frame_ready_video_echo(
    Websocket           *a_websock,
    struct evbuffer     *a_frame_data,
    void                *a_ctx)
{
    HttpRequest *req = (HttpRequest *)a_ctx;
    int32_t     fd   = bufferevent_getfd(req->bev);

#ifdef __x86_64__
    PDEBUG(100, "websock frame ready, len: '%lu'\n",
        EVBUFFER_LENGTH(a_frame_data)
    );
#else
    PDEBUG(100, "websock frame ready, len: '%u'\n",
        EVBUFFER_LENGTH(a_frame_data)
    );
#endif

    // send answer
    websocket_send(
        fd,
        FRAME_OPCODE_BIN,
        a_frame_data
    );

    return;
}

