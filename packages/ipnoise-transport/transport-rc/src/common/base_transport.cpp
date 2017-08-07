#include <stdlib.h>

#include "common/base_transport.hpp"

BaseTransport::BaseTransport(
    struct event_base *a_base)
{
    m_base       = a_base;
    m_server     = NULL;
    m_bind_addr  = "";
    m_bind_port  = 0;
};

BaseTransport::~BaseTransport()
{

}

void BaseTransport::setServer(Server *a_server)
{
    m_server = a_server;
}

Server * BaseTransport::getServer()
{
    return m_server;
}

struct event_base *BaseTransport::getEventBase()
{
    return m_base;
}

string BaseTransport::getBindAddrPort()
{
    char buffer[1024] = { 0x00 };

    snprintf(buffer, sizeof(buffer),
        "%s:%d",
        m_bind_addr.c_str(),
        m_bind_port
    );

    return buffer;
}

int BaseTransport::setBindAddrPort(const string &a_bind_addr_port)
{
    string::size_type pos;
    string  bind_addr_str;
    string  bind_port_str;

    pos = a_bind_addr_port.find(":");
    if (string::npos != pos){
        bind_addr_str = a_bind_addr_port.substr(0, pos);
        bind_port_str = a_bind_addr_port.substr(pos + 1);
    }

    if (bind_addr_str.size()){
        setBindAddr(bind_addr_str);
    }

    if (bind_port_str.size()){
        setBindPort(atoi(bind_port_str.c_str()));
    }

    return 0;
}

int BaseTransport::setBindAddr(const string &a_bind_addr)
{
    m_bind_addr = a_bind_addr;
    return 0;
}

int BaseTransport::setBindPort(int a_bind_port)
{
    m_bind_port = a_bind_port;
    return 0;
}

string BaseTransport::getBindAddr()
{
    return m_bind_addr;
}

int BaseTransport::getBindPort()
{
    return m_bind_port;
}

