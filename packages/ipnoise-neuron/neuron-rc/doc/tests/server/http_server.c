#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <openssl/sha.h>
#include <ipnoise-common/log_common.h>

#include "http_server.h"
#include "base64.h"
#include "file.h"

#include "http_request.h"
#include "http_answer.h"

#include "websocket_video_echo.h"
#include "websocket_video_record.h"
#include "websocket_video_play.h"
#include "websocket_video_screen.h"
#include "websocket_api.h"

void http_server_read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    int32_t         res, fd     = -1;
    HttpServer      *server     = NULL;
    HttpRequest     *req        = NULL;
    HttpAnswer      *answer     = NULL;
    int32_t         send_answer = 1;
    int32_t         close_conn  = 1;

    fd      = bufferevent_getfd(a_bev);
    server  = (HttpServer *)a_ctx;
    req     = http_request_get(fd);

    if (req->websock){
        if (!strcmp(
            (const char *)EVBUFFER_DATA(req->url),
            "/video/record"))
        {
            close_conn = websocket_parse(
                req->websock,
                req->bev,
                websocket_frame_ready_video_record,
                (void *)req
            );
        } else if (!strcmp(
            (const char *)EVBUFFER_DATA(req->url),
            "/video/play"))
        {
            close_conn = websocket_parse(
                req->websock,
                req->bev,
                websocket_frame_ready_video_play,
                (void *)req
            );
        } else if (!strcmp(
            (const char *)EVBUFFER_DATA(req->url),
            "/video/echo"))
        {
            close_conn = websocket_parse(
                req->websock,
                req->bev,
                websocket_frame_ready_video_echo,
                (void *)req
            );
        } else if (!strcmp(
            (const char *)EVBUFFER_DATA(req->url),
            "/video/screen"))
        {
            close_conn = websocket_parse(
                req->websock,
                req->bev,
                websocket_frame_ready_video_screen,
                (void *)req
            );
        } else if (!strcmp(
            (const char *)EVBUFFER_DATA(req->url),
            "/api"))
        {
            close_conn = websocket_parse(
                req->websock,
                req->bev,
                websocket_frame_ready_api,
                (void *)req
            );
        } else {
            PERROR("websock is not supported for url: '%s'\n",
                (const char *)EVBUFFER_DATA(req->url)
            );
            close_conn = 1;
        }
        send_answer = 0;
        goto out;
    }

    // parse generic request
    http_request_parse_bev(req, req->bev);

    // check what request is parsed
    res = http_request_is_parsed(req);
    if (!res){
        goto out;
    }

    // ok, request fully parsed, prepare answer
    answer = http_answer_alloc();

    // store request to answer
    answer->request = req;

    // copy proto from request to answer
    http_answer_proto_set(
        answer,
        (const char *)EVBUFFER_DATA(req->proto)
    );

    // maybe upgrade to websock?
    do {
        HttpHeader  *upgrade = NULL;
        HttpHeader  *key     = NULL;

        upgrade = http_request_header_get(req, "Upgrade");
        if (!upgrade){
            break;
        }
        res = strcmp(
            "websocket",
            (const char *)EVBUFFER_DATA(upgrade->val)
        );
        if (res){
            break;
        }

        // trying upgrade to websocket
        key = http_request_header_get(req, "sec-websocket-key");
        if (!key){
            PERROR("websock upgrade failed,"
                " missing 'sec-websocket-key' header\n"
            );
            goto drop;
        }

        // add Sec-WebSocket-Accept header
        {
            char        buffer[512]             = { 0x00 };
            int32_t     buffer_len              = 0;
            uint8_t     sha1[SHA_DIGEST_LENGTH] = { 0x00 };
            char        sha1_base64[
                base64_encoded_len(SHA_DIGEST_LENGTH) + 1
            ];

            buffer_len = snprintf(buffer, sizeof(buffer),
                "%s%s",
                (const char *)EVBUFFER_DATA(key->val),
                WEBSOCK_STUPID_CONST
            );

            SHA1(
                (const unsigned char *)buffer,
                (size_t)buffer_len,
                (unsigned char *)sha1
            );

            base64_encode(sha1, SHA_DIGEST_LENGTH, sha1_base64);
            http_answer_header_set(
                answer,
                "Sec-WebSocket-Accept",
                sha1_base64
            );
        }

        http_answer_header_set(
            answer,
            "Content-Type",
            "text/html"
        );
        http_answer_header_set(
            answer,
            "Upgrade",
            "WebSocket"
        );
        http_answer_header_set(
            answer,
            "Connection",
            "Upgrade"
        );
        http_answer_status_set(
            answer,
            101,
            "Web Socket Protocol Handshake"
        );

        // prepare websocket
        if (req->websock){
            PWARN("websock already assigned for request\n");
            websocket_free(req->websock);
            req->websock = NULL;
        }
        req->websock = websocket_allocate();
        if (!req->websock){
            PERROR("cannot allocate memory for websocket\n");
            send_answer = 0;
            close_conn  = 1;
            goto fail;
        }

        // send answer
        send_answer = 1;
        close_conn  = 0;
        goto out;
    } while (0);

    if (!strcmp((const char *)EVBUFFER_DATA(req->url), "/api")){
        res = evbuffer_add_printf(
            answer->body,
            "<html>"
            "<body>"
            "<h1>worik was here</h1>"
            "</body>"
            "</html>"
        );
        if (0 >= res){
            PERROR("evbuffer_add_printf failed\n");
            goto fail;
        }

        // all ok
        http_answer_status_set(answer, 200, "OK");
    } else {
        struct evbuffer *path = NULL;

        path = evbuffer_new();
        if (!path){
            PERROR("cannot allocate memory\n");
            goto fail;
        }

        res = evbuffer_add_printf(
            path,
            "%s%s",
            server->root_path,
            (const char *)EVBUFFER_DATA(req->url)
        );
        if (0 >= res){
            PERROR("evbuffer_add_printf failed\n");
            goto fail;
        }

        res = http_answer_body_file_add(
            answer,
            (const char *)EVBUFFER_DATA(path)
        );

        evbuffer_free(path);

        if (res){
            PERROR("http_answer_add_file failed,"
                " res: '%d'\n",
                res
            );
            http_answer_status_set(answer, 404, "Not found");
            goto fail;
        }

        http_answer_header_set(
            answer,
            "Connection",
            "close"
        );

        // all ok
        http_answer_status_set(answer, 200, "OK");
    }

out:
    if (answer){
        if (send_answer){
            http_answer_send(answer);
        }
        http_answer_free(answer);
    }
    if (close_conn){
        http_request_free(fd);
    }
    return;
drop:
    goto out;
fail:
    goto out;
}

void http_server_write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    PWARN("http_write_cb\n");
}

void http_server_event_cb(
    struct bufferevent  *a_bev,
    short               a_what,
    void                *a_ctx)
{
    // HttpServer *http_server = NULL;
    // http_server = (HttpServer *)a_ctx;
    int32_t fd = -1;

    fd = bufferevent_getfd(a_bev);
    http_request_free(fd);
}

void http_server_accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx)
{
    HttpServer  *http_server    = NULL;
    HttpRequest *http_request   = NULL;

    http_server = (HttpServer *)a_ctx;

    if (http_server->accept_cb){
        http_server->accept_cb(
            a_listener,
            a_fd,
            a_src_addr,
            a_src_addr_len,
            a_ctx
        );
    }

    http_request = http_request_alloc(a_fd);
    if (!http_request){
        PFATAL("cannot allocate request, for fd: '%d'\n",
            a_fd
        );
        close(a_fd);
        goto fail;
    }
    //http_request->server = http_server;
    http_request->bev    = bufferevent_socket_new(
        http_server->libevent->evbase,
        a_fd,
        BEV_OPT_CLOSE_ON_FREE
    );
    if (!http_request->bev){
        PFATAL("cannot create bufferevent_socket_new"
            " for fd: '%d'\n",
            a_fd
        );
    }

    bufferevent_setcb(
        http_request->bev,
        http_server_read_cb,
        http_server_write_cb,
        http_server_event_cb,
        http_server // ctx
    );

    bufferevent_enable(
        http_request->bev,
        EV_READ | EV_PERSIST
    );

out:
    return;
fail:
    goto out;
}

void http_server_free(
    HttpServer *a_val)
{
    if (!a_val){
        PFATAL("missing argument: 'a_val'\n");
    }

    if (a_val->evconn){
        evconnlistener_free(a_val->evconn);
        a_val->evconn = NULL;
    }

    http_requests_free();

    free(a_val);
}

HttpServer * http_server_alloc(
    LibEvent        *a_libevent,
    const char      *a_addr,
    const uint32_t  a_port,
    const char      *a_root_path,
    AcceptCb        a_accept_cb,
    ReadCb          a_read_cb,
    WriteCb         a_write_cb,
    EventCb         a_event_cb)
{
    HttpServer  *ret = NULL;
    uint32_t    size = sizeof(*ret);

    ret = (HttpServer *)malloc(size);
    if (!ret){
        PERROR("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            size
        );
        goto fail;
    }
    memset(ret, 0x00, size);

    ret->root_path  = a_root_path;
    ret->libevent   = a_libevent;
    ret->accept_cb  = a_accept_cb;
    ret->read_cb    = a_read_cb;
    ret->write_cb   = a_write_cb;
    ret->event_cb   = a_event_cb;

    ret->evconn = libevent_create_listener(
        a_libevent,
        a_addr,
        a_port,
        http_server_accept_cb,
        ret // ctx
    );
    if (!ret->evconn){
        PFATAL("libevent_create_listener failed,"
            " host: '%s', port: '%d'\n",
            a_addr,
            a_port
        );
    }

out:
    return ret;
fail:
    if (ret){
        http_server_free(ret);
        ret = NULL;
    }
    goto out;
}

