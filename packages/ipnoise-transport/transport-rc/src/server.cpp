#define RD_BUFFER_SIZE 65536

#include "server.hpp"

Server::Server()
{
    m_transport      = NULL;
    m_base           = NULL;
    m_bev            = NULL;
    m_tun_fd         = -1;
    m_timer          = NULL;
    m_old_hwaddr     = "";
};

Server::~Server()
{
};

void Server::shutdown_cb(evutil_socket_t, short, void *a_ctx)
{
    Server *server = (Server *)a_ctx;
    assert(server != NULL);

    PDEBUG(5, "shutdown..\n");
    event_base_loopexit(server->m_base, NULL);
};

void Server::netdev_start_xmit(struct bufferevent *a_bev, void *a_ctx)
{
    char buffer[RD_BUFFER_SIZE + 1];
    int res, rd;

    Server *server = (Server *)a_ctx;
    assert(server != NULL);

    PDEBUG(5, "Server::netdev_start_xmit\n");

    do {
        buffer[0] = 0x00;
        rd = bufferevent_read(a_bev, buffer, RD_BUFFER_SIZE);
        if (rd > 0){
            // PDEBUG(5, "was received:\n");
            // ipnoise_hexdump ((unsigned char *)buffer, res);
            buffer[rd] = 0x00;
            res = server->m_transport->start_xmit(buffer, rd);
            if (res){
                PERROR("start_xmit failed\n");
            }
        }
    } while (rd > 0);
};

void Server::netdev_eventcb(
    struct bufferevent  *,  // a_bev,
    short,                  // a_what,
    void                *a_ctx)
{
    Server *server  = (Server *)a_ctx;
    assert(server != NULL);

    PDEBUG(5, "new event!\n");
};

int Server::netif_rx(const char *a_buffer, int a_len)
{
    int err = -1;
    // PDEBUG(5, "was received packet '%d' byte(s)\n", a_len);
    err = tun_write(m_tun_fd, (const void *)a_buffer, a_len);
    // ipnoise_hexdump((unsigned char *)buffer, len);
    // PDEBUG(5, "tun_write: err: '%d'\n", err);
    return err;
}

int Server::setup_tun_device(const string &a_ifname)
{
    int err = -1;

    // create tun name
    m_tunname = a_ifname + "_%d";

    // trying to open
    m_tun_fd = tun_open(m_tunname);
    if (m_tun_fd < 0){
        goto fail;
    }

    // make interface up
    err = tun_bring_up(m_tunname);
    if (err){
        PERROR("Cannot up device: '%s'\n", m_tunname.c_str());
        goto fail;
    }

    // trying to create bufferevent socket
    m_bev = bufferevent_socket_new(
        m_base,
        m_tun_fd,
        BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS
    );
    if (not m_bev){
        //PERROR("Cannot create bufferevent socket\n");
        goto fail;
    }

    // setup callbacks
    bufferevent_setcb(m_bev,
        Server::netdev_start_xmit,
        NULL,
        Server::netdev_eventcb,
        (void *)this
    );
    bufferevent_enable(m_bev, EV_READ|EV_WRITE|EV_PERSIST);

    // all ok
    err = 0;

out:
    return err;
fail:
    goto out;
}

string Server::get_hw_addr()
{
    string  ret;
    int     res;

    if (0){
        // TODO XXX use tun_get_hwaddr here (need be checked)
        // 20120626 morik@
        tun_get_hwaddr(m_tunname, ret);
    } else {
        string std_out, std_err;
        string::size_type pos;

        const char * const argv[] = {
            "/sbin/ip",
            "link",
            "show",
            m_tunname.c_str(),
            NULL
        };
        res = my_system(argv, std_out, std_err);
        if (res){
            goto fail;
        }

        // parse strings looks like:
        // "    link/ipnoise 127.0.0.1:55553 brd"
        // "    link/ipnoise  brd"
        pos = std_out.find("ipnoise");
        if (pos == string::npos){
            goto fail;
        }

        pos = std_out.find(" ", pos);
        if (pos == string::npos){
            goto fail;
        }

        do {
            pos++;
            char symbol = std_out.at(pos);
            if (symbol == ' '){
                break;
            }
            ret += symbol;
        } while (pos < std_out.size());
    }

out:
    return ret;

fail:
    goto out;
}

void Server::timer_cb(
    evutil_socket_t,    // a_fd,
    short,              // a_event
    void                *a_arg)
{
    string cur_hwaddr;
    Server *server = (Server *)a_arg;
    assert (server != NULL);

    cur_hwaddr = server->get_hw_addr();

    if (cur_hwaddr != server->m_old_hwaddr){
        PINFO("[%s] hw adress was changed, '%s' => '%s'"
            " trying to restart transport\n",
                server->m_transport->getName().c_str(),
                server->m_old_hwaddr.c_str(),
                cur_hwaddr.c_str()
        );
        server->m_old_hwaddr = cur_hwaddr;
        server->m_transport->setBindAddrPort(cur_hwaddr);
        server->m_transport->do_init();
    }

    // reschedule
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 1;
    evtimer_add(server->m_timer, &tv);
}

int Server::do_init(
    struct event_base   *a_base,
    const string        &a_transport_name,
    const string        &a_lladdr)
{
    int i, res, err             = -1;
    struct event *signal_event  = NULL;
    struct timeval  tv;
    string address;

    if (not a_base){
        goto fail;
    }

    // store ev base
    m_base = a_base;

    // handle interrupts
    signal_event = evsignal_new(
        m_base,
        SIGINT,
        Server::shutdown_cb,
        (void *)this
    );

    if (!signal_event || event_add(signal_event, NULL) < 0){
        //PERROR("Could not create/add a signal event!\n");
        goto fail;
    }

    // setup tap device
    err = setup_tun_device(a_transport_name);
    if (err){
        PERROR("Cannot setup tun device: '%s'\n", a_transport_name.c_str());
        goto fail;
    }

    // create transport
    if ("tcp_v4" == a_transport_name){
        m_transport = new TcpV4Transport(a_base);
    } else
    if ("udp_v4" == a_transport_name){
        m_transport = new UdpV4Transport(a_base);
    } else {
        PERROR("Unsupported transport: '%s'\n",
            a_transport_name.c_str());
        goto fail;
    }

    // setup bind address
    m_transport->setBindAddrPort(a_lladdr);

    // try to init
    err = m_transport->do_init();
    if (err){
        PERROR("Cannot init transport: '%s'\n",
            m_transport->getName().c_str());
        goto fail;
    }

    // store us
    m_transport->setServer(this);

    // create link address
    address = "s:" + m_transport->getBindAddrPort();

    if (0){
        // TODO XXX use tun_set_hwaddr here (need be checked)
        // 20120626 morik@
        tun_set_hwaddr(m_tunname, address);
    } else {
        string std_out, std_err;
        const char * const argv[] = {
            "/sbin/ip",
            "link",
            "set",
            m_tunname.c_str(),
            "up",
            "address",
            address.c_str(),
            NULL
        };
        for (i = 0; i < 10; i++){
            res = my_system(argv, std_out, std_err);
            if (not res){
                break;
            }
            sleep (1);
        }
        if (res){
            PERROR("[%s] Cannot setup link address: '%s': %s",
                m_tunname.c_str(),
                address.c_str(),
                std_err.c_str()
            );
            goto fail;
        }

        // store device hw addr
        m_old_hwaddr = get_hw_addr();
    }

    if (m_timer == NULL){
        m_timer = evtimer_new(m_base, Server::timer_cb, (void*)this);
        evutil_timerclear(&tv);
        tv.tv_sec = 1;
        evtimer_add(m_timer, &tv);
    }

    // all ok
    err  = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

