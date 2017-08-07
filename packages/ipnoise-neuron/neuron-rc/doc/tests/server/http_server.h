#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "libevent.h"

#define WEBSOCK_STUPID_CONST "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#include "http_request.h"

typedef struct
{
    LibEvent                *libevent;
    struct evconnlistener   *evconn;
    const char              *root_path;
    AcceptCb                accept_cb;
    ReadCb                  read_cb;
    WriteCb                 write_cb;
    EventCb                 event_cb;
} HttpServer;

void http_server_parse_websock(HttpRequest *);

void http_server_read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx
);

void http_server_write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx
);

void http_server_event_cb(
    struct bufferevent  *a_bev,
    short               a_what,
    void                *a_ctx
);

void http_server_accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx
);

void            http_server_free(HttpServer *a_val);
HttpServer *    http_server_alloc(
    LibEvent        *a_libevent,
    const char      *a_addr,
    const uint32_t  a_port,
    const char      *a_root_path,
    AcceptCb        a_accept_cb,
    ReadCb          a_read_cb,
    WriteCb         a_write_cb,
    EventCb         a_event_cb
);

void    http_requests_free();

#endif

