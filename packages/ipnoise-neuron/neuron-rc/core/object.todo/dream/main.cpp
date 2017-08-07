#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/dream/main.hpp"

ObjectDream::ObjectDream(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectDream::~ObjectDream()
{
};

void ObjectDream::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectDream::getType()
{
    return ObjectDream::_getType();
}

void ObjectDream::do_init_props()
{
    Object::do_init_props();
}

void ObjectDream::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectDream::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectDream::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectDream::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectDream::_getType()
{
    return "core.object.dream";
}

string ObjectDream::_getTypePerl()
{
    return "core::object::dream::main";
}

int32_t ObjectDream::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDream::_destroy()
{
}

ObjectSptr ObjectDream::_object_create()
{
    ObjectSptr object;
    OBJECT_DREAM(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDream::_getType,
    .type_perl      = ObjectDream::_getTypePerl,
    .init           = ObjectDream::_init,
    .destroy        = ObjectDream::_destroy,
    .object_create  = ObjectDream::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

