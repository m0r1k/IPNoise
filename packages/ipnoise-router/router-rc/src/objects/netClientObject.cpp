/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev May 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include <ipnoise-common/ipnoise.h>

#include "objects/netClientObject.h"

NetClientObject * NetClientObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NetClientObject(a_node, a_doc, a_tagname);
}

NetClientObject::NetClientObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   IoObject(a_node, a_doc, a_tagname)
{
    _bev = NULL;

    // do remove us after exit
    // (will be called NetClientObject::expired)
    setExpired("0");

    use_hostos              = false;
    partial_read_cb         = NULL;
    partial_read_cb_ctx     = NULL;
    write_cb                = NULL;
    write_cb_ctx            = NULL;
    connected_cb            = NULL;
    connected_cb_ctx        = NULL;
    connect_closed_cb       = NULL;
    connect_closed_cb_ctx   = NULL;

    setState("init");

    setAttributeSafe("timeout_read",    0);
    setAttributeSafe("timeout_write",   10);
};

NetClientObject::~NetClientObject()
{
};

void NetClientObject::registrySignals()
{
    getDocument()->registrySignal(
        "client_closed",
        "ipn_client"
    );
}

void NetClientObject::close()
{
    string state = "";

    state = getState();
    if ("closed" == state){
        goto out;
    }

    if (_bev){
        bufferevent_disable(_bev, EV_READ|EV_WRITE|EV_PERSIST);
        bufferevent_free(_bev);
        _bev = NULL;
    }

    if (connect_closed_cb){
        connect_closed_cb(
            this,
            0,                  // current_offset,
            content.size(),     // current_length,
            connect_closed_cb_ctx
        );
    };

    // mark as closed
    setState("closed");

out:
    return;
}

int NetClientObject::accept(
    evutil_socket_t     socket,
    struct sockaddr     *,      // sa
    int)                        // socklen
{
    int err = 0;

    // Create buffer event object
    _bev = bufferevent_socket_new(
        getDocument()->getEvBase(),
        socket,
        BEV_OPT_CLOSE_ON_FREE
    );
    if (!_bev){
        PERROR_OBJ(this, "%s\n",
            "Cannot create bufferevent_socket_new");
        goto fail;
    }

    bufferevent_setcb(
        _bev,
        NetClientObject::_readCb,
        NetClientObject::_writeCb,
        NetClientObject::_eventCb,
        (void *)this
    );

    bufferevent_enable(_bev, EV_READ|EV_PERSIST);

    setState("accepted");

out:
    return err;
fail:
    goto out;
}

void NetClientObject::shutdown()
{
    // will be called when app going shutdown
    close();
}

string NetClientObject::getState()
{
    return getAttribute("state");
}

string NetClientObject::getDstAddr()
{
    return getAttribute("dst_addr");
}

string NetClientObject::getDstPort()
{
    return getAttribute("dst_port");
}

bool NetClientObject::isConnected()
{
    bool ret = false;

    const string state = getAttribute("state");
    if (    ("connected" == state)
        ||  ("accepted"  == state))
    {
        ret = true;
    }

    return ret;
}

void NetClientObject::setState(
    const string &a_new_state)
{
    string cur_state = getAttribute("state");

    PDEBUG(10, "net client (fd: '%d'), set state: '%s'\n",
        _bev ? bufferevent_getfd(_bev) : -1,
        a_new_state.c_str()
    );

    if (cur_state == a_new_state){
        // nothing changed
        goto out;
    }

    setAttributeSafe("state", a_new_state);

    if ("init" == a_new_state){
    } else if ("closed" == a_new_state){
        getDocument()->emitSignal(
            "client_closed",
            this
        );
    } else if ("connecting" == a_new_state){
    } else if ("connected"  == a_new_state){
    } else if ("accepted"   == a_new_state){
    } else {
        PERROR_OBJ(this,
            "Attempt to set invalid state: '%s'\n",
            a_new_state.c_str()
        );
        assert(0);
    }

out:
    return;
}

// partial read callback
void NetClientObject::setPartialReadCb(
    void (*_partial_read_cb)(
        NetClientObject         *client,
        size_t                  current_offset,
        size_t                  current_length,
        void                    *ctx
    ),
    void *_partial_read_cb_ctx)
{
    partial_read_cb     = _partial_read_cb;
    partial_read_cb_ctx = _partial_read_cb_ctx;
}

// write callback
void NetClientObject::setWriteCb(
    void (*_write_cb)(
        NetClientObject         *client,
        void                    *ctx
    ),
    void *_write_cb_ctx)
{
    write_cb     = _write_cb;
    write_cb_ctx = _write_cb_ctx;
}

// connection established callback
void NetClientObject::setConnectedCb(
    void (*_connected_cb)(
        NetClientObject         *client,
        size_t                  current_offset,
        size_t                  current_length,
        void                    *ctx
    ),
    void *_connected_cb_ctx)
{
    connected_cb      = _connected_cb;
    connected_cb_ctx  = _connected_cb_ctx;
}

// connection closed callback
void NetClientObject::setConnectClosedCb(
    void (*_connect_closed_cb)(
        NetClientObject         *client,
        size_t                  current_offset,
        size_t                  current_length,
        void                    *ctx
    ),
    void *_connect_closed_cb_ctx)
{
    connect_closed_cb      = _connect_closed_cb;
    connect_closed_cb_ctx  = _connect_closed_cb_ctx;
}

void NetClientObject::_readCb(struct bufferevent *bev, void *ctx)
{
    assert(ctx);
    unsigned char buffer[1024]  = { 0x00 };
    NetClientObject *client     = (NetClientObject *)ctx;
    size_t current_offset       = 0;
    size_t current_length       = 0;
    int res;

    PDEBUG(20, "NetClientObject::_readCb\n");

    string state = client->getState();
    if ("closed" == state){
        goto out;
    }

    current_offset = client->content.size();
    current_length = client->content.size();

    do {
        res = bufferevent_read(
            bev,
            buffer,
            sizeof(buffer)
        );
        if (res <= 0){
            break;
        }
        // store content
        client->content.insert(client->content.end(),
            buffer, buffer + res);
    } while (res > 0);

    // calculate current read length
    current_length = (int)client->content.size() - (int)current_length;

    if (client->partial_read_cb && current_length > 0){
        client->partial_read_cb(
            client,
            current_offset,
            current_length,
            client->partial_read_cb_ctx
        );
    }

out:
    return;
}

void NetClientObject::_writeCb(
    struct bufferevent  *, // bev
    void                *ctx)
{
    assert(ctx);
    NetClientObject *client = (NetClientObject *)ctx;

    PDEBUG(20, "morik NetClientObject::_writeCb\n");

    if (client->write_cb){
        client->write_cb(
            client,
            client->write_cb_ctx
        );
    }
}

void NetClientObject::_eventCb(
    struct bufferevent  *bev    __attribute__ ((unused)),
    short               what,
    void                *ctx)
{
    assert(ctx);
    NetClientObject *client = (NetClientObject *)ctx;

    PDEBUG(20, "NetClientObject::_eventCb, what: %d\n", what);

    string state = client->getState();
    if ("closed" == state){
        goto out;
    }

    if (    (what & BEV_EVENT_CONNECTED)
        &&  ("connecting" == state))
    {
        // we are connected
        PDEBUG(20, "client->setState(\"connected\")\n");
        client->setState("connected");
        if (client->connected_cb){
            client->connected_cb(
                client,
                0, // current_offset,
                0, // current_length,
                client->connected_cb_ctx
            );
        };
    }

    if (    (what & BEV_EVENT_EOF)
        &&  (what & BEV_EVENT_READING))
    {
        // connection was closed
        PERROR_OBJ(client, "connection was closed by remote part\n");
        goto do_close_conn;
    }

    if (what & BEV_EVENT_ERROR){
        // connection was closed
        PERROR_OBJ(client, "connection was closed because of error\n");
        goto do_close_conn;
    }

    if (what & BEV_EVENT_TIMEOUT){
        // connection was closed
        PERROR_OBJ(client, "connection was closed because of timeout\n");
        PDEBUG(20, "timeout!\n");
        goto do_close_conn;
    }

out:
    return;

do_close_conn:
    client->close();
    goto out;
}

int NetClientObject::open_socket(
    int     domain,
    int     type,
    int     protocol)
{
    int         res;
    int         optval  = 1;
    socklen_t   optlen  = sizeof(optval);
    int         fd      = -1;

    fd = socket(domain, type, protocol);
    if (fd < 0){
        PERROR_OBJ(this, "Cannot open socket,"
            " domain: '%d'"
            " type: '%d'"
            " protocol: '%d'"
            "\n",
            domain,
            type,
            protocol
        );
        goto fail;
    }

    res = evutil_make_socket_nonblocking(fd);
    if (res < 0){
        PERROR_OBJ(this, "%s\n", "Cannot set socket flag: 'nonblocking'\n");
        goto fail;
    }

    res = evutil_make_socket_closeonexec(fd);
    if (res < 0){
        PERROR_OBJ(this, "%s\n", "Cannot set socket flag: 'closeonexec'\n");
        goto fail;
    }

    res = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&optval, optlen);
    if (res < 0){
        PERROR_OBJ(this, "%s\n", "Cannot set socket flag: 'keepalive'\n");
        goto fail;
    }

    res = getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&optval, &optlen);
    if (res < 0){
        PERROR_OBJ(this, "%s\n", "Cannot get socket flag: 'keepalive'\n");
        goto fail;
    }
    if (not optval){
        PERROR_OBJ(this, "%s\n", "Cannot set socket flag: 'keepalive'\n");
        goto fail;
    }

out:
    return fd;
fail:
    if (fd >= 0){
        ::close(fd);
        fd = -1;
    }
    goto out;
}

#ifdef HOSTOS
void NetClientObject::useHostOS(const bool &val)
{
    use_hostos = val;
}
#endif

int NetClientObject::connect(string host, int port)
{
    char buffer[1024]           = { 0x00 };
    int res, err                = -1;
    int fd                      = -1;

    struct addrinfo hints, *servinfo, *p;
    int rv;

    if (_bev){
        PERROR_OBJ(this, "Connection to '%s' port '%d' already open\n",
            host.c_str(), port);
        goto out;
    }

    // clear
    memset(&hints, 0, sizeof hints);
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;

    snprintf(buffer, sizeof(buffer), "%d", port);
    rv = getaddrinfo(host.c_str(), buffer, &hints, &servinfo);
    if (rv != 0) {
        PERROR_OBJ(this, "Cannot resolve host: '%s' while connect"
            " to '%s' port '%d'\n",
            host.c_str(), host.c_str(), port);
        goto fail;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if (use_hostos){
            // open HOSTOS socket
            fd = open_socket(
                PF_HOSTOS,
                p->ai_socktype,
                p->ai_protocol
            );
        } else {
            // open local socket
            fd = open_socket(
                p->ai_family,
                p->ai_socktype,
                p->ai_protocol
            );
        }

        if (fd < 0){
            PERROR_OBJ(this, "Cannot open socket for connect"
               " to '%s' port '%d'\n",
               host.c_str(), port
            );
            continue;
        }

        // create buffer event
        _bev = bufferevent_socket_new(
            getDocument()->getEvBase(),
            fd,
            BEV_OPT_CLOSE_ON_FREE
        );

        // setup callbacks
        bufferevent_setcb(
            _bev,
            NetClientObject::_readCb,
            NetClientObject::_writeCb,
            NetClientObject::_eventCb,
            (void *)this
        );

        // enable events
        bufferevent_enable(_bev, EV_READ|EV_WRITE|EV_PERSIST);

        // setup timeouts
        {
            struct timeval timeout_read;
            struct timeval timeout_write;

            timeout_read.tv_sec     = getAttributeInt("timeout_read");
            timeout_read.tv_usec    = 0; // TODO timeout_read must be float
            timeout_write.tv_sec    = getAttributeInt("timeout_write");
            timeout_write.tv_usec   = 0; // TODO timeout_write must be float

            bufferevent_set_timeouts(_bev,
                &timeout_read,
                &timeout_write
            );
        }

        // trying to connect
        res = bufferevent_socket_connect(
            _bev,
            p->ai_addr,
            p->ai_addrlen
        );
        if (res < 0){
            PERROR_OBJ(this, "Connection to '%s' port '%d' failed\n",
                host.c_str(), port);
            goto fail;
        }

        res = evutil_make_socket_nonblocking(fd);
        if (res < 0){
            PERROR_OBJ(this, "%s\n", "Cannot set socket flag: 'nonblocking'\n");
            goto fail;
        }

        // store destination address
        setAttributeSafe("dst_addr", host.c_str());

        // store destination port
        snprintf(buffer, sizeof(buffer), "%d", port);
        setAttributeSafe("dst_port", buffer);

        // start connecting
        setState("connecting");

        // don't forget
        break;
    }

    // all done with this structure
    freeaddrinfo(servinfo);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    if (fd > 0){
        ::close(fd);
        fd = -1;
    }
    goto out;
}

int NetClientObject::haveSoError()
{
    bool ret = true;

    int         err = 0;
    socklen_t   len = sizeof(err);
    int         res = 0;
    int         fd  = bufferevent_getfd(_bev);

    res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (res){
        PERROR_OBJ(this, "getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len)"
            " failed\n");
        goto out;
    }

    if (!err){
        // all ok
        ret = false;
    }

out:
    return ret;
}

ssize_t NetClientObject::write(const string &a_buffer)
{
    int     fd      = -1;
    ssize_t res     = 0;
    ssize_t wrote   = 0;

    if (not _bev){
        goto out;
    }

    fd = bufferevent_getfd(_bev);

    if (not a_buffer.size()){
        goto out;
    }

    do {
        const string &data = a_buffer.substr(wrote);
        res = (ssize_t)::write(
            fd,
            data.c_str(),
            data.size()
        );
        if (res < 0){
            if (EINTR == errno){
                continue;
            }
            break;
        } else if (!res){
            break;
        }
        wrote += res;
    } while (res > 0);

    if (wrote){
        addTxBytes(wrote);
        fsync(fd);
    }

out:
    return wrote;
};

ssize_t NetClientObject::send_answer(const string &buffer)
{
    ssize_t err = -1;

    err = write(buffer);
    close();
    return err;
}

