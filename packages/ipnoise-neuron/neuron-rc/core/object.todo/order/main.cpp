#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/order/main.hpp"

ObjectOrder::ObjectOrder(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectOrder::~ObjectOrder()
{
};

void ObjectOrder::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectOrder::getType()
{
    return ObjectOrder::_getType();
}

void ObjectOrder::do_init_props()
{
    Object::do_init_props();
}

void ObjectOrder::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectOrder::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectOrder::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectOrder::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectOrder::_getType()
{
    return "core.object.order";
}

string ObjectOrder::_getTypePerl()
{
    return "core::object::order::main";
}

int32_t ObjectOrder::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectOrder::_destroy()
{
}

ObjectSptr ObjectOrder::_object_create()
{
    ObjectSptr object;
    OBJECT_ORDER(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectOrder::_getType,
    .type_perl      = ObjectOrder::_getTypePerl,
    .init           = ObjectOrder::_init,
    .destroy        = ObjectOrder::_destroy,
    .object_create  = ObjectOrder::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

