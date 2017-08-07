#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/god/main.hpp"
#include "core/object/session/main.hpp"
#include "core/object/param/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/shop/main.hpp"

ObjectShop::ObjectShop(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectShop::~ObjectShop()
{
};

void ObjectShop::do_init(
    const Object::CreateFlags   &a_flags)
{
    setId("static.shop");
}

string ObjectShop::getType()
{
    return ObjectShop::_getType();
}

void ObjectShop::do_init_props()
{
    Object::do_init_props();

//    ObjectIdSptr    prop_id;
//    PROP_ID(   prop_id,   "static.shop");
//    INIT_PROP(this, Id,   prop_id);
}

void ObjectShop::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectShop::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectShop::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectShop::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectShop::_getType()
{
    return "core.object.shop";
}

string ObjectShop::_getTypePerl()
{
    return "core::object::shop::main";
}

int32_t ObjectShop::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectShop::_destroy()
{
}

ObjectSptr ObjectShop::_object_create()
{
    ObjectSptr object;
    OBJECT_SHOP(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectShop::_getType,
    .type_perl      = ObjectShop::_getTypePerl,
    .init           = ObjectShop::_init,
    .destroy        = ObjectShop::_destroy,
    .object_create  = ObjectShop::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

