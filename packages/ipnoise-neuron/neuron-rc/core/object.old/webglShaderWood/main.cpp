#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/webglShader/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/webglShaderWood/main.hpp"

ObjectWebglShaderWood::ObjectWebglShaderWood(
    const Object::CreateFlags   &a_flags)
    :   ObjectWebglShader(a_flags)
{
    do_init(a_flags);
};

ObjectWebglShaderWood::~ObjectWebglShaderWood()
{
};

void ObjectWebglShaderWood::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectWebglShaderWood::getType()
{
    return ObjectWebglShaderWood::_getType();
}

void ObjectWebglShaderWood::do_init_props()
{
    ObjectWebglShader::do_init_props();
}

void ObjectWebglShaderWood::do_init_api()
{
    ObjectWebglShader::do_init_props();

    // register actions
}

int32_t ObjectWebglShaderWood::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShaderWood::getAllProps(
    ObjectMapSptr a_props)
{
    ObjectWebglShader::getAllProps(a_props);
}

int32_t ObjectWebglShaderWood::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = ObjectWebglShader::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectWebglShaderWood::_getType()
{
    return "core.object.webgl.shader.wood";
}

string ObjectWebglShaderWood::_getTypePerl()
{
    return "core::object::webglShaderWood::main";
}

int32_t ObjectWebglShaderWood::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShaderWood::_destroy()
{
}

ObjectSptr ObjectWebglShaderWood::_object_create()
{
    ObjectSptr object;
    _OBJECT_WEBGL_SHADER_WOOD(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectWebglShaderWood::_getType,
    .type_perl      = ObjectWebglShaderWood::_getTypePerl,
    .init           = ObjectWebglShaderWood::_init,
    .destroy        = ObjectWebglShaderWood::_destroy,
    .object_create  = ObjectWebglShaderWood::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

