#include "class.hpp"
DEFINE_CLASS(ThreadWorkerHttp);

#ifndef THREAD_WORKER_HTTP_HPP
#define THREAD_WORKER_HTTP_HPP

#include <stdint.h>

#include <QAbstractSocket>
#include <QTcpServer>
#include <QObject>
#include <QStringList>
#include <QThread>

#include <QSocketNotifier>
DEFINE_CLASS(QSocketNotifier);

#include <string>

#include "http_request.hpp"
#include "thread.hpp"
#include "core/neuron/neuron/main.hpp"

using namespace std;

class ThreadWorkerHttp
    :   public QObject
{
    Q_OBJECT

    signals:
        void error(QTcpSocket::SocketError socketError);

    public:
        ThreadWorkerHttp(
            Network         *a_net,
            int32_t         a_fd,
            const string    &a_name   = "thread_http_worker",
            QObject         *a_parent = 0
        );
        virtual ~ThreadWorkerHttp();

//        void processFd();

        static NeuronSessionSptr createSession(
            HttpAnswerSptr,
            NeuronUserSptr
        );

        static PropMapSptr getParams(
            HttpAnswerSptr  a_answer,
            PropMapSptr     a_req_params
        );

        // error codes
        static void setError(
            HttpAnswerSptr,
            const int32_t   &a_status_code,
            const string    &a_status_string
        );
        static void set302(
            HttpAnswerSptr,
            const string    &a_location,
            const string    &a_status_string = "Found"
        );
        static void set404(
            HttpAnswerSptr,
            const string &a_status_string = "Not found"
        );
        static void set500(
            HttpAnswerSptr,
            const string &a_status_string = "Internal error"
        );

        // api
        void apiLogout(HttpAnswerSptr);
        void    apiRegister(
            HttpAnswerSptr,
            const PropMapSptr
        );
        void    apiLogin(
            HttpAnswerSptr,
            const PropMapSptr
        );

    public slots:
        void    readSlot(int);
        void    exceptionSlot(int);
        void    stateChangedSlot(
            QAbstractSocket::SocketState
        );
        void    processSlot();
        void    disconnectedSlot();

    protected:
        int32_t     willRedirect(
            HttpAnswerSptr  a_answer,
            PropMapSptr     a_req_api
        );
        NeuronSptr  getCurNeuron(const HttpRequestSptr);

        void        processPost(
            HttpAnswerSptr,
            PropMapSptr
        );
        void        processGet(
            HttpAnswerSptr,
            PropMapSptr
        );

        Network *   getNetwork();
        void        setContext(ContextSptr);
        void        setupContext(
            HttpAnswerSptr  a_answer,
            PropMapSptr     a_req_api
        );
        void        processRequests(const HttpRequestSptr);
        void        processRequest(
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props,
            PropMapSptr         a_req_api
        );

        string      getName();

    private:
        string              m_name;
        QSocketNotifierSptr m_socket_notifier;
        int32_t             m_fd;
        Network             *m_net;
        HttpRequestSptr     m_req;
};

#endif

