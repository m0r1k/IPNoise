#include "class.hpp"
DEFINE_CLASS(ObjectThread);

#ifndef OBJECT_THREAD_HPP
#define OBJECT_THREAD_HPP

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include <map>
#include <string>

#include "engine.hpp"
#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);

typedef map<int32_t, string>            InotifyWatches;
typedef InotifyWatches::iterator        InotifyWatchesIt;
typedef InotifyWatches::const_iterator  InotifyWatchesConstIt;

// accept
typedef map<string, struct evconnlistener *>    Accepts;
typedef Accepts::iterator                       AcceptsIt;
typedef Accepts::const_iterator                 AcceptsConstIt;

// bevs
typedef map<int32_t, struct bufferevent *>      Bevs;
typedef Bevs::iterator                          BevsIt;
typedef Bevs::const_iterator                    BevsConstIt;

using namespace std;

class ObjectThread
    :   public Object
{
    public:
        ObjectThread(const CreateFlags &a_create_flags);
        virtual ~ObjectThread();

        virtual string      getType();
        virtual int32_t     do_init_as_prop(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual int32_t     do_init_as_object(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual ObjectSptr  copy();
        virtual void        getAllProps(ObjectMapSptr);
        virtual void        run(Object *a_thread = NULL);
        virtual int32_t     saveAsProp(const char*);

        // generic
        void    loopbreak();

        struct evconnlistener * getEvConn(
            const char      *a_addr,
            const int32_t   &a_port
        );
        struct evconnlistener * getCreateEvConn(
            const char      *a_addr,
            const int32_t   &a_port
        );
        struct bufferevent *    getBev(const int32_t &a_fd);
        struct bufferevent *    getCreateBev(
            const int32_t &a_fd
        );
        void freeBev(const int32_t &);

        // static
        static int32_t      s_actionListen(
            Object *a_object,
            Object *a_action
        );
        static int32_t      s_actionWatchAdd(
            Object *a_object,
            Object *a_action
        );
        static int32_t      s_actionWrite(
            Object *a_object,
            Object *a_action
        );
        static int32_t      s_actionClose(
            Object *a_object,
            Object *a_action
        );

        static void         s_accept_cb(
            struct evconnlistener   *a_listener,
            evutil_socket_t         a_fd,
            struct sockaddr         *a_src_addr,
            int32_t                 a_src_addr_len,
            void                    *a_ctx
        );
        static void         s_read_cb(
            struct bufferevent  *a_bev,
            void                *a_ctx
        );
        static void         s_write_cb(
            struct bufferevent  *a_bev,
            void                *a_ctx
        );
        static void         s_event_cb(
            struct bufferevent  *a_bev,
            short               a_what,
            void                *a_ctx
        );

        struct event_base   *m_event_base;
        struct evdns_base   *m_dns_base;

        struct bufferevent  *m_inotify_bev;
        InotifyWatches      m_inotify_dirs;

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    protected:
        int32_t libevent_init();
        void    libevent_free();

    private:
        Accepts     m_accepts;
        Bevs        m_bevs;
};

#endif

