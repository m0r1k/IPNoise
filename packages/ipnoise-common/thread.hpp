class Thread;

#ifndef THREAD_HPP
#define THREAD_HPP

#include <pthread.h>
#include <vector>

typedef vector<Thread *>    Threads;
typedef Threads::iterator   ThreadsIt;

using namespace std;

class Thread
{
    public:
        enum State {
            THREAD_STATE_ERROR      = 0,
            THREAD_STATE_STARTED,
            THREAD_STATE_STOPPED
        };

        Thread(){
            m_state     = THREAD_STATE_ERROR;
            m_func      = NULL;
            m_func_arg  = NULL;

            memset(&m_thread,       0x00, sizeof(m_thread));
            memset(&m_thread_attr,  0x00, sizeof(m_thread_attr));
        }
        virtual ~Thread(){
            // TODO XXX
            // add stop thread here
        }

        int start(void *a_arg){
            int res, err = -1;

            // store arg
            m_func_arg = a_arg;

            // init attrs
            res = pthread_attr_init(&m_thread_attr);
            if (res){
                PERROR("pthread_attr_init failed\n");
                goto fail;
            }

            // create thread
            res = pthread_create(
                &m_thread,
                &m_thread_attr,
                Thread::func,
                (void *)this
            );
            if (res){
                PERROR("pthread_create failed\n");
                goto fail;
            }

           // all ok;
           err = 0;

        out:
            return err;

        fail:
            if (err > 0){
                err = -1;
            }
            goto out;
        }

        int wait(){
            int     res, err    = -1;
            void    *retval     = NULL;

            // wait thread exit
            res = pthread_join(m_thread, &retval);
            if (res){
                PERROR("pthread_join failed\n");
                goto fail;
            }

            // all ok
            err = 0;

            out:
                return err;

            fail:
                if (err >= 0){
                    err = -1;
                }
                goto out;
        }

        void setFunc(void * (*a_func)(void *)){
            m_func = a_func;
        }

        static void * func(void *a_ctx){
            void    *retval = NULL;
            Thread  *thread = (Thread *)a_ctx;

            thread->m_state = THREAD_STATE_STARTED;
            PDEBUG(10, "thread started\n");

            if (thread->m_func){
                retval = thread->m_func(thread->m_func_arg);
            }

            thread->m_state = THREAD_STATE_STOPPED;
            PDEBUG(10, "thread stoped\n");

            pthread_exit(retval);
        }


    private:
        State           m_state;
        pthread_t       m_thread;
        pthread_attr_t  m_thread_attr;

        void *          (*m_func)(void *);
        void            *m_func_arg;

};


#endif

