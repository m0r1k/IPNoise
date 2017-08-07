#include "class.hpp"
DEFINE_CLASS(ThreadMain);

#ifndef THREAD_MAIN_HPP
#define THREAD_MAIN_HPP

#include <thread>
#include <map>
#include <mutex>
#include <string>
#include <condition_variable>

#include "thread.hpp"

DEFINE_CLASS(ObjectHttpServer);
DEFINE_CLASS(ObjectCore);
DEFINE_CLASS(ObjectGod);

using namespace std;

#define HUMAN_HTTP_SERVER_PORT  8081
#define HTTP_SERVER_PORT        8080
#define HTTP_EVENT_SERVER_PORT  8079

class ThreadMain
    :   public Thread
{
    public:
        ThreadMain(
            const string &a_name = "main"
        );
        virtual ~ThreadMain();

        int32_t             start();

        virtual bool        joinable() const noexcept;

        // generic
/*
        void                    createPoints();
        ObjectCoreSptr          getCoreObject();
        ObjectCoreSptr          getCreateCoreObject();
*/

        ObjectHttpServerSptr    getHttpServerObject();
        ObjectHttpServerSptr    getCreateHttpServerObject();

        ObjectGodSptr           getGodObject();
        ObjectGodSptr           getCreateGodObject();

        // static
        static void         shutdown_cb(
            evutil_socket_t,
            short,
            void *a_ctx
        );

        // static

    protected:
        virtual int32_t loop_before();
        virtual void    loop_tick();
        virtual int32_t loop_after();

    private:
};

#endif

