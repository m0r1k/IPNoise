#ifndef UDPV4_H
#define UDPV4_H

#include <string>

#include "ipnoise-common/log.h"
#include "ipnoise-common/hexdump.h"

#include "common/base_transport.hpp"

using namespace std;

class UdpV4Transport
    : public BaseTransport
{
    public:
        UdpV4Transport(struct event_base *base);
        ~UdpV4Transport();

        int do_init();
        string getName();
        int start_xmit(const char*, int);

        static void readcb(evutil_socket_t fd, short events, void *ctx);
        static void eventcb(struct bufferevent *bev, short what, void *ctx);

    private:
        int fd;
        struct event _event;
        int _event_inited;

        int server_start();
        int server_stop();
};

#endif

