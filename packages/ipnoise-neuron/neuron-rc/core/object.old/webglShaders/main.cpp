#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/webglShaders/main.hpp"

ObjectWebglShaders::ObjectWebglShaders(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectWebglShaders::~ObjectWebglShaders()
{
};

void ObjectWebglShaders::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectWebglShaders::getType()
{
    return ObjectWebglShaders::_getType();
}

void ObjectWebglShaders::do_init_props()
{
    Object::do_init_props();
}

void ObjectWebglShaders::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectWebglShaders::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShaders::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectWebglShaders::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectWebglShaders::_getType()
{
    return "core.object.webgl.shaders";
}

string ObjectWebglShaders::_getTypePerl()
{
    return "core::object::webglShaders::main";
}

int32_t ObjectWebglShaders::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShaders::_destroy()
{
}

ObjectSptr ObjectWebglShaders::_object_create()
{
    ObjectSptr object;
    _OBJECT_WEBGL_SHADERS(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectWebglShaders::_getType,
    .type_perl      = ObjectWebglShaders::_getTypePerl,
    .init           = ObjectWebglShaders::_init,
    .destroy        = ObjectWebglShaders::_destroy,
    .object_create  = ObjectWebglShaders::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

