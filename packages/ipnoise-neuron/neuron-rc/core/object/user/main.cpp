#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/vector/main.hpp>

#include "main.hpp"

ObjectUser::ObjectUser(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectUser::~ObjectUser()
{
}

string ObjectUser::getType()
{
    return ObjectUser::s_getType();
}

int32_t ObjectUser::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    return err;
}

int32_t ObjectUser::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_object(a_data, a_data_size);

    return err;
}

ObjectSptr ObjectUser::copy()
{
    ObjectUserSptr  ret;

    ret = OBJECT<ObjectUser>();

    return ret;
}

void ObjectUser::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectUser::s_getType()
{
    return "core.object.user";
}

int32_t ObjectUser::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;

    return 0;
}

int32_t ObjectUser::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectUser::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectUser(a_create_flags));

    return object;
}

void ObjectUser::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_user = {
    .getType        = ObjectUser::s_getType,
    .init           = ObjectUser::s_init,
    .shutdown       = ObjectUser::s_shutdown,
    .objectCreate   = ObjectUser::s_objectCreate,
    .getTests       = ObjectUser::s_getTests
};

