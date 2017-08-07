#include "class.hpp"
DEFINE_CLASS(Thread);

#ifndef THREAD_HPP
#define THREAD_HPP

#include <QObject>
#include <QThread>
#include <string>

#include "log.hpp"
#include "context.hpp"

class Network;

using namespace std;

class Thread
    :   public  QThread
{
    Q_OBJECT

    public:
        Thread(
            Network         *a_net,
            const string    &a_name
        );
        virtual ~Thread();

        enum ThreadState {
            THREAD_STATE_ERROR_CANNOT_SHUTDOWN  = -4,
            THREAD_STATE_ERROR_CANNOT_START     = -3,
            THREAD_STATE_ERROR_CANNOT_INIT      = -2,
            THREAD_STATE_ERROR                  = -1,
            THREAD_STATE_NOT_INITED             = 0,
            THREAD_STATE_INIT,
            THREAD_STATE_INITED,
            THREAD_STATE_RUNNING,
            THREAD_STATE_SHUTDOWN
        };

        string              getName();
        Thread::ThreadState getState();
        void                setState(Thread::ThreadState);

        Network *           getNetwork();

        // context
        void                _setContext(ContextSptr);
        ContextSptr         _getContext();
        void                _resetContext();

        static ContextSptr  getContext();
        static void         setContext(ContextSptr);
        static void         resetContext();

    signals:
        void stateChanged(Thread *, Thread::ThreadState);

    protected:
        void run();

        // interface
        virtual int32_t do_init()  = 0;
        virtual void    do_start() = 0;
        virtual void    do_stop()  = 0;

    private:
        Network         *m_net;
        string          m_name;
        ThreadState     m_state;
        ContextSptr     m_context;
};

#endif

