#include "utils.hpp"
#include "thread/worker/http.hpp"

#include "http_server.hpp"

HttpServer::HttpServer(
    Network *a_net,
    QObject *a_parent)
    :   QTcpServer(a_parent)
{
    m_net       = a_net;
    m_thread    = NULL;

    setMaxPendingConnections(1);
}

void HttpServer::incomingConnection(
    int     a_fd)
{
    char                buffer[65535]   = { 0x00 };
    ThreadWorkerHttp    *thread_worker  = NULL;

    if (0 > a_fd){
        goto out;
    }

    // create name
    snprintf(buffer, sizeof(buffer),
        "worker_%d",
        a_fd
    );


    PWARN("HttpServer::incomingConnection, fd: '%d'\n",
        a_fd
    );

    if (!m_thread){
        m_thread = new ThreadHttp(m_net, buffer);
        CHECKED_CONNECT(
            m_thread, SIGNAL(finished()),
            m_thread, SLOT(deleteLater())
        );
        m_thread->start();
    }

    thread_worker = new ThreadWorkerHttp(
        m_net,
        a_fd,
        buffer // name
    );

    thread_worker->moveToThread(m_thread);

    CHECKED_CONNECT(
        this,           SIGNAL(fireHttpThread()),
        thread_worker,  SLOT(processSlot())
    );

    emit fireHttpThread();

out:
    return;
}

