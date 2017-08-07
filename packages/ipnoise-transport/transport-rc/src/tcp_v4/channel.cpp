#include <assert.h>

#include "tcp_v4/tcp_v4.hpp"
#include "tcp_v4/channel.hpp"

// IPNoise kernel support
#include <linux/if_tun.h>
#include <ipnoise-common/ipnoise.h>

#include "server.hpp"

#define RD_BUFFER_SIZE 65535

Channel::Channel(struct event_base *base, TcpV4Transport *transport)
{
    _base       = base;
    _transport  = transport;
    _bev        = NULL;
    _dst_addr   = "";

    setState("UNCONNECTED");

    tx_queue = allocate_packet();
    if (!tx_queue){
        PERROR("Cannot allocate packet\n");
        goto fail;
    }

    INIT_LIST_HEAD(&tx_queue->list);

out:
    return;
fail:
    goto out;
};

Channel::~Channel()
{
    if (_bev){
        bufferevent_free(_bev);
        _bev = NULL;
    }
    while (!list_empty(&tx_queue->list)){
        struct list_head    *first  = NULL;
        QueuePacket         *packet = NULL;

        // ok, list is not empty
        first   = tx_queue->list.next;
        packet  = list_entry(first, QueuePacket, list);

        // remove from queue
        list_del(first);

        free_packet(packet);
    }
    if (tx_queue){
        free_packet(tx_queue);
        tx_queue = NULL;
    }
};

string Channel::getState()
{
    return _state;
}

int Channel::isConnected()
{
    return _state == "CONNECTED";
}

int Channel::isUnConnected()
{
    return _state == "UNCONNECTED";
}

int Channel::isConnecting()
{
    return _state == "CONNECTING";
}

int Channel::isError()
{
    return _state == "ERROR";
}

int Channel::isClosed()
{
    return _state == "CLOSED";
}

void Channel::setState(const string &new_state)
{
    if (_state == new_state){
        goto out;
    }
    if (    "CONNECTED"    !=  new_state
        &&  "CONNECTING"   !=  new_state
        &&  "UNCONNECTED"  !=  new_state
        &&  "CLOSED"       !=  new_state
        &&  "ERROR"        !=  new_state)
    {
        PERROR("Unsupported state: '%s'\n", new_state.c_str());
        goto fail;
    }

    _state = new_state;

    if (    "ERROR"   == _state
        ||  "CLOSED"  == _state)
    {
        if (_bev){
            bufferevent_free(_bev);
            _bev = NULL;
        }
    }

    if ("CONNECTED" == new_state){
        flushTxQueue();
    }

out:
    return;
fail:
    goto out;
}

TcpV4Transport * Channel::getTransport()
{
    return _transport;
}

void Channel::readcb(struct bufferevent *bev, void *ctx)
{
    PDEBUG(5, "read callback\n");

    Channel *channel = (Channel *)ctx;
    assert (channel != NULL);

    TcpV4Transport *transport = channel->getTransport();
    assert (transport != NULL);

    Server *server = transport->getServer();
    assert(server != NULL);

    struct ipnoisehdr *hdr = NULL;
    int res;
    int buffer_size = RD_BUFFER_SIZE + sizeof(struct ipnoisehdr);
    char buffer[buffer_size + 1];
    memset(buffer, 0x00, buffer_size);

    hdr = (struct ipnoisehdr *)buffer;
    ipnoise_hdr_init(hdr);

    do {
        res = bufferevent_read(
            bev,
            buffer      + sizeof(struct ipnoisehdr),
            buffer_size - sizeof(struct ipnoisehdr)
        );
        if (res > 0){
            buffer[res + sizeof(struct ipnoisehdr)] = 0x00;
            // fill source addr info
            snprintf((char *)hdr->h_source, sizeof(hdr->h_source),
                "%s", channel->_dst_addr.c_str()
            );
            PDEBUG(5, "was received packed from: '%s', %d byte(s)\n",
                hdr->h_source,
               res
            );
            // start rx packet
            server->netif_rx(buffer, res + sizeof(struct ipnoisehdr));
        }
    } while (res > 0);
}

void Channel::flushTxQueue()
{
    int res;

    res = isConnected();
    if (!res){
        goto out;
    }

    while (!list_empty(&tx_queue->list)){
        struct list_head    *first  = NULL;
        QueuePacket         *packet = NULL;

        // ok, list is not empty
        first   = tx_queue->list.next;
        packet  = list_entry(first, QueuePacket, list);

        // remove from queue
        list_del(first);

        _start_xmit(
            packet->buffer,
            packet->len
        );

        free_packet(packet);
    }

out:
    return;
}

void Channel::eventcb(
    struct bufferevent  *,  // bev
    short               what,
    void                *ctx)
{
    Channel *channel = (Channel *)ctx;
    assert (channel != NULL);

    if (what & BEV_EVENT_CONNECTED){
        PDEBUG(5, "MORIK we are connected\n");
        channel->setState("CONNECTED");
    }

    if (what & BEV_EVENT_ERROR){
        channel->setState("ERROR");
    }

    if (what & BEV_EVENT_EOF){
        channel->setState("CLOSED");
    }

    PDEBUG(5, "event callback, what: '%d'\n", what);
}

/*
 * For create new connection
 */
int Channel::do_init(const char *dst_addr)
{
    int res, err    = -1;
    string daddr    = dst_addr;
    string host     = "";
    string port     = "";
    int port_int    = 0;
    size_t pos      = string::npos;

    // store destination addr
    _dst_addr = daddr;

    // split dst_addr by host and port
    pos = daddr.find(":");
    if (pos == string::npos){
        PERROR("Cannot parse: '%s' on host and port\n",
            daddr.c_str());
        goto fail;
    }

    // clean sockaddr_in
    memset(&connect_to_addr, 0x00, sizeof(connect_to_addr));

    // get destination addresses
    host        = daddr.substr(0, pos);
    port        = daddr.substr(pos + 1);
    port_int    = atoi(port.c_str());

    // store connect addr in sockaddr_in
    connect_to_addr.sin_family  = AF_HOSTOS;
    connect_to_addr.sin_port    = htons(port_int);
    res = inet_aton(host.c_str(), &connect_to_addr.sin_addr);
    if (res != 1){
        err = -1;
        PERROR("inet_aton failed for: '%s'\n", host.c_str());
        goto fail;
    }

    _bev = bufferevent_socket_new(
        _base,
        -1,
        BEV_OPT_CLOSE_ON_FREE
    );
    if (!_bev){
        PERROR("Cannot create bufferevent_socket_new\n");
        goto fail;
    }

    res = bufferevent_socket_connect(
        _bev,
        (struct sockaddr*)&connect_to_addr,
        sizeof(connect_to_addr)
    );
    if (res < 0){
        err = -1;
        PERROR("bufferevent_socket_connect failed for: '%s:%d'\n",
            host.c_str(), port_int);
        goto fail;
    }

    bufferevent_setcb(
        _bev,
        Channel::readcb,
        NULL,
        Channel::eventcb,
        (void *)this
    );

    bufferevent_enable(_bev, EV_READ|EV_WRITE|EV_PERSIST);
    setState("CONNECTING");

    PDEBUG(5, "trying to connect to %s:%s\n", host.c_str(), port.c_str());

    // all ok
    err = 0;

out:
    return err;

fail:
    if (_bev){
        bufferevent_free(_bev);
        _bev = NULL;
    }
    goto out;
}

/*
 * For accepted connections
 */
int Channel::do_init(struct bufferevent *bev, char *dst_addr)
{
    int err = -1;

    struct ipnoisehdr *hdr = NULL;
    int buffer_size = RD_BUFFER_SIZE + sizeof(struct ipnoisehdr);
    char buffer[buffer_size + 1];

    // store buffer event
    assert(bev);
    _bev = bev;

    // store our address
    assert(dst_addr);
    _dst_addr = dst_addr;

    // clear buffers
    memset(buffer, 0x00, buffer_size);

    // init IPNoise header
    hdr = (struct ipnoisehdr *)buffer;
    ipnoise_hdr_init(hdr);

    // setup buffer event
    bufferevent_setcb(_bev,
        Channel::readcb,
        NULL,
        Channel::eventcb,
        (void *)this
    );
    bufferevent_enable(_bev, EV_READ|EV_WRITE|EV_PERSIST);

    // mark channel as "connected"
    setState("CONNECTED");

    // all ok
    err = 0;

    return err;
}

void Channel::free_packet(QueuePacket *packet)
{
    if (!packet){
        goto out;
    }
    if (packet->buffer){
        free(packet->buffer);
        packet->buffer = NULL;
    }
    if (packet){
        free(packet);
        packet = NULL;
    }
out:
    return;
}

QueuePacket * Channel::allocate_packet()
{
    QueuePacket *packet = NULL;

    packet = (QueuePacket *)malloc(sizeof(*packet));
    if (!packet){
        PERROR("Cannot allocate memory, was needed '%d\n",
            sizeof(*packet));
        goto fail;
    }
    memset(packet, 0x00, sizeof(*packet));

out:
    return packet;
fail:
    if (packet){
        free(packet);
        packet = NULL;
    }
    goto out;
}

QueuePacket * Channel::queue_tx_packet(
    const char  *buffer,
    int         len)
{
    QueuePacket *packet = NULL;

    packet = allocate_packet();
    if (!packet){
        PERROR("Cannot allocate packet\n");
        goto fail;
    }
    packet->len     = len;
    packet->buffer  = (char *)malloc(packet->len);
    if (!packet->buffer){
        PERROR("Cannot allocate memory, was needed '%d\n",
            packet->len);
        goto fail;
    }

    // copy data
    memcpy(packet->buffer, buffer, packet->len);

    // queue packet
    list_add_tail(
        &packet->list,
        &tx_queue->list
    );

out:
    return packet;
fail:
    if (packet){
        free_packet(packet);
        packet = NULL;
    }
    goto out;
}

int Channel::_start_xmit(const char *buffer, int len)
{
    int res, err = -1;
    int fd = -1;

    res = isConnected();
    if (!res){
        goto fail;
    }

    fd = bufferevent_getfd(_bev);
    PDEBUG(5, "channel fd: '%d', state: '%s'\n",
        fd, getState().c_str());

    if (fd < 0){
        setState("ERROR");
        goto fail;
    }

    err = write(fd, buffer, len);
    PDEBUG(5, "channel err: '%d', state: '%s'\n",
        err, getState().c_str());
    if (err <= 0){
        // do not mart socket as "error" here,
        // err maybe -EGAIN, so it is not fatal error
        goto fail;
    }

    // all ok
    err = 0;

out:
    PDEBUG(5, "channel write: '%d', state: '%s'\n",
        err, getState().c_str());
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int Channel::start_xmit(const char *buffer, int len)
{
    int err = 0;

    queue_tx_packet(buffer, len);
    flushTxQueue();

    return err;
}

