#include <sys/time.h>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/http.request/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/uint32/main.hpp>
#include <core/object/unique/main.hpp>

#include "main.hpp"

ObjectHttpServer::ObjectHttpServer(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectHttpServer::~ObjectHttpServer()
{
}

string ObjectHttpServer::getType()
{
    return ObjectHttpServer::s_getType();
}

int32_t ObjectHttpServer::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    return err;
}

int32_t ObjectHttpServer::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = -1;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    m_host         = OBJECT<ObjectString>(DEFAULT_LISTEN_HOST);
    m_port         = OBJECT<ObjectUint32>(DEFAULT_LISTEN_PORT);
    m_pending_data = OBJECT<ObjectMap>();

    // in
    registerActionIn(
        "core.action.thread.started",
        ObjectHttpServer::s_actionListen
    );
    registerActionIn(
        "core.action.thread.listened",
        ObjectHttpServer::s_actionListened
    );
    registerActionIn(
        "core.action.thread.accepted",
        ObjectHttpServer::s_actionAccepted
    );
    registerActionIn(
        "core.action.thread.readed",
        ObjectHttpServer::s_actionReaded
    );
    registerActionIn(
        "core.action.thread.event",
        ObjectHttpServer::s_actionEvent
    );

    // out
    registerActionOut("core.action.thread.listen");
    registerActionOut("core.action.thread.watch.add");
    registerActionOut("core.action.thread.write");
    registerActionOut("core.action.thread.close");

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

ObjectSptr ObjectHttpServer::copy()
{
    ObjectHttpServerSptr    ret;

    ret = OBJECT<ObjectHttpServer>();

    ret->setHost(m_host->c_str());
    ret->setPort(m_port->getVal());

    return ret;
}

void ObjectHttpServer::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        a_out->add("server_host",        m_host);
        a_out->add("server_port",        m_port);
        a_out->add("server_requests",    m_pending_data);
        Object::getAllProps(a_out);
    }
}

void ObjectHttpServer::setHost(
    const char *a_val)
{
    m_host = OBJECT<ObjectString>(a_val);
}

ObjectStringSptr ObjectHttpServer::getHost()
{
    return m_host;
}

void ObjectHttpServer::setPort(
    const uint32_t &a_port)
{
    m_port = OBJECT<ObjectUint32>(a_port);
}

ObjectUint32Sptr ObjectHttpServer::getPort()
{
    return m_port;
}

// ---------------- static ----------------

int32_t ObjectHttpServer::s_actionListen(
    Object *a_object,
    Object *a_action)
{
    ObjectHttpServer    *object = NULL;
    string              object_path;

    object = dynamic_cast<ObjectHttpServer *>(a_object);
    if (!object){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectHttpServer\n"
        );
    }

    object_path = object->getObjectPath();

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    PWARN("object: '%s', request listen\n",
        object->getObjectPath().c_str()
    );

    {
        double          cur_time = 0.0f;
        struct timeval  tv;

        gettimeofday(&tv, NULL);
        cur_time = tv.tv_sec + tv.tv_usec / 1e6;
        PWARN("cur time: '%f'\n", cur_time);
    }

    {
        ObjectSptr  action;

        action = OBJECT<Object>();

        action->setActionName("core.action.thread.listen");
        action->setActionParam("host", object->m_host);
        action->setActionParam("port", object->m_port);

        object->emit(action);
    }

    return 0;
}

int32_t ObjectHttpServer::s_actionListened(
    Object  *a_object,
    Object  *a_action)
{
    ObjectHttpServer    *object = NULL;
    string              object_path;

    object = dynamic_cast<ObjectHttpServer *>(a_object);
    if (!object){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectHttpServer\n"
        );
    }

    object_path = object->getObjectPath();

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    PWARN("object: '%s', action: listened\n",
        object_path.c_str()
    );

    {
        double          cur_time = 0.0f;
        struct timeval  tv;

        gettimeofday(&tv, NULL);
        cur_time = tv.tv_sec + tv.tv_usec / 1e6;
        PWARN("cur time: '%f'\n", cur_time);
    }

    return 0;
}

int32_t ObjectHttpServer::s_actionAccepted(
    Object  *a_object,
    Object  *a_action)
{
    ObjectHttpServer    *object = NULL;
    int32_t             fd      = -1;
    string              object_path;
    ObjectInt32Sptr     prop_fd;

    object = dynamic_cast<ObjectHttpServer *>(a_object);
    if (!object){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectHttpServer\n"
        );
    }

    object_path = object->getObjectPath();

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        a_action->getActionParam("fd")
    );
    fd = prop_fd->getVal();

    PDEBUG(100, "object: '%s', new accept, fd: '%d'\n",
        object_path.c_str(),
        fd
    );

    {
        double          cur_time = 0.0f;
        struct timeval  tv;

        gettimeofday(&tv, NULL);
        cur_time = tv.tv_sec + tv.tv_usec / 1e6;
        PWARN("cur time: '%f'\n", cur_time);
    }

    {
        ObjectSptr          action;
        ObjectUniqueSptr    unique;

        unique = OBJECT<ObjectUnique>();
        unique->add("read");
        unique->add("write");
        unique->add("action");

        action = OBJECT<Object>();
        action->setActionName("core.action.thread.watch.add");
        action->setActionParam("events",  unique);
        action->setActionParam("fd",      fd);

        object->emit(action);
    }

    return 0;
}

int32_t ObjectHttpServer::s_actionReaded(
    Object *a_object,
    Object *a_action)
{
    ObjectHttpServer    *object = NULL;
    int32_t             res, fd = -1;
    string              object_path;
    ObjectInt32Sptr     prop_fd;
    ObjectStringSptr    prop_data;

    object = dynamic_cast<ObjectHttpServer *>(a_object);
    if (!object){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectHttpServer\n"
        );
    }

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    object_path = object->getObjectPath();

    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        a_action->getActionParam("fd")
    );
    fd = prop_fd->getVal();

    prop_data = dynamic_pointer_cast<ObjectString>(
        a_action->getActionParam("data")
    );

    PDEBUG(100, "object: '%s', new read,"
        " fd: '%d',"
        " data: '%s'\n",
        object_path.c_str(),
        fd,
        prop_data->c_str()
    );

    if (!prop_data->empty()){
        ObjectMapIt             it;
        ObjectStringSptr        pending_data;
        ObjectHttpRequestSptr   request;

        it = object->m_pending_data->find(prop_fd);
        if (object->m_pending_data->end() == it){
            request = OBJECT<ObjectHttpRequest>();
            object->m_pending_data->add(prop_fd, request);
        } else {
            request = dynamic_pointer_cast<ObjectHttpRequest>(
                it->second
            );
        }

        res = request->parseRequest(
            prop_data->c_str(),
            prop_data->size()
        );
        if (!res){
            // all content read
        } else {
            object->save();
            goto out;
        }
    }

    {
        double          cur_time = 0.0f;
        struct timeval  tv;

        gettimeofday(&tv, NULL);
        cur_time = tv.tv_sec + tv.tv_usec / 1e6;
        PWARN("cur time: '%f'\n", cur_time);
    }

    if (0){
        ObjectSptr    action;

        action = OBJECT<Object>();
        action->setActionName("core.action.thread.write");
        action->setActionParam("data",    prop_data);
        action->setActionParam("fd",      fd);

        object->emit(action);
    }

    if (1){
        char                buffer[512] = { 0x00 };
        ObjectSptr          action;
        ObjectStringSptr    data;
        string              body;

        body += "<html>";
        body += "<head>";
        body += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"></meta>";
        body += "<title>Hi there!</title>";
        body += "</head>";
        body += "<body>";
        body += "<h1>Зверушкин зверь!</h1>";
        body += "</body>";
        body += "</html>";

        data = OBJECT<ObjectString>();
        data->add("HTTP/1.1 200 OK\r\n");
#ifdef __x86_64__
        snprintf(buffer, sizeof(buffer),
            "Content-Length: %ld\r\n",
            body.size()
        );
#else
        snprintf(buffer, sizeof(buffer),
            "Content-Length: %u\r\n",
            body.size()
        );
#endif
        data->add(buffer);
        data->add("\r\n");
        data->add(body);

        action = OBJECT<Object>();
        action->setActionName("core.action.thread.write");
        action->setActionParam("data",    data);
        action->setActionParam("fd",      fd);

        object->emit(action);
    }

    if (1){
        ObjectSptr  action;

        action = OBJECT<Object>();
        action->setActionName("core.action.thread.close");
        action->setActionParam("fd", fd);

        object->emit(action);
    }

out:
    return 0;
}

int32_t ObjectHttpServer::s_actionEvent(
    Object *a_object,
    Object *a_action)
{
    ObjectHttpServer    *object = NULL;
    int32_t             fd      = -1;
    string              object_path;
    ObjectInt32Sptr     prop_fd;
    ObjectUniqueSptr    prop_flags;

    object = dynamic_cast<ObjectHttpServer *>(a_object);
    if (!object){
        PFATAL("missing argument: 'a_object'"
            " or it's not ObjectHttpServer\n"
        );
    }

    object_path = object->getObjectPath();

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    prop_fd = dynamic_pointer_cast<ObjectInt32>(
        a_action->getActionParam("fd")
    );
    fd = prop_fd->getVal();

    prop_flags = dynamic_pointer_cast<ObjectUnique>(
        a_action->getActionParam("flags")
    );

    PDEBUG(100, "object: '%s', new IO event,"
        " fd: '%d', flags: '%s'\n",
        object_path.c_str(),
        fd,
        prop_flags->toString()->c_str()
    );

    if (prop_flags->has("eof")){
        object->m_pending_data->erase(prop_fd);
        object->save();
    }

    return 0;
}

// ---------------- module ----------------

string ObjectHttpServer::s_getType()
{
    return "core.object.http.server";
}

int32_t ObjectHttpServer::s_init(
    EngineInfo *a_engine_info)
{
    g_engine_info = a_engine_info;

    return 0;
}

int32_t ObjectHttpServer::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectHttpServer::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectHttpServer(a_create_flags));

    return object;
}

void ObjectHttpServer::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_http_server = {
    .getType        = ObjectHttpServer::s_getType,
    .init           = ObjectHttpServer::s_init,
    .shutdown       = ObjectHttpServer::s_shutdown,
    .objectCreate   = ObjectHttpServer::s_objectCreate,
    .getTests       = ObjectHttpServer::s_getTests
};

