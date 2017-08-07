#ifndef LIBEVENT_HPP
#define LIBEVENT_HPP

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer_compat.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/listener.h>

typedef struct
{
    struct event_base   *evbase;
} LibEvent;

typedef void (*ReadCb)(
    struct bufferevent  *a_bev,
    void                *a_ctx
);

typedef void (*WriteCb)(
    struct bufferevent  *a_bev,
    void                *a_ctx
);

typedef void (*EventCb)(
    struct bufferevent  *a_bev,
    short               a_what,
    void                *a_ctx
);

typedef void (*AcceptCb)(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx
);

void                    libevent_free(LibEvent *);
LibEvent *              libevent_alloc();
struct evconnlistener * libevent_create_listener(
    LibEvent    *a_libevent,
    const char  *a_addr,
    int32_t     a_port,
    AcceptCb    a_accept_cb,
    void        *a_ctx
);

#endif

