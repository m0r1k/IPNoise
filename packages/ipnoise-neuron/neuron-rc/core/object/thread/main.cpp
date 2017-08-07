#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/uint32/main.hpp>
#include <core/object/unique/main.hpp>
#include <core/object/vector/main.hpp>
#include "db.hpp"

#include "main.hpp"

ObjectThread::ObjectThread(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
    m_inotify_bev   = NULL;
    m_event_base    = NULL;
    m_dns_base      = NULL;
}

ObjectThread::~ObjectThread()
{
    AcceptsIt   it;

    while (m_accepts.size()){
        struct evconnlistener *evcon = NULL;
        it    = m_accepts.begin();
        evcon = it->second;
        evconnlistener_free(evcon);
        m_accepts.erase(it);
    }

    if (m_inotify_bev){
        bufferevent_free(m_inotify_bev);
        m_inotify_bev = NULL;
    }

    libevent_free();
}

string ObjectThread::getType()
{
    return ObjectThread::s_getType();
}

int32_t ObjectThread::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    return err;
}

int32_t ObjectThread::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_object(a_data, a_data_size);

    // in
    registerActionIn(
        "core.action.thread.listen",
        ObjectThread::s_actionListen
    );
    registerActionIn(
        "core.action.thread.watch.add",
        ObjectThread::s_actionWatchAdd
    );
    registerActionIn(
        "core.action.thread.write",
        ObjectThread::s_actionWrite
    );
    registerActionIn(
        "core.action.thread.close",
        ObjectThread::s_actionClose
    );

    // out
    registerActionOut("core.action.thread.started");
    registerActionOut("core.action.thread.listened");
    registerActionOut("core.action.thread.accepted");
    registerActionOut("core.action.thread.readed");
    registerActionOut("core.action.thread.event");
    registerActionOut("core.action.thread.wrote");

    return err;
}

ObjectSptr ObjectThread::copy()
{
    ObjectThreadSptr    ret;

    ret = OBJECT<ObjectThread>();

    return ret;
}

void ObjectThread::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();
    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

void ObjectThread::loopbreak()
{
    if (m_event_base){
        event_base_loopbreak(m_event_base);
    }
}

void ObjectThread::s_accept_cb(
    struct evconnlistener   *a_listener,
    evutil_socket_t         a_fd,
    struct sockaddr         *a_src_addr,
    int32_t                 a_src_addr_len,
    void                    *a_ctx)
{
    ObjectThread    *thread = NULL;
    ObjectSptr      action;

    thread = static_cast<ObjectThread *>(a_ctx);
    if (!thread){
        PFATAL("thread not found\n");
    }

    PDEBUG(50, "accept fd: '%d'\n", a_fd);

    action = OBJECT<Object>();
    action->setActionName("core.action.thread.accepted");
    action->setActionParam("fd", a_fd);
    thread->emit(action);
}

void ObjectThread::s_read_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    ObjectThread *thread = static_cast<ObjectThread *>(a_ctx);
    lock_guard<recursive_mutex> guard(thread->m_rmutex);

    int32_t res;
    char    buffer[65535]   = { 0x00 };
    string  object_path     = thread->getObjectPath();

    int32_t fd = bufferevent_getfd(a_bev);

    PDEBUG(40, "Thread::read_cb, fd: '%d'\n", fd);

    if (a_bev == thread->m_inotify_bev){
        // it is inotify event
        char    *buffer_read    = buffer;
        char    *buffer_write   = buffer;
        char    *buffer_end     = buffer + sizeof(buffer);

        struct inotify_event *ev = NULL;

        ev  = (struct inotify_event *)buffer_read;
        res = bufferevent_read(
            a_bev,
            buffer_write,
            buffer_end - buffer_write
        );
        if (0 < res){
            buffer_write += res;
        }

#ifdef __x86_64__
        PDEBUG(50, "after read from fd: '%d':\n"
            "  res:                         '%d'\n"
            "  buffer:                      0x%lx\n"
            "  buffer read:                 0x%lx\n"
            "  buffer write:                0x%lx\n"
            "  buffer end:                  0x%lx\n"
            "  buffer_write - buffer_read:  '%ld'\n"
            "  sizeof(*ev):                 '%lu'\n",
            fd,
            res,
            uint64_t(buffer),
            uint64_t(buffer_read),
            uint64_t(buffer_write),
            uint64_t(buffer_end),
            uint64_t(buffer_write - buffer_read),
            sizeof(*ev)
        );
#else
        PDEBUG(50, "after read from fd: '%d':\n"
            "  res:                         '%d'\n"
            "  buffer:                      0x%x\n"
            "  buffer read:                 0x%x\n"
            "  buffer write:                0x%x\n"
            "  buffer end:                  0x%x\n"
            "  buffer_write - buffer_read:  '%d'\n"
            "  sizeof(*ev):                 '%u'\n",
            fd,
            res,
            uint32_t(buffer),
            uint32_t(buffer_read),
            uint32_t(buffer_write),
            uint32_t(buffer_end),
            uint32_t(buffer_write - buffer_read),
            sizeof(*ev)
        );
#endif

        while (sizeof(*ev) <= uint64_t(buffer_write - buffer_read)){
            string              full_path;
            string              action_prefix;
            InotifyWatchesIt    it;
            int32_t             wd   = ev->wd;
            uint32_t            mask = ev->mask;

            // prepare for read next event
            buffer_read += sizeof(*ev);
            buffer_read += ev->len;

            // search watch descriptor
            it = thread->m_inotify_dirs.find(wd);
            if (it == thread->m_inotify_dirs.end()){
                PWARN("watch descriptor: '%d' was not found"
                    " in watch list, event will be skipped\n",
                    wd
                );
                continue;
            }
            full_path = it->second;

            // may be event has sub path?
            if (ev->len){
                full_path += ev->name;
            }

            PDEBUG(10, "read event:\n"
                "  object_path: '%s'\n"
                "  full_path:   '%s'\n"
                "  wd:          '%d'\n"
                "  mask:        0x%x\n",
                object_path.c_str(),
                full_path.c_str(),
                wd,
                mask
            );

            // monitor IPNOISE_DB_THREADS dir
            if (    IN_DELETE & mask
                &&  object_path == (full_path + "/"))
            {
                // request quit
                thread->loopbreak();
                break;
            }

            // monitor OBJECT_DIR_ACTIONS
            action_prefix = object_path + OBJECT_DIR_ACTIONS;
            if (    IN_CREATE & mask
                &&  action_prefix == full_path.substr(
                    0,
                    action_prefix.size()
                ))
            {
                ObjectSptr action;

                PDEBUG(100, "object: '%s',"
                    " was found new action: '%s'\n",
                    object_path.c_str(),
                    full_path.c_str()
                );

                // load action
                action = g_engine_info->objectLoad(
                    full_path.c_str()
                );

                // remove link
                unlink(full_path.c_str());

                if (action){
                    action->run(thread);
                } else {
                    PERROR("cannot load object from: '%s'\n",
                        full_path.c_str()
                    );
                }
            }

            // remove event from buffer
            // and process next events
            memmove(
                buffer,
                buffer_read,
                buffer_write - buffer_read
            );
            buffer_read  -= sizeof(*ev) + ev->len;
            buffer_write -= sizeof(*ev) + ev->len;
        }
    } else {
        // it is not inotify event
        res = bufferevent_read(
            a_bev,
            buffer,
            sizeof(buffer)
        );

        PDEBUG(50, "after read from fd: '%d', res: '%d'\n",
            fd,
            res
        );

        if (0 < res){
            ObjectSptr  action;
            action = OBJECT<Object>();
            action->setActionName("core.action.thread.readed");
            action->setActionParam("data", buffer, res);
            action->setActionParam("fd", fd);
            thread->emit(action);
        }
    }
}

void ObjectThread::s_write_cb(
    struct bufferevent  *a_bev,
    void                *a_ctx)
{
    ObjectThread *thread = static_cast<ObjectThread *>(a_ctx);
    lock_guard<recursive_mutex> guard(thread->m_rmutex);

    int32_t fd = bufferevent_getfd(a_bev);

    PDEBUG(40, "Thread::write_cb, fd: '%d'\n", fd);
}

void ObjectThread::s_event_cb(
    struct bufferevent  *a_bev,
    short               a_what,
    void                *a_ctx)
{
    ObjectThread *thread = static_cast<ObjectThread *>(a_ctx);
    lock_guard<recursive_mutex> guard(thread->m_rmutex);

    ObjectSptr          action;
    ObjectUniqueSptr    flags;
    int32_t             fd = bufferevent_getfd(a_bev);
    int32_t             free_bev = 0;

    PDEBUG(40, "Thread::event_cb: fd: '%d', what: '%d'\n",
        fd,
        a_what
    );

    flags = OBJECT<ObjectUnique>();

    if (BEV_EVENT_READING & a_what){
        // error encountered while reading
        free_bev = 1;
        flags->add("reading");
    }
    if (BEV_EVENT_WRITING & a_what){
        // error encountered while writing
        free_bev = 1;
        flags->add("writing");
    }
    if (BEV_EVENT_EOF & a_what){
        // eof file reached
        free_bev = 1;
        flags->add("eof");
    }
    if (BEV_EVENT_ERROR & a_what){
        // unrecoverable error encountered
        free_bev = 1;
        flags->add("error");
    }
    if (BEV_EVENT_TIMEOUT & a_what){
        // user-specified timeout reached
        free_bev = 1;
        flags->add("timeout");
    }
    if (BEV_EVENT_CONNECTED & a_what){
        // connect operation finished
        flags->add("connected");
    }

    action = OBJECT<Object>();
    action->setActionName("core.action.thread.event");
    action->setActionParam("fd",    fd);
    action->setActionParam("flags", flags);
    thread->emit(action);

    if (free_bev){
        thread->freeBev(fd);
    }
}

void ObjectThread::run(
    Object *a_thread)
{
    char        buffer[65535]   = { 0x00 };
    int32_t     res             = -1;
    uint32_t    mask            = 0;

    string      object_path;
    ObjectSptr  action;
    int32_t     fd = -1;

    object_path = getObjectPath();

    PINFO("thread: '%s' started\n", object_path.c_str());

    {
        double          cur_time = 0.0f;
        struct timeval  tv;

        gettimeofday(&tv, NULL);
        cur_time = tv.tv_sec + tv.tv_usec / 1e6;
        PWARN("cur time: '%f'\n", cur_time);
    }

    // init libevent
    res = libevent_init();
    if (res){
        goto fail;
    }

    // init inotify
    fd = inotify_init();
    if (0 > fd){
        PERROR("cannot init inotify\n");
        goto fail;
    }

    // add for watch
    {
        string  path;
        path = IPNOISE_DB_THREADS;
        mask = IN_DELETE;
        res  = inotify_add_watch(
            fd,
            path.c_str(),
            mask
        );
        if (0 > res){
            PERROR("cannot add for watch: '%s':\n"
                "  mask: 0x%x\n"
                "  res:  '%d'\n",
                path.c_str(),
                mask,
                res
            );
            goto fail;
        }
        m_inotify_dirs[res] = path;
    }

    // add for watch
    {
        string  path;
        path = object_path + OBJECT_DIR_ACTIONS;

        mask = IN_CREATE;
        res  = inotify_add_watch(
            fd,
            path.c_str(),
            mask
        );
        if (0 > res){
            PERROR("cannot add for watch: '%s':\n"
                "  mask: 0x%x\n"
                "  res:  '%d'\n",
                path.c_str(),
                mask,
                res
            );
            goto fail;
        }
        m_inotify_dirs[res] = path;
    }

    // add inotify fd for watch to libevent
    m_inotify_bev = bufferevent_socket_new(
        m_event_base,
        fd,
        BEV_OPT_CLOSE_ON_FREE
    );
    if (!m_inotify_bev){
        PERROR("cannot create bufferevent_socket_new"
            " for fd: '%d'\n",
            fd
        );
        goto fail;
    }

    bufferevent_setcb(
        m_inotify_bev,
        ObjectThread::s_read_cb,
        ObjectThread::s_write_cb,
        ObjectThread::s_event_cb,
        this
    );

    bufferevent_enable(m_inotify_bev, EV_READ | EV_PERSIST);

    // create action
    action = OBJECT<Object>();
    action->setActionName("core.action.thread.started");
    emit(action);

    // start main loop
    event_base_dispatch(m_event_base);

out:
    PINFO("thread: '%s' stopped\n", object_path.c_str());

    // remove link to us, from threads
    snprintf(buffer, sizeof(buffer),
        "%s/%s",
        IPNOISE_DB_THREADS,
        getId().c_str()
    );
    unlink(buffer);

    return;
fail:
    goto out;
}

int32_t ObjectThread::saveAsProp(
    const char  *a_dir)
{
    int32_t     res, err = -1;
    string      dir      = getObjectPath();

    if (a_dir){
        dir = a_dir;
    }

    if (dir.empty()){
        PERROR("cannot save object,"
            " unknown object directory\n"
        );
        goto fail;
    }

    // add '/' to end if not exist
    if ('/' != dir.at(dir.size() - 1)){
        dir += "/";
    }

    // create dirs
    {
        ObjectVectorSptr    dirs;

        dirs = OBJECT<ObjectVector>();
        dirs->add(dir);
        dirs->add(dir + OBJECT_DIR_ACTIONS);

        res = ObjectDir::s_mkdir(dirs, 1);
        if (res){
            PFATAL("cannot create one or more dir(s),"
                " dir(s) dump below:\n"
                "%s\n",
                dirs->toString()->c_str()
            );
        }
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){

    }
    goto out;
}

int32_t ObjectThread::libevent_init()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    int32_t err = -1;

    libevent_free();

    // event base
    m_event_base = ::event_base_new();
    if (!m_event_base){
        PERROR("event_base_new failed\n");
        goto fail;
    }

    // event dns
    m_dns_base = ::evdns_base_new(
        m_event_base,
        1   // initialize_nameservers
    );
    if (!m_dns_base){
        PERROR("evdns_base_new failed\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

void ObjectThread::libevent_free()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    if (m_dns_base){
        ::evdns_base_free(
            m_dns_base,
            0  // fail_requests
        );
        m_dns_base = NULL;
    }
    if (m_event_base){
        ::event_base_free(m_event_base);
        m_event_base = NULL;
    }
}

// ---------------- evconn ----------------

struct evconnlistener * ObjectThread::getEvConn(
    const char      *a_addr,
    const int32_t   &a_port)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    char                    buffer[512] = { 0x00 };
    struct evconnlistener   *listener   = NULL;
    AcceptsIt               accepts_it;

    snprintf(buffer, sizeof(buffer),
        "%s:%d",
        a_addr,
        a_port
    );

    accepts_it = m_accepts.find(buffer);
    if (m_accepts.end() != accepts_it){
        listener = accepts_it->second;
    }

    return listener;
}

struct evconnlistener * ObjectThread::getCreateEvConn(
    const char      *a_addr,
    const int32_t   &a_port)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    char                    buffer[512] = { 0x00 };
    struct evconnlistener   *listener   = NULL;

    // maybe exist
    listener = getEvConn(a_addr, a_port);
    if (listener){
        goto out;
    }

    {
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        int32_t         res;

        memset(&hints, 0x00, sizeof(struct addrinfo));

        hints.ai_family     = AF_INET;
        hints.ai_socktype   = SOCK_STREAM;
        hints.ai_flags      = 0;
        hints.ai_protocol   = 0;

        snprintf(buffer, sizeof(buffer), "%d", a_port);
        res = getaddrinfo(
            a_addr,
            buffer,   // service
            &hints,
            &result
        );
        if (res){
            PERROR("getaddrinfo host: '%s', port: '%d'"
                " failed (%s)\n",
                a_addr,
                a_port,
                gai_strerror(res)
            );
            goto fail;
        }

        // getaddrinfo() returns a list of address structures.
        // Try each address until we successfully bind
        for (rp = result; rp != NULL; rp = rp->ai_next){
            //listener = ::evconnlistener_new_bind(
            listener = ::evconnlistener_hostos_new_bind(
                m_event_base,
                ObjectThread::s_accept_cb,
                (void *)this,
	            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                -1,
	            rp->ai_addr,
	            rp->ai_addrlen
            );
	        if (listener){
                break;
            }
        }

        freeaddrinfo(result);

        if (!listener){
		    PERROR("cannot not create a listener at %s:%d\n",
                a_addr,
                a_port
            );
            goto fail;
        }
    }

    snprintf(buffer, sizeof(buffer),
        "%s:%d",
        a_addr,
        a_port
    );
    m_accepts[buffer] = listener;

out:
    return listener;
fail:
    if (listener){
        evconnlistener_free(listener);
        listener = NULL;
    }
    goto out;
}

// ---------------- bev ----------------

struct bufferevent * ObjectThread::getBev(
    const int32_t &a_fd)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    BevsIt      it;
    bufferevent *bev = NULL;

    it = m_bevs.find(a_fd);
    if (m_bevs.end() != it){
        bev = it->second;
    }

    return bev;
}

struct bufferevent * ObjectThread::getCreateBev(
    const int32_t &a_fd)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    bufferevent *bev = NULL;

    bev = getBev(a_fd);
    if (bev){
        goto out;
    }

    bev = bufferevent_socket_new(
        m_event_base,
        a_fd,
        BEV_OPT_CLOSE_ON_FREE
    );
    if (!bev){
        PERROR("cannot create bufferevent_socket_new"
            " for fd: '%d'\n",
            a_fd
        );
        goto fail;
    }

    m_bevs[a_fd] = bev;

out:
    return bev;
fail:
    if (bev){
        bufferevent_free(bev);
        bev = NULL;
    }
    goto out;
}

void ObjectThread::freeBev(
    const int32_t &a_fd)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    BevsIt      it;
    bufferevent *bev = NULL;

    if (0 > a_fd){
        PFATAL("invalid 'a_fd' value: '%d'\n",
            a_fd
        );
    }

    it = m_bevs.find(a_fd);
    if (m_bevs.end() != it){
        bev = it->second;
        bufferevent_free(bev);
        m_bevs.erase(it);
    }

    ::close(a_fd);
}

// ---------------- static ----------------

int32_t ObjectThread::s_actionListen(
    Object *a_object,
    Object *a_action)
{
    int32_t             err         = -1;
    ObjectThread        *thread     = NULL;
    int32_t             port        = -1;
    ObjectStringSptr    prop_host;
    ObjectUint32Sptr    prop_port;
    string              host;
    string              object_path;
    string              object_type;
    string              action_path;
    string              action_type;

    struct evconnlistener *listener = NULL;

    thread = dynamic_cast<ObjectThread *>(a_object);
    if (!thread){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectThread\n"
        );
    }

    if (!thread->m_event_base){
        PFATAL("m_event_base is NULL\n");
    }

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    object_path = thread->getObjectPath();
    object_type = thread->getType();
    action_path = a_action->getObjectPath();
    action_type = a_action->getType();

    // host
    prop_host = dynamic_pointer_cast<ObjectString>(
        a_action->getActionParam("host")
    );
    if (!prop_host){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s' (type: '%s'),"
            " missing param 'host',"
            " action's dump below:\n"
            "%s\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            action_type.c_str(),
            a_action->toString()->c_str()
        );
        goto fail;
    }

    host = prop_host->getVal();
    if (host.empty()){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s',"
            " empty param: 'host',\n"
            " action's dump below:\n"
            "%s\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            a_action->toString()->c_str()
        );
        goto fail;
    }

    // port
    prop_port = dynamic_pointer_cast<ObjectUint32>(
        a_action->getActionParam("port")
    );
    if (!prop_port){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s' (type: '%s'),"
            " missing param 'port',\n"
            " action's dump below:\n"
            "%s\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            action_type.c_str(),
            a_action->toString()->c_str()
        );
        system("bash");
        goto fail;
    }

    port = prop_port->getVal();
    if (0 > port){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s',"
            " invalid param: 'port' value: '%d'\n"
            " action's dump below:\n"
            "%s\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            port,
            a_action->toString()->c_str()
        );
        goto fail;
    }

    PDEBUG(50, "object: '%s', request listen,"
        " host: '%s', port: '%d'\n",
        object_path.c_str(),
        host.c_str(),
        port
    );

    listener = thread->getCreateEvConn(
        host.c_str(),
        port
    );
    if (!listener){
        PERROR("cannot create listener\n");
        goto fail;
    }

    {
        ObjectSptr  action;
        action = OBJECT<Object>();
        action->setActionName("core.action.thread.listened");
        action->setActionParam("port", port);
        thread->emit(action);
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t ObjectThread::s_actionWatchAdd(
    Object *a_object,
    Object *a_action)
{
    int32_t             err         = -1;
    ObjectThread        *thread     = NULL;
    int32_t             fd          = -1;
    struct bufferevent  *bev        = NULL;
    ObjectInt32Sptr     prop_fd;
    string              object_path;
    string              object_type;
    string              action_path;
    string              action_type;

    thread = dynamic_cast<ObjectThread *>(a_object);
    if (!thread){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectThread\n"
        );
    }

    if (!thread->m_event_base){
        PFATAL("m_event_base is NULL\n");
    }

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    object_path = thread->getObjectPath();
    object_type = thread->getType();
    action_path = a_action->getObjectPath();
    action_type = a_action->getType();

    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        a_action->getActionParam("fd")
    );
    if (!prop_fd){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s' (type: '%s'),"
            " missing param 'fd'\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            action_type.c_str()
        );
        goto fail;
    }

    fd = prop_fd->getVal();
    if (0 > fd){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s',"
            " invalid param: 'fd' value: '%d'\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            fd
        );
        goto fail;
    }

    PDEBUG(50, "object: '%s', request watch, fd: '%d'\n",
        object_path.c_str(),
        fd
    );

    bev = thread->getCreateBev(fd);
    bufferevent_setcb(
        bev,
        ObjectThread::s_read_cb,
        ObjectThread::s_write_cb,
        ObjectThread::s_event_cb,
        thread
    );
    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t ObjectThread::s_actionWrite(
    Object *a_object,
    Object *a_action)
{
    int32_t             res, err    = -1;
    ObjectThread        *thread     = NULL;
    int32_t             fd          = -1;
    uint64_t            wrote       = 0;
    ObjectInt32Sptr     prop_fd;
    ObjectStringSptr    prop_data;
    string              object_path;
    string              object_type;
    string              action_path;
    string              action_type;

    thread = dynamic_cast<ObjectThread *>(a_object);
    if (!thread){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectThread\n"
        );
    }

    if (!thread->m_event_base){
        PFATAL("m_event_base is NULL\n");
    }

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    object_path = thread->getObjectPath();
    object_type = thread->getType();
    action_path = a_action->getObjectPath();
    action_type = a_action->getType();

    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        a_action->getActionParam("fd")
    );
    if (!prop_fd){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s' (type: '%s'),"
            " missing param 'fd'\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            action_type.c_str()
        );
        goto fail;
    }

    fd = prop_fd->getVal();
    if (0 > fd){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s',"
            " invalid param: 'fd' value: '%d'\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            fd
        );
        goto fail;
    }

    prop_data = dynamic_pointer_cast<ObjectString>(
        a_action->getActionParam("data")
    );
    if (!prop_data){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s' (type: '%s'),"
            " missing param 'data'\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            action_type.c_str()
        );
        goto fail;
    }

    do {
        res = ::write(
            fd,
            prop_data->c_str() + wrote,
            prop_data->size() - wrote
        );
        if (0 < res){
            wrote += res;
        }
    } while (wrote != prop_data->size());

    {
        ObjectSptr  action;

        action = OBJECT<Object>();
        action->setActionName("core.action.thread.wrote");
        action->setActionParam("fd",      fd);
        action->setActionParam("wrote",   wrote);
        thread->emit(action);
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t ObjectThread::s_actionClose(
    Object *a_object,
    Object *a_action)
{
    int32_t             err         = -1;
    ObjectThread        *thread     = NULL;
    int32_t             fd          = -1;
    ObjectInt32Sptr     prop_fd;
    string              object_path;
    string              object_type;
    string              action_path;
    string              action_type;

    thread = dynamic_cast<ObjectThread *>(a_object);
    if (!thread){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectThread\n"
        );
    }

    if (!thread->m_event_base){
        PFATAL("m_event_base is NULL\n");
    }

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    object_path = thread->getObjectPath();
    object_type = thread->getType();
    action_path = a_action->getObjectPath();
    action_type = a_action->getType();

    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        a_action->getActionParam("fd")
    );
    if (!prop_fd){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s' (type: '%s'),"
            " missing param 'fd'\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            action_type.c_str()
        );
        goto fail;
    }

    fd = prop_fd->getVal();
    if (0 > fd){
        PERROR("object: '%s' (type: '%s'),"
            " skip action: '%s',"
            " invalid param: 'fd' value: '%d'\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            fd
        );
        goto fail;
    }

    PDEBUG(50, "object: '%s', request close, fd: '%d'\n",
        object_path.c_str(),
        fd
    );

    thread->freeBev(fd);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

// ---------------- module ----------------

string ObjectThread::s_getType()
{
    return "core.object.thread";
}

int32_t ObjectThread::s_init(
    EngineInfo *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectThread::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectThread::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectThread(a_create_flags));

    return object;
}

void ObjectThread::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_thread = {
    .getType        = ObjectThread::s_getType,
    .init           = ObjectThread::s_init,
    .shutdown       = ObjectThread::s_shutdown,
    .objectCreate   = ObjectThread::s_objectCreate,
    .getTests       = ObjectThread::s_getTests
};

