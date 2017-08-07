#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <ipnoise-common/log_common.h>

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

struct event_base   *g_event_base   = NULL;
struct evdns_base   *g_dns_base     = NULL;

void libevent_free()
{
    if (g_dns_base){
        evdns_base_free(
            g_dns_base,
            0  // fail_requests
        );
        g_dns_base = NULL;
    }
    if (g_event_base){
        event_base_free(g_event_base);
        g_event_base = NULL;
    }
}

int32_t libevent_init()
{
    int32_t err = -1;

    libevent_free();

    // event base
    g_event_base = event_base_new();
    if (!g_event_base){
        PERROR("event_base_new failed\n");
        goto fail;
    }

    // event dns
    g_dns_base = evdns_base_new(
        g_event_base,
        1   // initialize_nameservers
    );
    if (!g_dns_base){
        PERROR("evdns_base_new failed\n");
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

void s_accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx)
{
    char    body[16384]   = { 0x00 };
    char    buffer[32768] = { 0x00 };
    int32_t res;

    PWARN("accept, fd: '%d'\n", a_fd);

    res = snprintf(body, sizeof(body),
        "<html>"
        "<head>"
        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"></meta>"
        "<title>Hi there!</title>"
        "</head>"
        "<body>"
        "<h1>Зверушкин зверь!</h1>"
        "</body>"
        "</html>"
    );

    res += snprintf(buffer, sizeof(buffer),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s",
        res,
        body
    );

    PWARN("buffer: '%s'\n", buffer);

    write(a_fd, buffer, res);
    close(a_fd);
}

struct evconnlistener * do_listen(
    const char      *a_addr,
    const int32_t   a_port)
{
    char                    buffer[512] = { 0x00 };
    struct evconnlistener   *listener   = NULL;

    {
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        int32_t         res;

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
            listener = evconnlistener_new_bind(
            //listener = evconnlistener_hostos_new_bind(
                g_event_base,
                s_accept_cb,
                NULL, // ctx
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
    }

out:
    return listener;
fail:
    if (listener){
        evconnlistener_free(listener);
        listener = NULL;
    }
    goto out;
}

int32_t main(void)
{
    int32_t                 res, err = -1;
    struct evconnlistener   *evconnlistener = NULL;

    res = libevent_init();
    if (res){
        err = res;
        PERROR("cannot init libevent\n");
        goto fail;
    }

    evconnlistener = do_listen("0.0.0.0", 8080);
    if (!evconnlistener){
        PFATAL("listen failed\n");
    }

    // start main loop
    event_base_dispatch(g_event_base);

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

