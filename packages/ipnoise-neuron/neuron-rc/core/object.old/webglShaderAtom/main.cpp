#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/webglShader/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/webglShaderAtom/main.hpp"

ObjectWebglShaderAtom::ObjectWebglShaderAtom(
    const Object::CreateFlags   &a_flags)
    :   ObjectWebglShader(a_flags)
{
    do_init(a_flags);
};

ObjectWebglShaderAtom::~ObjectWebglShaderAtom()
{
};

void ObjectWebglShaderAtom::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectWebglShaderAtom::getType()
{
    return ObjectWebglShaderAtom::_getType();
}

void ObjectWebglShaderAtom::do_init_props()
{
    ObjectWebglShader::do_init_props();
}

void ObjectWebglShaderAtom::do_init_api()
{
    ObjectWebglShader::do_init_api();

    // register actions
}

int32_t ObjectWebglShaderAtom::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShaderAtom::getAllProps(
    ObjectMapSptr a_props)
{
    ObjectWebglShader::getAllProps(a_props);
}

int32_t ObjectWebglShaderAtom::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = ObjectWebglShader::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectWebglShaderAtom::_getType()
{
    return "core.object.webgl.shader.atom";
}

string ObjectWebglShaderAtom::_getTypePerl()
{
    return "core::object::webglShaderAtom::main";
}

int32_t ObjectWebglShaderAtom::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglShaderAtom::_destroy()
{
}

ObjectSptr ObjectWebglShaderAtom::_object_create()
{
    ObjectSptr object;
    _OBJECT_WEBGL_SHADER_ATOM(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectWebglShaderAtom::_getType,
    .type_perl      = ObjectWebglShaderAtom::_getTypePerl,
    .init           = ObjectWebglShaderAtom::_init,
    .destroy        = ObjectWebglShaderAtom::_destroy,
    .object_create  = ObjectWebglShaderAtom::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

