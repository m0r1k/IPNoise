#include <stdio.h>
#include <string.h>

// stl
#include <string>

// IPNoise kernel support
#include <linux/socket.h>

using namespace std;

class Server;

#ifndef BASE_TRANSPORT_HPP
#define BASE_TRANSPORT_HPP

class BaseTransport
{
    public:
        BaseTransport(struct event_base *base);
        virtual ~BaseTransport();


        virtual int     do_init()                               = 0;
        virtual string  getName()                               = 0;
        virtual int     start_xmit(const char *buffer, int len) = 0;

        Server              *   getServer();
        void                    setServer(Server *server);
        struct event_base   *   getEventBase();

        virtual string  getBindAddrPort();
        virtual int     setBindAddrPort(const string &);
        virtual int     setBindAddr(const string &);
        virtual int     setBindPort(int);
        virtual string  getBindAddr();
        virtual int     getBindPort();

    private:
        struct event_base   *m_base;
        string              m_bind_addr;
        int                 m_bind_port;
        Server              *m_server;
};

#endif

