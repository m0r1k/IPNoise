#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/ipnoise/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/shop/main.hpp"
#include "core/object/users/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/core/main.hpp"

ObjectCore::ObjectCore(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectCore::~ObjectCore()
{
};

void ObjectCore::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectCore::getType()
{
    return ObjectCore::_getType();
}

void ObjectCore::do_init_props()
{
    Object::do_init_props();
}

void ObjectCore::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectCore::do_autorun()
{
    int32_t             err = -1;
    ObjectUsersSptr     users;
    ObjectShopSptr      shop;
    ObjectIPNoiseSptr   ipnoise;

    users = getCreateNeigh<ObjectUsers>(
        "core.object.users"
    );
    if (!users){
        PWARN("cannot get/create object ObjectUsers\n");
        goto fail;
    }

    shop = getCreateNeigh<ObjectShop>(
        "core.object.shop"
    );
    if (!shop){
        PWARN("cannot get/create object ObjectShop\n");
        goto fail;
    }

    ipnoise = getCreateNeigh<ObjectIPNoise>(
        "core.object.ipnoise"
    );
    if (!ipnoise){
        PWARN("cannot get/create object ObjectIPNoise\n");
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

void ObjectCore::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectCore::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectCore::_getType()
{
    return "core.object.core";
}

string ObjectCore::_getTypePerl()
{
    return "core::object::core::main";
}

int32_t ObjectCore::_module_init()
{
    int32_t err = 0;
    return err;
}

void ObjectCore::_module_destroy()
{
}

ObjectSptr ObjectCore::_module_object_create()
{
    ObjectCoreSptr object;
    OBJECT_CORE(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectCore::_getType,
    .type_perl      = ObjectCore::_getTypePerl,
    .init           = ObjectCore::_module_init,
    .destroy        = ObjectCore::_module_destroy,
    .object_create  = ObjectCore::_module_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

