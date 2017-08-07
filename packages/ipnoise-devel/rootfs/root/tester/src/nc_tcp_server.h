#ifndef NC_TCP_SERVER_H
#define NC_TCP_SERVER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "ipnoise-common/log_common.h"

struct nc_tcp_server_context
{
    struct bufferevent  *bev_to_remote;
    struct event_base   *base;
    struct bufferevent  *bev_stdin;
    struct bufferevent  *bev_stdout;
};

static void nc_tcp_server_read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx
);

static void nc_tcp_server_write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx
);

static void nc_tcp_server_event_cb(
    struct bufferevent  *a_bev,
    short               a_events,
    void                *a_ctx
);

static void nc_tcp_server_accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_address,
    int32_t                 a_socklen,
    void                    *a_ctx
);

static void nc_tcp_server_accept_error_cb(
    struct evconnlistener   *a_listener,
    void                    *a_ctx
);

static int32_t nc_tcp_server_bind(
    struct event_base           *a_base,
    struct nc_tcp_server_context    *a_context
);

int32_t nc_tcp_server();

#endif

