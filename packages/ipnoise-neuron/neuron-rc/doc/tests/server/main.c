#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <ipnoise-common/log_common.h>

#include "libevent.h"
#include "http_server.h"
#include "main.h"

int32_t         g_debug_level   = 50;
struct event    *signal_event1  = NULL;
struct event    *signal_event2  = NULL;

void accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx)
{
    PWARN("main http_accept_cb, fd: '%d'\n", a_fd);
}

void shutdown_cb(
    evutil_socket_t         a_fd,
    short                   a_what,
    void                    *a_ctx)
{
    LibEvent *libevent = (LibEvent *)a_ctx;

    PINFO("shutdown..\n");

    event_base_loopbreak(libevent->evbase);
}

void signals_free()
{
    if (signal_event1 != NULL){
        event_free(signal_event1);
        signal_event1 = NULL;
    }
    if (signal_event2 != NULL){
        event_free(signal_event2);
        signal_event2 = NULL;
    }
}

int32_t signals_alloc(
    LibEvent *a_libevent)
{
    int32_t err = -1;

    signal_event1 = evsignal_new(
        a_libevent->evbase,
        SIGINT,
        shutdown_cb,
        a_libevent
    );
    if (    !signal_event1
        ||  event_add(signal_event1, NULL) < 0)
    {
        PERROR("Could not create/add a signal event!\n");
        goto fail;
    }

    signal_event2 = evsignal_new(
        a_libevent->evbase,
        SIGTERM,
        shutdown_cb,
        a_libevent
    );
    if (    !signal_event2
        ||  event_add(signal_event2, NULL) < 0)
    {
        PERROR("Could not create/add a signal event!\n");
        goto fail;
    }

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

int32_t main(void)
{
    int32_t     res, err        = -1;
    LibEvent    *libevent       = NULL;
    HttpServer  *http_server    = NULL;

    libevent = libevent_alloc();
    if (!libevent){
        PERROR("cannot init libevent\n");
        goto fail;
    }

    // init SIGTERM SIGINT
    res = signals_alloc(libevent);
    if (res){
        PERROR("cannot init signals, res: '%d'\n",
            res
        );
        goto fail;
    }

    // init http server
    http_server = http_server_alloc(
        libevent,
        "0.0.0.0",
        8080,
        "/mnt/multimedia/git/IPNoise/packages/ipnoise-neuron/neuron-rc/",
        NULL,           // accept_cb
        NULL,           // read_cb
        NULL,           // write_cb
        NULL            // accept_cb
    );
    if (!http_server){
        PERROR("cannot init http_server\n");
        goto fail;
    }

    // start main loop
    event_base_dispatch(libevent->evbase);

    // shutdown
    http_server_free(http_server);
    signals_free();
    libevent_free(libevent);

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

