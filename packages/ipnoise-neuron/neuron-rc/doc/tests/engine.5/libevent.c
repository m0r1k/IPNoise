#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <ipnoise-common/log_common.h>

#include "libevent.h"

LibEvent * libevent_alloc()
{
    LibEvent    *ret = NULL;
    uint32_t    size = sizeof(*ret);

    ret = (LibEvent *)calloc(1, size);
    if (!ret){
        PERROR("cannot allocate memory,"
            " was need: '%d' byte(s)",
            size
        );
        goto fail;
    }

    ret->evbase = event_base_new();
    if (!ret->evbase){
        PERROR("event_base_new() failed\n");
        goto fail;
    }

out:
    return ret;
fail:
    if (ret){
        libevent_free(ret);
        ret = NULL;
    }
    goto out;
}

void libevent_free(
    LibEvent *a_val)
{
    if (!a_val){
        PFATAL("missing argument: 'a_val'\n");
    }

    if (a_val->evbase){
        event_base_free(a_val->evbase);
        a_val->evbase = NULL;
    }

    free(a_val);
}

void s_accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx)
{
    //struct request  *req = NULL;

    char    buffer[512] = { 0x00 };
    int32_t res;

    PWARN("accept fd: '%d'\n", a_fd);

    do {
        res = read(a_fd, buffer, sizeof(buffer));
        PWARN("res: '%d'\n", res);
        usleep(1e6);
    } while (1);

/*
    req = req_alloc(a_fd);
    if (!req){
        PERROR("cannot allocate request for fd: '%d'\n",
            a_fd
        );
        close(a_fd);
    }
*/

}

struct evconnlistener * libevent_create_listener(
    LibEvent    *a_libevent,
    const char  *a_addr,
    int32_t     a_port,
    AcceptCb    a_accept_cb,
    void        *a_ctx)
{
    char                    buffer[512] = { 0x00 };
    struct evconnlistener   *listener   = NULL;
    struct addrinfo         hints;
    struct addrinfo         *result, *rp;
    int32_t                 res;

    memset(&hints, 0x00, sizeof(struct addrinfo));

    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = 0;
    hints.ai_protocol   = 0;

    snprintf(buffer, sizeof(buffer), "%d", a_port);
    res = getaddrinfo(
        a_addr,
        buffer,   // service
        &hints,
        &result
    );
    if (res){
        PERROR("getaddrinfo host: '%s', port: '%d'"
            " failed (%s)\n",
            a_addr,
            a_port,
            gai_strerror(res)
        );
        goto fail;
    }

    // getaddrinfo() returns a list of address structures.
    // Try each address until we successfully bind
    for (rp = result; rp != NULL; rp = rp->ai_next){
        struct sockaddr_in *addr = NULL;
        addr = (struct sockaddr_in *)rp->ai_addr->sa_data;
        in_addr_t host = addr->sin_addr.s_addr;

        PWARN("bind %d.%d.%d.%d\n",
            (uint8_t)((host & 0xff000000) >> 24),
            (uint8_t)((host & 0x00ff0000) >> 16),
            (uint8_t)((host & 0x0000ff00) >> 8),
            (uint8_t)((host & 0x000000ff) >> 0)
        );
        listener = evconnlistener_new_bind(
        //listener = evconnlistener_hostos_new_bind(
            a_libevent->evbase,
            a_accept_cb,
            a_ctx,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
            -1,
            rp->ai_addr,
            rp->ai_addrlen
        );
        if (listener){
            break;
        }
    }

    freeaddrinfo(result);

    if (!listener){
        PERROR("cannot not create a listener at %s:%d\n",
            a_addr,
            a_port
        );
        goto fail;
    }

out:
    return listener;
fail:
    if (listener){
        evconnlistener_free(listener);
    }
    goto out;
}

struct event * libevent_add_event(
    LibEvent        *a_libevent,
    const int32_t   a_fd,
    const int32_t   a_flags,
    void            (*a_cb)(int32_t, short, void*),
    void            *a_ctx)
{
    struct event *ev = NULL;

    ev = event_new(
        a_libevent->evbase,
        a_fd,
        a_flags,
        a_cb,
        a_ctx
    );

    event_add(ev, NULL);

    return ev;
}

void libevent_loop_start(
    LibEvent        *a_libevent,
    const int32_t   a_flags)
{
    event_base_loop(
        a_libevent->evbase,
        a_flags
    );
}

void libevent_loop_stop(
    LibEvent    *a_libevent)
{
    event_base_loopbreak(a_libevent->evbase);
}

