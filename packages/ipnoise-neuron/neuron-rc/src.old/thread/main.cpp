#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

// libevent
#include <event2/event.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
//#include "core/object/core/main.hpp"
#include "core/object/user/main.hpp"
//#include "core/object/god/main.hpp"
#include "core/object/httpServer/main.hpp"
#include "core/object/vec3/main.hpp"
#include "core/object/object/__END__.hpp"
#include "db.hpp"

#include "thread/main.hpp"

ThreadMain::ThreadMain(
    const string    &a_name)
    :   Thread(a_name)
{
}

ThreadMain::~ThreadMain()
{
}

int32_t ThreadMain::start()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    // do not create new thread,
    // use current
    return Thread::run(this);
}

bool ThreadMain::joinable() const noexcept
{
    return false;
}

int32_t ThreadMain::loop_before()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t             err = -1;
    struct event        *signal_event1  = NULL;
    struct event        *signal_event2  = NULL;
    ObjectContextSptr   context         = contextGet();
    DbSptr              db              = getDb();

//    ObjectGodSptr   god;

    // setup context
//    god     = getCreateGodObject();
//    context = ContextSptr(new Context());
//    Thread::setCurContext(context);

    // call addNeigh method only after setup context,
    // because it will use user from context
//    god->do_autorun();

    // init signals
    signal_event1 = evsignal_new(
        getEvBase(),
        SIGINT,
        ThreadMain::shutdown_cb,
        this
    );
    if (    !signal_event1
        ||  0 > event_add(signal_event1, NULL))
    {
        PERROR("Could not create/add a signal event!\n");
        goto fail;
    }

    signal_event2 = evsignal_new(
        getEvBase(),
        SIGTERM,
        ThreadMain::shutdown_cb,
        this
    );

    if (    !signal_event2
        ||  0 > event_add(signal_event2, NULL))
    {
        PERROR("Could not create/add a signal event!\n");
        goto fail;
    }

    // clean db
    db->do_clean();

    // get/create objects
    //getCreateCoreObject();
    getCreateHttpServerObject();

    context->transactionCommit();
    Thread::resetCurContext();

    // all ok
    err = 0;

out:
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    if (signal_event1){
        event_free(signal_event1);
        signal_event1 = NULL;
    }
    if (signal_event2){
        event_free(signal_event2);
        signal_event2 = NULL;
    }
    goto out;
}

void ThreadMain::loop_tick()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    Thread::removeShutdownedThreads();
}

int32_t ThreadMain::loop_after()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t err = 0;

    Thread::stopAllThreads();

    closeKladrDb();

    return err;
}

/*
// ---- createPoints ----

void ThreadMain::createPoints()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    char    buffer[512] = { 0x00 };
    int32_t size        = 3;
    double  s           = (size - 1)/2;
    int32_t x, y, z;

    ObjectCoreSptr object = getCreateCoreObject();

    for (z = 0; z < size; z++)
    for (y = 0; y < size; y++)
    for (x = 0; x < size; x++)
    {
        ObjectSptr      new_object;
        ObjectVec3Sptr  new_pos;

        snprintf(buffer, sizeof(buffer),
            "%d:%d:%d",
            x, y, z
        );

        new_object = object->getCreateNeigh(
            "core.object.media",    // type
            buffer                  // name
        );

        OBJECT_VEC3(new_pos,
            (x-s)*ELECTRON_RADIUS,
            (y-s)*ELECTRON_RADIUS,
            (z-s)*ELECTRON_RADIUS
        );
        new_object->setPos(new_pos);

        new_object->save();
    }

    object->save();
}

// ---- CoreObject ----

ObjectCoreSptr ThreadMain::getCoreObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectCoreSptr      object;
    ObjectContextSptr   context = getCurContext();
    object = context->getObjectById<ObjectCore>(
        "static.core"
    );
    return object;
}

ObjectCoreSptr ThreadMain::getCreateCoreObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectCoreSptr      object;
    ObjectContextSptr   context = getCurContext();

    object = getCoreObject();
    if (!object){
        // create new
        object = Object::createObject<ObjectCore>(
            "core.object.core", // type
            1,                  // do_autorun
            "static.core"       // ID
        );
        object->save();
    }

    return object;
}

*/

// ---- httpServer ----

ObjectHttpServerSptr ThreadMain::getHttpServerObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectHttpServerSptr    object;
    ObjectContextSptr       context = getCurContext();
    object = context->getObjectById<ObjectHttpServer>(
        "static.http.server"
    );
    return object;
}

ObjectHttpServerSptr ThreadMain::getCreateHttpServerObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectHttpServerSptr    object;
    ObjectActionSptr        action;
    ObjectContextSptr       context = contextGet();

    // create new server
    object = getHttpServerObject();
    if (!object){
        // create new
        object = Object::createObject<ObjectHttpServer>(
            "core.object.http.server",  // type
            1,                          // do_autorun
            "static.http.server"        // ID
        );
        //object->save();
        context->transactionAddForSave(object);
    }

    OBJECT_ACTION(action, "thread.io.listen");

    Object::addNeigh(action, object);
    action->schedule();
    context->transactionAddForSave(action);

    return object;
}

ObjectGodSptr ThreadMain::getGodObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectGodSptr  object;

    PFATAL("FIXME\n");
/*

    ObjectContextSptr   context = getCurContext();

    object = context->getObjectById<ObjectGod>(
        OBJECT_ID("core.users.god"),
        ObjectTypeSptr(),
        0,  // a_get_removed
        0   // a_do_autorun
    );
*/
    return object;
}

ObjectGodSptr ThreadMain::getCreateGodObject()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectGodSptr object;

    PFATAL("FIXME\n");
/*
    object = _getGodObject();
    if (!object){
        // create new (without autorun);
        object = ThreadDb::get()->createObject<ObjectGod>("", 0);
        object->save();
    }
*/
    return object;
}

// ---------------- static ----------------

void ThreadMain::shutdown_cb(
    evutil_socket_t,
    short,
    void *a_ctx)
{
    ThreadMain *thread = (ThreadMain *)a_ctx;
    thread->stop();
}

/*
ThreadMainSptr ThreadMain::get()
{
    ThreadSptr     tmp    = getThreadByName("main");
    ThreadMainSptr thread = dynamic_pointer_cast<ThreadMain>(
        tmp
    );
    return thread;
}
*/

