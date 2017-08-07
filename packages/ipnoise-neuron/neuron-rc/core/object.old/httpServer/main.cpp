#include <netdb.h>

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
//#include "core/object/core/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/session/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"
#include "thread.hpp"

#include "core/object/httpServer/main.hpp"

ObjectHttpServer::ObjectHttpServer(
    const Object::CreateFlags &a_flags)
    :   Object(a_flags)
{
}

ObjectHttpServer::~ObjectHttpServer()
{
}

void ObjectHttpServer::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectHttpServer::getType()
{
    return ObjectHttpServer::_getType();
}

void ObjectHttpServer::do_init_props()
{
    Object::do_init_props();
}

void ObjectHttpServer::do_init_api()
{
    Object::do_init_api();

    // register actions
    registerAction("thread.io.listen", ObjectHttpServer::actionListen);
    registerAction("thread.io.accept", ObjectHttpServer::actionAccept);
}

/*
void ObjectHttpServer::processEvents(
    ObjectVectorSptr a_events)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr events = a_events;
    ObjectVectorIt   it;

    PWARN("process events, object ID: '%s'\n"
        "  events: '%s'\n",
        getId()->toString()->c_str(),
        a_events->toString()->c_str()
    );

again:
    for (it = a_events->begin();
        it != a_events->end();
        it++)
    {
        ObjectMapSptr   event;
        string          event_name;

        event      = dynamic_pointer_cast<ObjectMap>(*it);
        event_name = event->get("name")->toString();

        if ("thread.accept" == event_name){
            a_events->erase(it);
            goto again;
        }
    }

    Object::processEvents(a_events);
}
*/

// ---------------- api ----------------

void ObjectHttpServer::actionListen(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    int32_t     res         = -1;
    string      addr        = "::1";
    int32_t     port        = HTTP_SERVER_PORT;
    ThreadSptr  cur_thread  = Thread::getCurThread();

    PWARN("object ID: '%s', call listen: '%s:%d'\n",
        a_object->getId().c_str(),
        addr.c_str(),
        port
    );

    res = cur_thread->subscrAccept(a_object, addr, port);
    if (res){
        PERROR("cannot subscr for accept from: '%s:%d'\n",
            addr.c_str(),
            port
        );
        goto fail;
    }

out:
    return;
fail:
    goto out;
}

void ObjectHttpServer::actionAccept(
    ObjectSptr          a_object,
    ObjectActionSptr    a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PTIMING(0.0f, "event accept\n");

    ObjectInt32Sptr      prop_fd;
    ObjectSessionSptr    session;
    string               field_name;
    int32_t              res, fd     = -1;
    ThreadSptr           cur_thread  = Thread::getCurThread();
    ObjectContextSptr    context     = cur_thread->contextGet();

    // fd
    field_name  = "fd";
    prop_fd     = dynamic_pointer_cast<ObjectInt32>(
        a_action->getParam(field_name)
    );
    if (!prop_fd){
        PFATAL("missing '%s' param\n",
            field_name.c_str()
        );
    }
    fd = prop_fd->getVal();

    if (0){
        int32_t len;
        char    buffer[1024] = { 0x00 };
        string  body;

        PTIMING(0.010f, "write\n");

        body = ""
            "<html>\n"
                "<body>\n"
                    "<h1>MORIK was here</h1>\n"
                "</body>\n"
            "</html>\n";

        len = snprintf(buffer, sizeof(buffer),
            "HTTP/1.1 200 OK\r\n"
            "Connection: close\r\n"
            "Content-type:   text/html\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s",
            body.size(),
            body.c_str()
        );

        ::write(fd, buffer, len);

        cur_thread->close(fd);
    }

    // all ok, create session object
    session = Object::createNeigh<ObjectSession>(
        a_object,
        "core.object.session",  // type
        prop_fd->toString()     // name
    );

    session->setFd(fd);
    //session->save();
    context->transactionAddForSave(session);

    // all ok, subscr new object for read
    res = cur_thread->subscrIo(
        session->getId(),
        fd
    );
    if (res){
        PERROR("cannot subscr for read from fd: '%d'\n",
            fd
        );
        goto fail;
    }

out:
    return;
fail:
    goto out;
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectHttpServer::_getType()
{
    return "core.object.http.server";
}

string ObjectHttpServer::_getTypePerl()
{
    return "core::object::httpServer::main";
}

int32_t ObjectHttpServer::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectHttpServer::_destroy()
{
}

ObjectSptr ObjectHttpServer::_object_create()
{
    ObjectSptr object;
    _OBJECT_HTTP_SERVER(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectHttpServer::_getType,
    .type_perl      = ObjectHttpServer::_getTypePerl,
    .init           = ObjectHttpServer::_init,
    .destroy        = ObjectHttpServer::_destroy,
    .object_create  = ObjectHttpServer::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

