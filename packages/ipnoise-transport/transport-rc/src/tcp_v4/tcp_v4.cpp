#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

// IPNoise kernel support
#include <linux/if_tun.h>
#include <linux/socket.h>
#include <ipnoise-common/ipnoise.h>

#include "tcp_v4/tcp_v4.hpp"
#include "tcp_v4/channel.hpp"

#define DEFAULT_BIND_IP     "0.0.0.0"
#define DEFAULT_BIND_PORT   22102

TcpV4Transport::TcpV4Transport(struct event_base *base)
    : BaseTransport(base)
{
    listener = NULL;
    setBindAddr(DEFAULT_BIND_IP);
    setBindPort(DEFAULT_BIND_PORT);
};

TcpV4Transport::~TcpV4Transport()
{
    server_stop();
};

string TcpV4Transport::getName()
{
    string ret = "tcp_v4";
    return ret;
}

void TcpV4Transport::readcb(
    struct bufferevent  *, // bev
    void                *ctx)
{
    TcpV4Transport *transport  = (TcpV4Transport *)ctx;
    assert(transport != NULL);

    PDEBUG(5, "new read!\n");
}

void TcpV4Transport::eventcb(
    struct bufferevent  *,  // bev
    short,                  // what
    void                *ctx)
{
    TcpV4Transport *transport  = (TcpV4Transport *)ctx;
    assert(transport != NULL);

    PDEBUG(5, "new event!\n");
}

void TcpV4Transport::acceptSignalSocket(
    struct evconnlistener   *,  // listener
    evutil_socket_t         socket,
    struct sockaddr         *sa,
    int,                    // socklen
    void                    *ctx)
{
    // new accept
    PWARN("MORIK NEW_ACCEPT!\n");

    int res;
    char dst_addr[1024]             = { 0x00 };
    Channel             *channel    = NULL;
    TcpV4Transport      *transport  = NULL;
    struct bufferevent  *bev_in     = NULL;
    struct sockaddr_in  *from       = (struct sockaddr_in *)sa;

    CHANNELS::iterator channels_i;

    // fill source addr info
    snprintf(dst_addr, sizeof(dst_addr),
        "%s:%d",
        inet_ntoa(from->sin_addr),
        ntohs(from->sin_port)
    );

    PDEBUG(5, "new client was accepted from: '%s'\n", dst_addr);

    // get transport
    transport = (TcpV4Transport *)ctx;
    assert(transport != NULL);

    // do accept
    bev_in = bufferevent_socket_new(
        transport->getEventBase(),
        socket,
        BEV_OPT_CLOSE_ON_FREE
        //|BEV_OPT_DEFER_CALLBACKS
    );
    if (!bev_in){
        goto fail;
    }

    // TODO ??? how about channel what already exist,
    // example:
    // client "A" have created new channel to "192.168.1.1:2210",
    // now client "B" create new channel from "192.168.1.1:2210",
    // client "A" have accept channel, channel name is "192.168.1.1:2210" too,
    // so we have double channels with some name..
    // now I will ignore client "B" in this cases, but need think about it
    // 20110104 morik

    // search already open channel with some name
    channels_i = transport->channels.find(dst_addr);
    if (channels_i != transport->channels.end()){
        PWARN("New client was accepted from: '%s', but we already"
            " have open channel with some name, client will be droped\n",
            dst_addr
        );
        goto fail;
    }

    // create new channel
    PDEBUG(5, "MORIK create new channel\n");
    channel = new Channel(transport->getEventBase(), transport);
    if (!channel){
        PERROR("Cannot create new channel\n");
        goto fail;
    }

    // init new channel
    res = channel->do_init(bev_in, dst_addr);
    if (res){
        PERROR("Cannot init channel by addr: '%s'\n", dst_addr);
        goto fail;
    }

    // all ok, store transport
    transport->channels[dst_addr] = channel;

out:
    return;

fail:
    if (channel){
        delete channel;
        channel = NULL;
    }
    if (bev_in){
        bufferevent_free(bev_in);
        bev_in = NULL;
    }
    goto out;
}


int TcpV4Transport::server_start()
{
    int err = -1;
    struct sockaddr_in server;
    struct in_addr addr;

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

    err = inet_aton(bind_addr.c_str(), &addr);
    if (!err){
        PERROR("inet_aton failed for: '%s'\n", bind_addr.c_str());
        goto fail;
    }
    server.sin_family       = PF_INET;
    server.sin_port         = htons(bind_port);
    memcpy(&server.sin_addr, &addr.s_addr, sizeof(addr.s_addr));

    PDEBUG(5, "server.sin_family: '%d'\n", server.sin_family);
    listener = evconnlistener_hostos_new_bind(
        getEventBase(),
        TcpV4Transport::acceptSignalSocket,
        (void *)this,
        LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_EXEC|LEV_OPT_CLOSE_ON_FREE,
        -1,
        (struct sockaddr*)&server,
        sizeof(server)
    );

	if (!listener) {
		PERROR("Could not create a listener!\n");
        goto fail;
	}

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

int TcpV4Transport::server_stop()
{
    if (listener){
        evconnlistener_free(listener);
        listener = NULL;
    }

    // all ok
    PDEBUG(5, "'%s' transport was stoped successful\n", getName().c_str());

    return 0;
}

int TcpV4Transport::do_init()
{
    int res, err = -1;

    // stop current server if need
    res = server_stop();

    // start server
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
    PERROR("Cannot init '%s' transport\n", getName().c_str());

    err = -1;
    goto out;
}

int TcpV4Transport::start_xmit(const char *buffer, int len)
{
    int res, err = -1;
    CHANNELS::iterator channels_i;
    struct ipnoisehdr   *hdr        = (struct ipnoisehdr *)buffer;
    const char          *dst_addr   = (const char *)hdr->h_dest;
    string              dest        = dst_addr;
    size_t              pos         = string::npos;
    Channel             *channel    = NULL;

    // PDEBUG(5, "Need xmit to h_dest: '%s'\n", dst_addr);
    // ipnoise_hexdump ((unsigned char *)buffer, len);

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

    // search already opened channel
    channels_i = channels.find(dst_addr);
    if (channels_i == channels.end()){
        // channel not exist, create him
        PDEBUG(5, "MORIK create new channel: '%s'\n", dst_addr);
        channel = new Channel(getEventBase(), this);
        err = channel->do_init(dst_addr);
        if (err){
            PERROR("Cannot init channel by addr: '%s'\n", dst_addr);
            goto fail;
        }
        // all ok, store transport
        channels[dst_addr] = channel;
    } else {
        // channel already exist, use it
        PDEBUG(5, "MORIK use exist channel: '%s'\n", dst_addr);
        channel = channels_i->second;
    }

    // send data
    err = channel->start_xmit(
        buffer + sizeof(struct ipnoisehdr),
        len - sizeof (struct ipnoisehdr)
    );
    // PDEBUG(5, "channel->start_xmit: err: '%d'\n", err);
    if (    channel->isError()
        ||  channel->isClosed())
    {
        // looks like channel don't work, close him
        channels_i = channels.find(dst_addr);
        if (channels_i != channels.end()){
            channels.erase(channels_i);
        }
        delete channel;
        channel = NULL;
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    goto out;
}

