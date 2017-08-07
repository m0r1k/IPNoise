#ifndef TCPV4_HPP
#define TCPV4_HPP

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <string>
#include <map>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include "ipnoise-common/log.h"
#include "ipnoise-common/hexdump.h"
#include "common/base_transport.hpp"
#include "channel.hpp"

typedef map<string, Channel *> CHANNELS;

class TcpV4Transport
    : public BaseTransport
{
    public:
        TcpV4Transport(struct event_base *base);
        ~TcpV4Transport();

        int     do_init();
        string  getName();

        static void acceptSignalSocket(
            struct evconnlistener   *listener,
            evutil_socket_t         socket,
            struct sockaddr         *sa,
            int                     socklen,
            void                    *user_data
        );
        static void eventcb(struct bufferevent *bev, short what, void *ctx);
        static void readcb(struct bufferevent *bev, void *ctx);

        int start_xmit(const char *buffer, int len);

    private:
        struct evconnlistener *listener;
        CHANNELS channels;

        int server_start();
        int server_stop();
};

#endif

