#include "class.hpp"
DEFINE_CLASS(ThreadHttp);

#ifndef THREAD_HTTP_HPP
#define THREAD_HTTP_HPP

using namespace std;

#include "context.hpp"
#include "thread.hpp"

class ThreadHttp
    :   public Thread
{
    Q_OBJECT

    public:
        ThreadHttp(
            Network         *a_net,
            const string    &a_name = "thread_http"
        );
        virtual ~ThreadHttp();

        virtual     int32_t do_init();
        virtual     void    do_start();
        virtual     void    do_stop();

    private:
};

#endif

