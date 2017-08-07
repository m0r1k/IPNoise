#include <assert.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ipnoise-common/ipnoise.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include "server.hpp"
#include "udp_v4.hpp"

#define DEFAULT_BIND_IP     "0.0.0.0"
#define DEFAULT_BIND_PORT   22102

#define RD_BUFFER_SIZE 65535

UdpV4Transport::UdpV4Transport(struct event_base *base)
    :   BaseTransport(base)
{
    fd              = -1;
    _event_inited   = 0;

    setBindAddr(DEFAULT_BIND_IP);
    setBindPort(DEFAULT_BIND_PORT);
}

UdpV4Transport::~UdpV4Transport()
{
}

string UdpV4Transport::getName()
{
    string ret = "udp";
    return ret;
}

void UdpV4Transport::readcb(
    evutil_socket_t,    // fd
    short               events,
    void                *ctx)
{
    int                 res;
    struct sockaddr_in  from;
    socklen_t           fromlen         = sizeof(from);
    int                 buffer_size     = RD_BUFFER_SIZE
        + sizeof(struct ipnoisehdr);
    char                buffer[buffer_size + 1];
    struct ipnoisehdr   *hdr            = NULL;

    memset(&from, 0x00, sizeof(from));
    memset(buffer, 0x00, buffer_size);

    hdr = (struct ipnoisehdr *)buffer;
    ipnoise_hdr_init(hdr);

    UdpV4Transport *transport = (UdpV4Transport *)ctx;
    assert (transport != NULL);

    Server *server = (Server *)transport->getServer();
    assert(server != NULL);

    if (!(events & EV_READ)){
        goto out;
    }

    PDEBUG(5, "read callback\n");

    do {
        PDEBUG(5, "transport->fd: '%d'\n", transport->fd);
        fromlen = sizeof(from);
        res = recvfrom(
            transport->fd,
            buffer      + sizeof(struct ipnoisehdr),
            buffer_size - sizeof(struct ipnoisehdr),
            0,  // flags
            (struct sockaddr *)&from,
            &fromlen
        );
        PDEBUG(5, "recvfrom: res: '%d'\n", res);
        if (res > 0){
            buffer[res + sizeof(struct ipnoisehdr)] = 0x00;
            // fill source addr info
            snprintf((char *)hdr->h_source, sizeof(hdr->h_source),
                "%s:%d",
                inet_ntoa(from.sin_addr),
                ntohs(from.sin_port)
            );

            PDEBUG(5, "was received packed from: '%s', %d byte(s)\n",
                hdr->h_source,
                res
            );
            // start rx packet
            server->netif_rx(buffer, res + sizeof(struct ipnoisehdr));
        }
    } while (res > 0);

out:
    return;
}

void UdpV4Transport::eventcb(
    struct bufferevent  *,  // bev
    short                   what,
    void                    *ctx)
{
    UdpV4Transport *transport = (UdpV4Transport *)ctx;
    assert (transport != NULL);

    PDEBUG(5, "event callback, what: '%d'\n", what);
}


int UdpV4Transport::server_stop()
{
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    if (_event_inited){
        event_del(&_event);
        _event_inited = 0;
    }

    return 0;
}

int UdpV4Transport::server_start()
{
    struct protoent *proto  = NULL;
    int res, err            = -1;
    struct sockaddr_in in_addr;
    int on;

    string  bind_addr;
    int     bind_port;

    bind_addr = getBindAddr();
    if (!bind_addr.size()){
        bind_addr = DEFAULT_BIND_IP;
    }

    bind_port = getBindPort();
    if (bind_port <= 0){
        bind_port = DEFAULT_BIND_PORT;
    }

    // clear addr
    memset(&in_addr, 0x00, sizeof(in_addr));

    proto = getprotobyname("udp");
    fd = socket(PF_HOSTOS, SOCK_DGRAM, proto->p_proto);
    if (fd < 0){
        PERROR("Cannot create socket\n");
        goto fail;
    }

    res = evutil_make_socket_nonblocking(fd);
    if (res < 0){
        PERROR("Cannot make socket not blocking\n");
        close (fd);
        fd = -1;
        goto fail;
    }

    in_addr.sin_family  = AF_INET;
    in_addr.sin_port    = htons(bind_port);
    res = inet_aton(bind_addr.c_str(), &in_addr.sin_addr);
    if (!res){
        PERROR("Cannot parse addr: '%s'\n", bind_addr.c_str());
        goto fail;
    }

    res = setsockopt(
        fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        &on,
        sizeof(on)
    );
    if (res){
        PERROR("Failed to set 'SO_REUSEADDR' flag\n");
    }

    res = bind(
        fd,
        (struct sockaddr *)&in_addr,
        sizeof(in_addr)
    );
    if (res){
        PERROR("Bind failed\n");
        goto fail;
    }

    event_assign(
        &_event,
        getEventBase(),
        fd,
        EV_READ | EV_PERSIST,
        UdpV4Transport::readcb,
        (void *)this
    );
    _event_inited = 1;

    res = event_add(&_event, NULL);
    if (res < 0){
        PERROR("Cannot create event\n");
        goto fail;
    }

/*
    _bev = bufferevent_socket_new(
        getEventBase(),
        fd,
        BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS
    );
    if (!_bev){
        PERROR("Cannot create bufferevent_socket_new\n");
        goto fail;
    }

    bufferevent_setcb(
        _bev,
        UdpV4Transport::readcb,
        NULL,
        UdpV4Transport::eventcb,
        (void *)this
    );

    bufferevent_enable(_bev, EV_READ|EV_WRITE|EV_PERSIST);
*/

    // all ok
    err = 0;

    PINFO("[%s] transport was inited successful at: '%s'\n",
        getName().c_str(),
        getBindAddrPort().c_str()
    );

out:
    return err;

fail:
    PERROR("[%s] cannot init transport at: '%s'\n",
        getName().c_str(),
        getBindAddrPort().c_str()
    );
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int UdpV4Transport::do_init()
{
    int res, err = -1;

    // stop current server if need
    res = server_stop();

    // start new server
    res = server_start();
    if (res){
        err = res;
        goto fail;
    }

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

int UdpV4Transport::start_xmit(const char *buffer, int len)
{
    int res, err = -1;
    struct ipnoisehdr   *hdr = (struct ipnoisehdr *)buffer;
    const char          *dst_addr = (const char *)hdr->h_dest;
    string  dest                    = dst_addr;
    string  host                    = "";
    string  port                    = "";
    int     port_int                = 0;
    size_t  pos                     = string::npos;

    struct sockaddr_in in_addr;

    if ((len - sizeof(struct ipnoisehdr)) <= 0){
        PERROR("buffer length less that 'struct ipnoisehdr'\n");
        goto fail;
    }

    // check what it is ipnoise packet
    res = ipnoise_hdr_is_valid(hdr);
    if (!res){
        PERROR("request to send packet without 'ipnoise' header\n");
        ipnoise_hexdump((unsigned char *)buffer, len);
        goto fail;
    }

    pos = dest.find(":");
    if (pos == string::npos){
        PERROR("invalid destination address: '%s' must be ip:port\n",
            dest.c_str());
        goto fail;
    }

    host        = dest.substr(0, pos);
    port        = dest.substr(pos + 1);
    port_int    = atoi(port.c_str());

    // PDEBUG(5, "need xmit to: '%s:%d'\n", host.c_str(), port_int);
    // ipnoise_hexdump((unsigned char *)buffer, len);

    // clear addr
    memset(&in_addr, 0x00, sizeof(in_addr));

    in_addr.sin_family  = AF_INET;
    in_addr.sin_port    = htons(port_int);
    res = inet_aton(host.c_str(), &in_addr.sin_addr);
    if (!res){
        PERROR("Cannot parse addr: '%s'\n", host.c_str());
        goto fail;
    }

    res = sendto(
        fd,
        buffer + sizeof(struct ipnoisehdr),
        len - sizeof(struct ipnoisehdr),
        0,              // flags
        (const struct sockaddr *)&in_addr,
        sizeof(in_addr)
    );

    PDEBUG(5, "fd: '%d', sendto err: '%d'\n", fd, err);

    if (res <= 0){
        goto fail;
    }

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

