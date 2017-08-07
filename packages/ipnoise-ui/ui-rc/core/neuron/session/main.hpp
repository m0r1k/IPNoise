#include "class.hpp"
DEFINE_CLASS(NeuronSession);

#ifndef NEURON_SESSION_HPP
#define NEURON_SESSION_HPP

#include <stdio.h>

#include <QThread>
#include <map>
#include <string>
#include <memory>

#include <QSocketNotifier>
DEFINE_CLASS(QSocketNotifier);

#include "prop/map.hpp"
#include "prop/int.hpp"
#include "prop/string.hpp"
#include "prop.hpp"
#include "prop/neuron/type.hpp"
#include "http_answer.hpp"

#include "core/neuron/neuron/main.hpp"
#include "core/neuron/user/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

using namespace std;

class NeuronSession
    :   public  SUPER_CLASS
{
    Q_OBJECT

    public:
        NeuronSession();
        virtual ~NeuronSession();

        DEFINE_PROP(CurNeuronId,    PropNeuronIdSptr);
        DEFINE_PROP(Events,         PropVectorSptr);

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic
        virtual int32_t     do_autorun();
        NeuronUserSptr      getUser();
        void                setCurNeuron(NeuronSptr);
        NeuronSptr          getCurNeuron();
        NeuronUserSptr      unlinkFromRegisteredUsers();

        void                addEvent(PropMapSptr);
        void                clearEvents();
        PropVectorSptr      getClearEvents();
        void                flushEvents();

        void                setEventFd(int32_t);
        int32_t             getEventFd();
//        void                closeEventSocket();

//        void setEventSocket(TcpSocketSptr);

        // static
        static int32_t      init();
        static void         destroy();
        static NeuronSptr   object_create();

    protected slots:
        void eventSockExceptionSlot(int);

    private:
        void                do_init();
        int32_t             m_event_fd;
        QSocketNotifierSptr m_event_socket_notifier;
};

#endif

