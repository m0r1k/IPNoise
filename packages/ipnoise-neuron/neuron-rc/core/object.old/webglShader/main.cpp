#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/webglShader/main.hpp"

ObjectWebglShader::ObjectWebglShader(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectWebglShader::~ObjectWebglShader()
{
};

void ObjectWebglShader::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectWebglShader::getType()
{
    return ObjectWebglShader::_getType();
}

void ObjectWebglShader::do_init_props()
{
    Object::do_init_props();
}

void ObjectWebglShader::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectWebglShader::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShader::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectWebglShader::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectWebglShader::_getType()
{
    return "core.object.webgl.shader";
}

string ObjectWebglShader::_getTypePerl()
{
    return "core::object::webglShader::main";
}

int32_t ObjectWebglShader::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShader::_destroy()
{
}

ObjectSptr ObjectWebglShader::_object_create()
{
    ObjectSptr object;
    _OBJECT_WEBGL_SHADER(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectWebglShader::_getType,
    .type_perl      = ObjectWebglShader::_getTypePerl,
    .init           = ObjectWebglShader::_init,
    .destroy        = ObjectWebglShader::_destroy,
    .object_create  = ObjectWebglShader::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

