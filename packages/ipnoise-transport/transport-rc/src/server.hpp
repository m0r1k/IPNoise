#ifndef SERVER_HPP
#define SERVER_HPP

#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
//#include <event2/listener.h>
#include <event2/util.h>
//#include <event2/event.h>
#include <event2/event_struct.h>

// stl
#include <string>

// Local
#include "ipnoise-common/log.h"
#include "ipnoise-common/system.h"
#include "ipnoise-common/hexdump.h"
#include "ipnoise-common/strings.h"

#include "common/tun_dev.hpp"
#include "common/base_transport.hpp"
#include "tcp_v4/tcp_v4.hpp"
#include "udp_v4/udp_v4.hpp"

using namespace std;

class Server
{
    public:
        Server();
        ~Server();

        // device name
        string  m_tunname;
        string  m_old_hwaddr;

        int do_init(
            struct event_base   *a_base,
            const string        &a_transport_name,
            const string        &a_lladdr
        );

        string  get_hw_addr();
        int     setup_tun_device(const string &ifname);

        BaseTransport       *m_transport;
        struct event_base   *m_base;
        struct event        *m_timer;

        int netif_rx(const char *buffer, int len);

    protected:
        static void shutdown_cb(evutil_socket_t, short, void *arg);
        static void netdev_start_xmit(struct bufferevent *bev, void *ctx);
        static void netdev_eventcb(struct bufferevent *bev, short what,
            void *ctx);

    private:
        struct bufferevent      *m_bev;
        int                     m_tun_fd;
        static void timer_cb(
            evutil_socket_t     fd,
            short               event,
            void                *arg
        );
};

#endif

