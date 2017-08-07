#include "class.hpp"
DEFINE_CLASS(Thread);

#ifndef THREAD_HPP
#define THREAD_HPP

#include <sqlite3.h>

#include <mutex>
#include <map>
#include <vector>
#include <thread>
#include <string>

// libevent
#include <event2/util.h>

#include "log.hpp"
#include "utils.hpp"
#include "kladr.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectContext);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectUnique);

DEFINE_CLASS(Db);
DEFINE_CLASS(DbDisk);
DEFINE_CLASS(DbMongo);

using namespace std;

typedef map<string, ThreadSptr>         ThreadsByName;
typedef ThreadsByName::iterator         ThreadsByNameIt;
typedef ThreadsByName::const_iterator   ThreadsByNameConstIt;

typedef map<uint64_t, ThreadSptr>       ThreadsByAddr;
typedef ThreadsByAddr::iterator         ThreadsByAddrIt;
typedef ThreadsByAddr::const_iterator   ThreadsByAddrConstIt;

typedef map<uint64_t, ObjectContextSptr>    Contexts;
typedef Contexts::iterator                  ContextsIt;
typedef Contexts::const_iterator            ContextsConstIt;

// accept
typedef map<string, struct evconnlistener *>    Accepts;
typedef Accepts::iterator                       AcceptsIt;
typedef Accepts::const_iterator                 AcceptsConstIt;

typedef map<struct evconnlistener *, ObjectMapSptr>  AcceptsSubscr;
typedef AcceptsSubscr::iterator                      AcceptsSubscrIt;
typedef AcceptsSubscr::const_iterator                AcceptsSubscrConstIt;

// read, write, event
typedef map<int32_t, struct bufferevent *>      Bevs;
typedef Bevs::iterator                          BevsIt;
typedef Bevs::const_iterator                    BevsConstIt;

typedef map<struct bufferevent *, ObjectMapSptr>  IoSubscr;
typedef IoSubscr::iterator                        IoSubscrIt;
typedef IoSubscr::const_iterator                  IoSubscrConstIt;

extern recursive_mutex      g_threads_mutex;
extern ThreadsByName        g_threads_by_name;
extern ThreadsByAddr        g_threads_by_addr;

#define CREATE_THREAD(out, class_name, thread_name)         \
    do {                                                    \
        lock_guard<recursive_mutex> guard(g_threads_mutex); \
        out = shared_ptr<class_name>(                       \
            new class_name(thread_name)                     \
        );                                                  \
        g_threads_by_name[thread_name] = out;               \
    } while (0);

typedef map<string, NOT_USED>       Parents;
typedef Parents::iterator           ParentsIt;
typedef Parents::const_iterator     ParentsConstIt;

class Thread
{
    public:
        Thread(const string &a_name);
        virtual ~Thread();

        enum ThreadState {
            THREAD_STATE_ERROR_CANNOT_SHUTDOWN  = -6,
            THREAD_STATE_ERROR_CANNOT_START     = -5,
            THREAD_STATE_ERROR_CANNOT_INIT      = -4,
            THREAD_STATE_ERROR                  = -3,
            THREAD_STATE_SHUTDOWN               = -2,
            THREAD_STATE_SHUTDOWNING            = -1,
            THREAD_STATE_CREATED                = 0,
            THREAD_STATE_INIT,
            THREAD_STATE_INIT_THREAD,
            THREAD_STATE_RUNNING
        };
        virtual int32_t     start();
        virtual void        stop();
        virtual bool        joinable() const noexcept;

        // generic
        string              getName();
        string              getLocation();
        void                setAddr(const uint64_t &);
        uint64_t            getAddr();
        Thread::ThreadState getState();
        void                setState(
            const Thread::ThreadState &
        );
        DbSptr              getDb();
        void                getParents(vector<string> &);
        void                addParent(const string &);
        void                addParent(ThreadSptr);
        void                close(struct bufferevent *);
        void                close(const int32_t &);
        ObjectContextSptr   contextGet();
        void                contextSet(ObjectContextSptr);
        ObjectContextSptr   contextReset();
        void                contextTransCommit();
        struct event_base * getEvBase();
        int32_t             initLibEvent();
        void                freeLibEvent();
        int32_t             initDb();
        void                loopbreak();
        void                join();
        void                detach();

        // evcon
        struct evconnlistener * getEvConn(
            const string    &a_addr,
            const int32_t   &a_port
        );
        struct evconnlistener * getCreateEvConn(
            const string    &a_addr,
            const int32_t   &a_port
        );

        // accept
        ObjectMapSptr     getAcceptSubsr(
            struct evconnlistener   *a_listener
        );
        void    getCreateAcceptSubsr(
            struct evconnlistener   *a_listener,
            const string            &a_object_id
        );
        int32_t         subscrAccept(
            ObjectSptr      a_object,
            const string    &a_addr,
            const int32_t   &a_port
        );

        // bev
        struct bufferevent *    getBev(
            const int32_t   &a_fd
        );
        struct bufferevent *    getCreateBev(
            const int32_t   &a_fd
        );

        // subscr IO
        ObjectMapSptr       getIoSubsr(
            struct bufferevent *a_bev
        );
        void                getCreateIoSubsr(
            struct bufferevent  *a_bev,
            const string        &a_object_id
        );
        int32_t         subscrIo(
            const string    &a_object_id,
            const int32_t   &a_fd
        );

        int32_t         processKladrSql(
            const   string  &a_sql,
            KladrCb         a_cb,
            KladrReqInfo    &a_kladr_req_info,
            string          &a_sql_err_msg
        );

        string  _processPerlCode(
            const string &a_code
        );
        string _processPerlSub(
            const string    &a_sub_name,
            ObjectMapSptr   a_arg
        );

        recursive_mutex     m_mutex;

        // static
        static ThreadSptr   getCurThread();
        static string       getCurThreadName();
        static uint64_t     getCurThreadAddr();
        static void         accept_cb(
            struct evconnlistener   *a_listener,
            evutil_socket_t         a_fd,
            struct sockaddr         *a_src_addr,
            int32_t                 a_src_addr_len,
            void                    *a_ctx
        );
        static  void        read_cb(
            struct bufferevent  *a_bev,
            void                *a_ctx
        );
        static void         write_cb(
            struct bufferevent  *a_bev,
            void                *a_ctx
        );
        static void         event_cb(
            struct bufferevent  *a_bev,
            short               a_what,
            void                *a_ctx
        );

        static ObjectContextSptr  getCurContext();
        static void               setCurContext(
            ObjectContextSptr
        );
        static ObjectContextSptr  resetCurContext();

        static ThreadSptr   getThreadByName(const string &);
        static ThreadSptr   getThreadByAddr(const uint64_t &);
        static DbSptr       getCurThreadDb();

        static void         getThreadsRunning(
            ThreadsByName   &a_out,
            const int32_t   &a_skip_main = 0
        );
        static void         removeShutdownedThreads(
            const int32_t   &a_skip_main = 1
        );
        static void         getThreadsWithRunningParents(
            ThreadsByName   &a_out,
            const int32_t   &a_skip_main = 0
        );
        static void         getThreadsWithoutParents(
            ThreadsByName   &a_out,
            const int32_t   &a_skip_main = 0
        );
        static void         getThreadsWithoutChildren(
            ThreadsByName   &a_out,
            const int32_t   &a_skip_main = 0
        );
        static void         getThreadsWithStopedChildren(
            ThreadsByName   &a_out,
            const int32_t   &a_skip_main = 0
        );
        static void         getThreadsAll(
            ThreadsByName   &a_out,
            const int32_t   &a_skip_main = 0
        );
        static void         getThreadsForRun(
            ThreadsByName   &a_out,
            const int32_t   &a_skip_main = 1
        );
        static void         getThreadsForStop(
            ThreadsByName   &a_out,
            const int32_t   &a_skip_main = 1
        );
        static void         startThreads(
            const ThreadsByName &
        );
        static void         stopThreads(
            const ThreadsByName &
        );
        static void         startAllThreads();
        static void         stopAllThreads();
        static void         joinAll();

        static void         timer_cb(
            evutil_socket_t     a_fd,
            short               a_event,
            void                *a_arg
        );

        static bool         isSameThread(
            ObjectMapSptr   a_req_params
        );

        static string       processPerlCode(
            const string &a_code
        );
        static string       processPerlSub(
            const string  &a_sub_name,
            ObjectMapSptr a_arg
        );

    protected:
        struct timeval      m_timer_interval;
        double              m_timer_interval_max;
        struct event        *m_timer;
        struct event_base   *m_event_base;
        struct evdns_base   *m_dns_base;

        // interface
        virtual int32_t loop_before();
        virtual void    loop();
        virtual void    loop_tick();
        virtual int32_t loop_after();

        // generic
        void    getTimerInterval(struct timeval       &);
        double  getTimerInterval();
        double  getTimerIntervalMax();
        void    setTimerInterval(const struct timeval &);
        void    setTimerInterval(const double         &);
        void    setTimerIntervalMax(const double      &);
        void    addTimerInterval(const double         &);
        void    subTimerInterval(const double         &);
        void    speedUp(const double &a_interval = 0.0f);
        void    speedDown();

        int32_t     initKladrDb();
        void        closeKladrDb();
        int32_t     initPerl();
        static void xs_init(pTHX);

        // static
        static int32_t  run(Thread *);

    private:
        string                  m_name;
        ThreadState             m_state;

        thread                  *m_std_thread;
        ObjectContextSptr       m_context;
        uint64_t                m_addr;

        Parents                 m_parents;

        Accepts                 m_accepts;
        AcceptsSubscr           m_accepts_subscr;
        Bevs                    m_bevs;
        IoSubscr                m_io_subscr;

        DbSptr                  m_db;

        PerlInterpreter         *m_perl;
};

#endif

