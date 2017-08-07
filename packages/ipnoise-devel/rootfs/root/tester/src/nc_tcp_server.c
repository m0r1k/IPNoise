#include "main.h"

#include "nc_tcp_server.h"

static void nc_tcp_server_read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    int32_t                         fd          = -1;
    struct nc_tcp_server_context    *context    = NULL;

    // get context
    context = (struct nc_tcp_server_context *)a_ctx;
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

            PDEBUG(10, "nc_tcp_server_read_cb, fd: '%d'\n",
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

static void nc_tcp_server_write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
}

static void nc_tcp_server_event_cb(
    struct bufferevent  *a_bev,
    short               a_events,
    void                *a_ctx)
{
    int32_t fd = -1;

    fd = bufferevent_getfd(a_bev);

    PDEBUG(10, "nc_tcp_server_event_cb, fd: '%d'\n", fd);

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

static void nc_tcp_server_accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_address,
    int32_t                 a_socklen,
    void                    *a_ctx)
{
    struct event_base           *base       = NULL;
    struct bufferevent          *bev        = NULL;
    struct nc_tcp_server_context    *context    = NULL;

    // get context
    context = (struct nc_tcp_server_context *)a_ctx;
    if (!context){
        PFATAL("cannot get context\n");
    }

    PINFO("accepted new connection fd: '%d'\n", a_fd);

    // we got a new connection, set up a bufferevent for it
    base = evconnlistener_get_base(a_listener);
    bev  = bufferevent_socket_new(
        base,
        a_fd,
        BEV_OPT_CLOSE_ON_FREE
    );
    context->bev_to_remote = bev;

    bufferevent_setcb(
        bev,
        nc_tcp_server_read_cb,
        nc_tcp_server_write_cb,
        nc_tcp_server_event_cb,
        context
    );

    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void nc_tcp_server_accept_error_cb(
    struct evconnlistener   *a_listener,
    void                    *a_ctx)
{
    struct event_base   *base = NULL;
    int32_t             err   = EVUTIL_SOCKET_ERROR();

    base = evconnlistener_get_base(a_listener);

    PERROR("Got an error %d (%s) on the listener. "
        "Shutting down.\n",
        err,
        evutil_socket_error_to_string(err)
    );

    event_base_loopexit(base, NULL);
}

static int32_t nc_tcp_server_bind(
    struct event_base               *a_base,
    struct nc_tcp_server_context    *a_context)
{
    int32_t                 res, err    = -1;
    struct evconnlistener   *listener   = NULL;
    struct sockaddr_in      bind_addr;

    memset(&bind_addr, 0x00, sizeof(bind_addr));

    bind_addr.sin_family = conf.hostos ? PF_HOSTOS : PF_INET,
    bind_addr.sin_addr.s_addr = htonl(0);
    bind_addr.sin_port        = htons(conf.port);

    listener = evconnlistener_new_bind(
        a_base,
        nc_tcp_server_accept_cb,
        a_context,
        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
        -1,
        (struct sockaddr*)&bind_addr,
        sizeof(bind_addr)
    );
    if (!listener) {
        PERROR("Cannot create listener\n");
        goto fail;
    }

    evconnlistener_set_error_cb(
        listener,
        nc_tcp_server_accept_error_cb
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

static int32_t nc_tcp_server_stdin(
    struct event_base               *a_base,
    struct nc_tcp_server_context    *a_context)
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
        nc_tcp_server_read_cb,
        nc_tcp_server_write_cb,
        nc_tcp_server_event_cb,
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

int32_t nc_tcp_server_stdout(
    struct event_base               *a_base,
    struct nc_tcp_server_context    *a_context)
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

int32_t nc_tcp_server()
{
    int32_t                         res, err    = -1;
    struct event_base               *base       = NULL;
    struct nc_tcp_server_context    *context    = NULL;

    int32_t nc_tcp_server_context_len = 0;

    // get context size
    nc_tcp_server_context_len = sizeof(*context);

    // alloc context info
    context = (struct nc_tcp_server_context *)malloc(
        nc_tcp_server_context_len
    );
    if (!context){
        PERROR("cannot allocate memory, was needed: '%d'"
            " byte(s)\n",
            nc_tcp_server_context_len
        );
        goto fail;
    }
    memset(context, 0x00, nc_tcp_server_context_len);

    // create new base
    base = event_base_new();
    if (!base) {
        PERROR("Cannot init event_base_new\n");
        goto fail;
    }
    context->base = base;

    // init stdin
    res = nc_tcp_server_stdin(
        base,
        context
    );
    if (res){
        PERROR("stdin setup failed\n");
        goto fail;
    }

    // init stdout
    res = nc_tcp_server_stdout(
        base,
        context
    );
    if (res){
        PERROR("stdout setup failed\n");
        goto fail;
    }

    // trying to bind
    res = nc_tcp_server_bind(
        base,
        context
    );
    if (res){
        PERROR("nc_tcp_server_bind failed\n");
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

