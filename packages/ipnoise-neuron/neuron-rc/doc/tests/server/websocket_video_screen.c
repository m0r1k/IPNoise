#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "snapshot.h"
#include "http_request.h"

#include "websocket_video_screen.h"

void websocket_frame_ready_video_screen(
    Websocket           *a_websock,
    struct evbuffer     *a_frame,
    void                *a_ctx)
{
    HttpRequest     *req        = (HttpRequest *)a_ctx;
    int32_t         res         = 0;
    int32_t         fd          = bufferevent_getfd(req->bev);
    struct evbuffer *frame_data  = evbuffer_new();

    if (!frame_data){
        PERROR("evbuffer_new() failed\n");
        goto fail;
    }

#ifdef __x86_64__
    PDEBUG(100, "websock frame ready, len: '%lu'\n",
        EVBUFFER_LENGTH(a_frame)
    );
#else
    PDEBUG(100, "websock frame ready, len: '%u'\n",
        EVBUFFER_LENGTH(a_frame)
    );
#endif

    res = create_snapshot(frame_data);
    if (res){
        PERROR("create_snapshot failed\n");
        goto fail;
    }

    // send answer
    websocket_send(
        fd,
        FRAME_OPCODE_BIN,
        frame_data
    );

out:
    if (frame_data){
        evbuffer_free(frame_data);
        frame_data = NULL;
    }
    return;
fail:
    goto out;
}

