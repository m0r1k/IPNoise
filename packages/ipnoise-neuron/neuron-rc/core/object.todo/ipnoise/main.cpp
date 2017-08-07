#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/ipnoiseLinks/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/ipnoise/main.hpp"

ObjectIPNoise::ObjectIPNoise(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectIPNoise::~ObjectIPNoise()
{
};

void ObjectIPNoise::do_init(
    const Object::CreateFlags   &a_flags)
{
    setId("static.ipnoise");
}

string ObjectIPNoise::getType()
{
    return ObjectIPNoise::_getType();
}

void ObjectIPNoise::do_init_props()
{
    Object::do_init_props();

//    ObjectIdSptr    prop_id;
//    PROP_ID(   prop_id,   "static.ipnoise");
//    INIT_PROP(this, Id,   prop_id);
}

void ObjectIPNoise::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectIPNoise::do_autorun()
{
    int32_t err = 0;

    // init neighs if need
    getCreateNeigh<ObjectIPNoiseLinks>(
        "core.ipnoise.links"
    );

    return err;
}

void ObjectIPNoise::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectIPNoise::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectIPNoise::_getType()
{
    return "core.object.ipnoise";
}

string ObjectIPNoise::_getTypePerl()
{
    return "core::object::ipnoise::main";
}

int32_t ObjectIPNoise::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectIPNoise::_destroy()
{
}

ObjectSptr ObjectIPNoise::_object_create()
{
    ObjectSptr object;
    OBJECT_IPNOISE(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectIPNoise::_getType,
    .type_perl      = ObjectIPNoise::_getTypePerl,
    .init           = ObjectIPNoise::_init,
    .destroy        = ObjectIPNoise::_destroy,
    .object_create  = ObjectIPNoise::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

