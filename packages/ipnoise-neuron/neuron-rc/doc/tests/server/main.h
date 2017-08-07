#ifndef MAIN_HPP
#define MAIN_HPP

#include <stdint.h>

// libevent
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/listener.h>
#include <event2/util.h>

void accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx
);

void shutdown_cb(
    evutil_socket_t a_fd,
    short           a_what,
    void            *a_ctx
);

void    signals_free();
int32_t signals_alloc(LibEvent *a_libevent);
int32_t main(void);

#endif

