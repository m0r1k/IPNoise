#ifndef NC_TCP_CLIENT_H
#define NC_TCP_CLIENT_H

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

struct nc_tcp_client_context
{
    struct bufferevent  *bev_to_remote;
    struct event_base   *base;
    struct bufferevent  *bev_stdin;
    struct bufferevent  *bev_stdout;
};

static void nc_tcp_client_read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx
);

static void nc_tcp_client_write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx
);

static void nc_tcp_client_event_cb(
    struct bufferevent  *a_bev,
    short               a_what,
    void                *a_ctx
);

static int32_t nc_tcp_client_stdin(
    struct event_base               *a_base,
    struct nc_tcp_client_context    *a_context
);

static int32_t nc_tcp_client_stdout(
    struct event_base               *a_base,
    struct nc_tcp_client_context    *a_context
);

static int32_t nc_tcp_client_connect(
    struct event_base               *a_base,
    struct nc_tcp_client_context    *a_context
);

int32_t nc_tcp_client();

#endif

