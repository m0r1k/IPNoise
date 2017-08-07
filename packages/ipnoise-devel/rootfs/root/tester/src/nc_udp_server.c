#include <ipnoise-common/utils.h>
#include "main.h"

#include "nc_udp_server.h"

static void nc_udp_server_read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    int32_t                         fd          = -1;
    struct nc_udp_server_context    *context    = NULL;

    // get context
    context = (struct nc_udp_server_context *)a_ctx;
    if (!context){
        PFATAL("cannot get context\n");
    }

    fd = bufferevent_getfd(a_bev);
    if (FD_STDIN == fd){
        if (context->bev_to_remote){
            struct evbuffer *input  = NULL;
            struct evbuffer *output = NULL;

            input  = bufferevent_get_input(a_bev);
            output = bufferevent_get_output(
                context->bev_to_remote
            );

            PDEBUG(10, "nc_udp_server_read_cb, fd: '%d'\n",
                fd);

            // copy all the data from the input buffer
            // to the output buffer
            evbuffer_add_buffer(output, input);
        }
    } else if (FD_STDOUT == fd){
    } else if (FD_STDERR < fd){
        struct evbuffer *input  = NULL;
        struct evbuffer *output = NULL;

        input  = bufferevent_get_input(a_bev);
        output = bufferevent_get_output(
            context->bev_stdout
        );

        evbuffer_add_buffer(output, input);
    }
}

static void nc_udp_server_write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
}

static void nc_udp_server_event_cb(
    struct bufferevent  *a_bev,
    short               a_events,
    void                *a_ctx)
{
    int32_t fd = -1;

    fd = bufferevent_getfd(a_bev);

    PDEBUG(10, "nc_udp_server_event_cb, fd: '%d'\n", fd);

    if (a_events & BEV_EVENT_ERROR){
        PERROR("Error from bufferevent");
        goto fail;
    }

    if (a_events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)){
        PINFO("connection closed, fd: '%d'\n", fd);
        bufferevent_free(a_bev);
    }

out:
    return;
fail:
    goto out;
}

static int32_t nc_udp_server_bind(
    struct event_base               *a_base,
    struct nc_udp_server_context    *a_context)
{
    int32_t                 res, err    = -1;
    int32_t                 fd          = -1;
    struct sockaddr_in      bind_addr;
    socklen_t               bind_addr_len;

    bind_addr_len = sizeof(bind_addr);
    memset(&bind_addr, 0x00, sizeof(bind_addr));

    bind_addr.sin_family      = PF_INET,
    bind_addr.sin_addr.s_addr = htonl(0);
    bind_addr.sin_port        = htons(conf.port);

    fd = socket(
        conf.hostos ? PF_HOSTOS : PF_INET,
        SOCK_DGRAM,
        getprotonobyname("udp")
    );
    if (0 > fd){
        PERROR("cannot create socket for port: '%d'\n",
            conf.port
        );
        goto fail;
    }

    res = bind(
        fd,
        (struct sockaddr *)&bind_addr,
        bind_addr_len
    );
    if (res){
        PERROR("bind port: '%d', for fd: '%d' failed\n",
            conf.port,
            fd
        );
        goto fail;
    }

    // do all work async
    set_non_block(fd);

    a_context->bev_to_remote = bufferevent_socket_new(
        a_base,
        fd,
        0
    );
    if (!a_context->bev_to_remote){
        PERROR("bufferevent_socket_new"
            " failed for port: '%d', fd: '%d'\n",
            conf.port,
            fd
        );
        goto fail;
    }

    bufferevent_setcb(
        a_context->bev_to_remote,
        nc_udp_server_read_cb,
        nc_udp_server_write_cb,
        nc_udp_server_event_cb,
        a_context
    );

    bufferevent_enable(
        a_context->bev_to_remote,
        EV_READ | EV_WRITE | EV_PERSIST
    );

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    if (0 <= fd){
        close(fd);
        fd = -1;
    }
    goto out;
}

static int32_t nc_udp_server_stdin(
    struct event_base               *a_base,
    struct nc_udp_server_context    *a_context)
{
    int32_t     res, err    = -1;
    int32_t     fd          = FD_STDIN;
    int32_t     options     = 0;

    // create event
    a_context->bev_stdin = bufferevent_socket_new(
        a_base,
        fd,
        options
    );
    if (!a_context->bev_stdin){
        PERROR("bufferevent_socket_new failed\n");
        goto fail;
    }

    // setup callback
    bufferevent_setcb(
        a_context->bev_stdin,
        nc_udp_server_read_cb,
        nc_udp_server_write_cb,
        nc_udp_server_event_cb,
        a_context
    );

    bufferevent_enable(
        a_context->bev_stdin,
        EV_READ | EV_WRITE | EV_PERSIST
    );

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t nc_udp_server_stdout(
    struct event_base               *a_base,
    struct nc_udp_server_context    *a_context)
{
    int32_t             res, err    = -1;
    int32_t             fd          = FD_STDOUT;
    int32_t             options     = 0;

    // create event
    a_context->bev_stdout = bufferevent_socket_new(
        a_base,
        fd,
        options
    );
    if (!a_context->bev_stdout){
        PERROR("bufferevent_socket_new failed\n");
        goto fail;
    }

/*
    // setup callback
    bufferevent_setcb(
        a_context->bev_stdout,
        nc_client_readcb,
        nc_client_writecb,
        nc_client_eventcb,
        a_context
    );

    bufferevent_enable(
        a_context->bev_stdout,
        EV_READ | EV_WRITE | EV_PERSIST
    );
*/

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t nc_udp_server()
{
    int32_t                         res, err    = -1;
    struct event_base               *base       = NULL;
    struct nc_udp_server_context    *context    = NULL;

    int32_t nc_udp_server_context_len = 0;

    // get context size
    nc_udp_server_context_len = sizeof(*context);

    // alloc context info
    context = (struct nc_udp_server_context *)malloc(
        nc_udp_server_context_len
    );
    if (!context){
        PERROR("cannot allocate memory, was needed: '%d'"
            " byte(s)\n",
            nc_udp_server_context_len
        );
        goto fail;
    }
    memset(context, 0x00, nc_udp_server_context_len);

    // create new base
    base = event_base_new();
    if (!base) {
        PERROR("Cannot init event_base_new\n");
        goto fail;
    }
    context->base = base;

    // init stdin
    res = nc_udp_server_stdin(
        base,
        context
    );
    if (res){
        PERROR("stdin setup failed\n");
        goto fail;
    }

    // init stdout
    res = nc_udp_server_stdout(
        base,
        context
    );
    if (res){
        PERROR("stdout setup failed\n");
        goto fail;
    }

    // trying to bind
    res = nc_udp_server_bind(
        base,
        context
    );
    if (res){
        PERROR("nc_udp_server_bind failed\n");
        goto fail;
    }

    // process events
    event_base_dispatch(base);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

