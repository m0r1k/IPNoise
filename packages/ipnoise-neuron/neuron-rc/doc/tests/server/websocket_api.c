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

#include "websocket_api.h"

void process_chunks(
    struct evbuffer *a_in,
    struct evbuffer *a_out)
{
    int32_t     res;
    HttpRequest *req = NULL;

    res = evbuffer_add(
        a_in,
        "",
        1
    );
    if (res){
        PFATAL("evbuffer_add failed\n");
    }

#ifdef __x86_64__
    PDEBUG(100, "received: '%s', len: '%lu'\n",
        (const char *)EVBUFFER_DATA(a_in),
        EVBUFFER_LENGTH(a_in)
    );
#else
    PDEBUG(100, "received: '%s', len: '%u'\n",
        (const char *)EVBUFFER_DATA(a_in),
        EVBUFFER_LENGTH(a_in)
    );
#endif

    // copy back
    evbuffer_add_printf(
        a_out,
        "%s",
        (const char *)EVBUFFER_DATA(a_in)
    );

    // prepare http parsing
    req = _http_request_alloc();

    // parse as generic http request
    http_request_parse_evb(req, a_in);

    PWARN("url: '%s'\n",
        (const char *)EVBUFFER_DATA(req->url)
    );

    // TODO XXX FIXME deliver action here

    _http_request_free(req);
}

void websocket_frame_ready_api(
    Websocket           *a_websock,
    struct evbuffer     *a_frame,
    void                *a_ctx)
{
    HttpRequest     *req        = (HttpRequest *)a_ctx;
    int32_t         fd          = bufferevent_getfd(req->bev);
    struct evbuffer *frame_data = NULL;

#ifdef __x86_64__
    PWARN("websock api frame ready, len: '%lu'\n",
        EVBUFFER_LENGTH(a_frame)
    );
#else
    PWARN("websock api frame ready, len: '%u'\n",
        EVBUFFER_LENGTH(a_frame)
    );
#endif

    frame_data = evbuffer_new();
    if (!frame_data){
        PERROR("evbuffer_new() failed\n");
        goto fail;
    }

    process_chunks(a_frame, frame_data);

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

