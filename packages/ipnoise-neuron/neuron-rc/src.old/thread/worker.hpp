#include "class.hpp"
DEFINE_CLASS(ThreadWorker);

#ifndef THREAD_WORKER_HPP
#define THREAD_WORKER_HPP

using namespace std;

#include "thread.hpp"

typedef vector< ThreadWorkerSptr >  Workers;
typedef Workers::iterator           WorkersIt;
typedef Workers::const_iterator     WorkersConstIt;

class ThreadWorker
    :   public Thread
{
    public:
        ThreadWorker(
            const string &a_name = "worker"
        );
        virtual ~ThreadWorker();

        // generic

        // static
        static void setWorkersCount(const int32_t &);

    protected:
        virtual int32_t loop_before();
        virtual void    loop_tick();
        virtual int32_t loop_after();

    private:
};

#endif

