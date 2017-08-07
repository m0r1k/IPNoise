#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/map/main.hpp>

#include "main.hpp"

ObjectInotify::ObjectInotify(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectInotify::~ObjectInotify()
{
}

string ObjectInotify::getType()
{
    return ObjectInotify::s_getType();
}

ObjectSptr ObjectInotify::copy()
{
    ObjectInotifySptr   ret;

    ret = OBJECT<ObjectInotify>();

    return ret;
}

int32_t ObjectInotify::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    return err;
}

int32_t ObjectInotify::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_object(a_data, a_data_size);

    return err;
}

void ObjectInotify::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectInotify::s_getType()
{
    return "core.object.inotify";
}

int32_t ObjectInotify::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectInotify::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectInotify::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectInotify(a_create_flags));

    return object;
}

void ObjectInotify::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_inotify = {
    .getType        = ObjectInotify::s_getType,
    .init           = ObjectInotify::s_init,
    .shutdown       = ObjectInotify::s_shutdown,
    .objectCreate   = ObjectInotify::s_objectCreate,
    .getTests       = ObjectInotify::s_getTests
};

