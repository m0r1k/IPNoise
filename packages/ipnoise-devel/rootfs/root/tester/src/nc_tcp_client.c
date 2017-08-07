#include "main.h"

#include "nc_tcp_client.h"

static void nc_tcp_client_read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    int32_t                         res, fd     = -1;
    struct  nc_tcp_client_context   *context    = NULL;

    // get context
    context = (struct nc_tcp_client_context *)a_ctx;
    if (!context){
        PFATAL("cannot get context\n");
    }

    // get descriptor
    fd = bufferevent_getfd(a_bev);

    PDEBUG(10, "nc_tcp_client_read_cb, fd: '%d'\n", fd);

    if (FD_STDIN == fd){
        // process stdin
        struct evbuffer *input  = NULL;
        struct evbuffer *output = NULL;

        input  = bufferevent_get_input(a_bev);
        if (context->bev_to_remote){
            output = bufferevent_get_output(
                context->bev_to_remote
            );
            evbuffer_add_buffer(output, input);
        } else {
            PERROR("attempt to copy data from stdin"
                " to closed socket\n"
            );
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

static void nc_tcp_client_write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    int32_t fd = -1;

    fd = bufferevent_getfd(a_bev);

    PDEBUG(10, "nc_tcp_client_write_cb, fd: '%d'\n", fd);
}

static void nc_tcp_client_event_cb(
    struct bufferevent  *a_bev,
    short               a_what,
    void                *a_ctx)
{
    int32_t                         fd          = -1;
    struct  event_base              *base       = NULL;
    struct  nc_tcp_client_context   *context    = NULL;

    // get context
    context = (struct nc_tcp_client_context *)a_ctx;
    if (!context){
        PFATAL("cannot get context\n");
    }

    // get base and fd
    base = context->base;
    fd   = bufferevent_getfd(a_bev);

    PDEBUG(10, "nc_tcp_client_event_cb, fd: '%d',"
        " a_what: 0x'%x'\n",
        fd,
        a_what
    );

    PWARN("nc_tcp_client_event_cb, fd: '%d',"
        " a_what: 0x'%x'\n",
        fd,
        a_what
    );


    if (BEV_EVENT_ERROR & a_what){
        PERROR("socket error\n");
        if (BEV_EVENT_READING & a_what){
            PERROR("socket read failed\n");
        }
        goto free;
    } else if (BEV_EVENT_EOF & a_what){
        PINFO("connection closed by remote part\n");
        goto free;
    } else if (a_what & BEV_EVENT_CONNECTED){
        PINFO("connected\n");
    }

out:
    return;
free:
//    if (a_bev){
//        bufferevent_free(a_bev);
//        a_bev = NULL;
//    }
    event_base_loopexit(base, NULL);
    goto out;
}

static int32_t nc_tcp_client_stdin(
    struct event_base               *a_base,
    struct nc_tcp_client_context    *a_context)
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
        nc_tcp_client_read_cb,
        nc_tcp_client_write_cb,
        nc_tcp_client_event_cb,
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

int32_t nc_tcp_client_stdout(
    struct event_base               *a_base,
    struct nc_tcp_client_context    *a_context)
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
        nc_tcp_client_read_cb,
        nc_tcp_client_write_cb,
        nc_tcp_client_event_cb,
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

static int32_t nc_tcp_client_connect(
    struct event_base               *a_base,
    struct nc_tcp_client_context    *a_context)
{
    int32_t             res, err    = -1;
    int32_t             fd          = -1;
    int32_t             options     = 0;

    struct sockaddr_in  server_addr;
    int32_t             server_addrlen = sizeof(server_addr);

    // setup address and port
    memset(&server_addr, 0x00, sizeof(server_addr));
    memcpy(
        &server_addr.sin_addr.s_addr,
        &conf.addr,
        sizeof(server_addr.sin_addr.s_addr)
    );
    server_addr.sin_port    = htons(conf.port);
    server_addr.sin_family  = AF_INET;

    // create socket
    fd = socket(
        conf.hostos ? PF_HOSTOS : PF_INET,
        SOCK_STREAM,
        TCP_PROTO_NUM
    );
    if (fd < 0){
        PERROR("socket(%d, SOCK_STREAM, %d) failed\n",
            conf.hostos ? PF_HOSTOS : PF_INET,
            TCP_PROTO_NUM
        );
        goto fail;
    }
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1){
        PERROR("Cannot set socket: '%d'"
            " in non-blocked mode\n",
            fd
        );
        goto fail;
    }

    // do all work async
    set_non_block(fd);

    // create event
    a_context->bev_to_remote = bufferevent_socket_new(
        a_base,
        fd,
        options
    );
    if (!a_context->bev_to_remote){
        PERROR("bufferevent_socket_new failed\n");
        goto fail;
    }

    // setup callback
    bufferevent_setcb(
        a_context->bev_to_remote,
        nc_tcp_client_read_cb,
        nc_tcp_client_write_cb,
        nc_tcp_client_event_cb,
        a_context
    );

    bufferevent_enable(
        a_context->bev_to_remote,
        EV_READ | EV_WRITE | EV_PERSIST
    );

    // connect
    res = bufferevent_socket_connect(
        a_context->bev_to_remote,
        (struct sockaddr *)&server_addr,
        server_addrlen
    );
    if (res){
        PERROR("bufferevent_socket_connect failed\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (a_context->bev_to_remote){
        bufferevent_free(a_context->bev_to_remote);
        a_context->bev_to_remote = NULL;
    }
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int32_t nc_tcp_client()
{
    int32_t                         res, err    = -1;
    struct  event_base              *base       = NULL;
    struct  nc_tcp_client_context   *context    = NULL;

    int32_t nc_tcp_client_context_len = 0;

    // get context size
    nc_tcp_client_context_len = sizeof(*context);

    // alloc context info
    context = (struct nc_tcp_client_context *)malloc(
        nc_tcp_client_context_len
    );
    if (!context){
        PERROR("cannot allocate memory, was needed: '%d'"
            " byte(s)\n",
            nc_tcp_client_context_len
        );
        goto fail;
    }
    memset(context, 0x00, nc_tcp_client_context_len);

    // create new base
    base = event_base_new();
    if (!base) {
        PERROR("Cannot init event_base_new\n");
        goto fail;
    }
    context->base = base;

    // init stdin
    res = nc_tcp_client_stdin(
        base,
        context
    );
    if (res){
        PERROR("stdin setup failed\n");
        goto fail;
    }

    // init stdout
    res = nc_tcp_client_stdout(
        base,
        context
    );
    if (res){
        PERROR("stdout setup failed\n");
        goto fail;
    }

    // trying to connect
    res = nc_tcp_client_connect(
        base,
        context
    );
    if (res){
        PERROR("nc_tcp_client_connect failed\n");
        goto fail;
    }

    // process events
    event_base_dispatch(base);

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

