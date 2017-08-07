#include "class.hpp"

#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <stdint.h>

#include <QTcpServer>
#include <QObject>
#include <QStringList>

#include "thread/http.hpp"

class Network;

class HttpServer
    :   public QTcpServer
{
    Q_OBJECT

    public:
        HttpServer(
            Network *,
            QObject *a_parent = 0
        );

    signals:
        void fireHttpThread();

    protected:
        virtual void incomingConnection(int);

    private:
        Network     *m_net;
        ThreadHttp  *m_thread;
};

#endif

