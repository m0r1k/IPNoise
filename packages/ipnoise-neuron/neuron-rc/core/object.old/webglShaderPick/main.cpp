#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/webglShader/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/webglShaderPick/main.hpp"

ObjectWebglShaderPick::ObjectWebglShaderPick(
    const Object::CreateFlags   &a_flags)
    :   ObjectWebglShader(a_flags)
{
    do_init(a_flags);
};

ObjectWebglShaderPick::~ObjectWebglShaderPick()
{
};

void ObjectWebglShaderPick::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectWebglShaderPick::getType()
{
    return ObjectWebglShaderPick::_getType();
}

void ObjectWebglShaderPick::do_init_props()
{
    ObjectWebglShader::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.webgl.shader.pick");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectWebglShaderPick::do_init_api()
{
    ObjectWebglShader::do_init_api();

    // register actions
}

int32_t ObjectWebglShaderPick::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShaderPick::getAllProps(
    ObjectMapSptr a_props)
{
    ObjectWebglShader::getAllProps(a_props);
}

int32_t ObjectWebglShaderPick::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = ObjectWebglShader::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectWebglShaderPick::_getType()
{
    return "core.object.webgl.shader.pick";
}

string ObjectWebglShaderPick::_getTypePerl()
{
    return "core::object::webglShaderPick::main";
}

int32_t ObjectWebglShaderPick::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShaderPick::_destroy()
{
}

ObjectSptr ObjectWebglShaderPick::_object_create()
{
    ObjectSptr object;
    _OBJECT_WEBGL_SHADER_PICK(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectWebglShaderPick::_getType,
    .type_perl      = ObjectWebglShaderPick::_getTypePerl,
    .init           = ObjectWebglShaderPick::_init,
    .destroy        = ObjectWebglShaderPick::_destroy,
    .object_create  = ObjectWebglShaderPick::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

