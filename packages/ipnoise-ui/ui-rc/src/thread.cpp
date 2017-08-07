#include <stdio.h>
#include <QMetaType>

#include "network.hpp"
#include "thread.hpp"

extern ContextSptr g_context;

Thread::Thread(
    Network         *a_net,
    const string    &a_name)
    :   QThread((QObject *)a_net)
{
    m_net       = a_net;
    m_name      = a_name;
    m_state     = Thread::THREAD_STATE_NOT_INITED;
    // register type for signals
    qRegisterMetaType<Thread::ThreadState>("Thread::ThreadState");
}

Thread::~Thread()
{
}

string Thread::getName()
{
    return m_name;
}

Thread::ThreadState Thread::getState()
{
    return m_state;
}

void Thread::run()
{
    PDEBUG(30, "Thread::run, thread name: '%s'\n",
        getName().c_str()
    );

    // init
    setState(Thread::THREAD_STATE_INIT);
    if (Thread::THREAD_STATE_ERROR >= getState()){
        PERROR("cannot init thread: '%s'\n",
            getName().c_str()
        );
        goto fail;
    }

    setState(Thread::THREAD_STATE_RUNNING);

out:
    return;
fail:
    goto out;
}

void Thread::setState(Thread::ThreadState a_state)
{
    int32_t res = 0;

    if (m_state == a_state){
        // state not changed
        goto out;
    }

    if (Thread::THREAD_STATE_INIT == a_state){
        if (Thread::THREAD_STATE_NOT_INITED != m_state){
            // ready for init?
            m_state = Thread::THREAD_STATE_ERROR_CANNOT_INIT;
//            emit stateChanged(this, m_state);
            m_net->threadStateChangedSlot(this, m_state);
            PERROR("attempt to double init thread: '%s'\n",
                getName().c_str()
            );
            goto fail;
        }
        m_state = Thread::THREAD_STATE_INIT;
        // emit stateChanged(this, m_state);
        // try to init
        res = do_init();
        if (res){
            m_state = Thread::THREAD_STATE_ERROR_CANNOT_INIT;
            m_net->threadStateChangedSlot(this, m_state);
//            emit stateChanged(this, m_state);
            PERROR("Cannot init thread: '%s', res: '%d'\n",
                getName().c_str(),
                res
            );
            goto fail;
        }
        m_state = Thread::THREAD_STATE_INITED;
        m_net->threadStateChangedSlot(this, m_state);
        // emit stateChanged(this, m_state);
    } else if (Thread::THREAD_STATE_RUNNING == a_state){
        if (Thread::THREAD_STATE_INITED != m_state){
            // read for run?
            m_state = Thread::THREAD_STATE_ERROR_CANNOT_START;
//            emit stateChanged(this, m_state);
            m_net->threadStateChangedSlot(this, m_state);
            PERROR("attempt to start not inited thread: '%s'\n",
                getName().c_str()
            );
            goto fail;
        }
        // start thread
        m_state = Thread::THREAD_STATE_RUNNING;
        m_net->threadStateChangedSlot(this, m_state);
//        emit stateChanged(this, m_state);
        // call thread
        do_start();
        m_state = Thread::THREAD_STATE_SHUTDOWN;
        m_net->threadStateChangedSlot(this, m_state);
//        emit stateChanged(this, m_state);
    } else if (Thread::THREAD_STATE_SHUTDOWN == a_state){
        if (Thread::THREAD_STATE_RUNNING != m_state){
            // ready for shutdown?
            m_state = Thread::THREAD_STATE_ERROR_CANNOT_SHUTDOWN;
            m_net->threadStateChangedSlot(this, m_state);
//            emit stateChanged(this, m_state);
            PERROR("attempt to shutdown not running thread: '%s'\n",
                getName().c_str()
            );
            goto fail;
        }
        // shutdown thread
        m_state = Thread::THREAD_STATE_SHUTDOWN;
        m_net->threadStateChangedSlot(this, m_state);
//        emit stateChanged(this, m_state);
        // call stop callback
        do_stop();
    } else {
        PERROR("unsupported state: '%d' for thread: '%s'\n",
            a_state,
            getName().c_str()
        );
        goto fail;
    }

out:
    return;
fail:
    goto out;
}

Network * Thread::getNetwork()
{
    return m_net;
}

// ---------------- context ----------------

void Thread::_setContext(
    ContextSptr a_context)
{
    m_context = a_context;
}

ContextSptr Thread::_getContext()
{
    return m_context;
}

void Thread::_resetContext()
{
    m_context = ContextSptr();
}

ContextSptr Thread::getContext()
{
    ContextSptr ret;
    QThread     *_cur_thread = NULL;
    Thread      *cur_thread  = NULL;

    _cur_thread = QThread::currentThread();
    cur_thread  = dynamic_cast<Thread *>(
        _cur_thread
    );

    if (cur_thread){
        ret = cur_thread->_getContext();
    } else {
        ret = g_context;
    }

    return ret;
}

void Thread::setContext(
    ContextSptr a_context)
{
    QThread *_cur_thread = NULL;
    Thread  *cur_thread  = NULL;

    _cur_thread = QThread::currentThread();
    cur_thread  = dynamic_cast<Thread *>(_cur_thread);

    if (cur_thread){
        cur_thread->_setContext(a_context);
    } else {
        g_context = a_context;
    }
}

void Thread::resetContext()
{
    QThread *_cur_thread = QThread::currentThread();
    Thread  *cur_thread  = dynamic_cast<Thread *>(_cur_thread);

    if (cur_thread){
        cur_thread->_resetContext();
    } else {
        g_context = ContextSptr();
    }
}

