#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/webglViewer/main.hpp"

ObjectWebglViewer::ObjectWebglViewer(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectWebglViewer::~ObjectWebglViewer()
{
};

void ObjectWebglViewer::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectWebglViewer::getType()
{
    return ObjectWebglViewer::_getType();
}

void ObjectWebglViewer::do_init_props()
{
    Object::do_init_props();
}

void ObjectWebglViewer::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectWebglViewer::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglViewer::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectWebglViewer::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectWebglViewer::_getType()
{
    return "core.object.webgl.viewer";
}

string ObjectWebglViewer::_getTypePerl()
{
    return "core::object::webglViewer::main";
}

int32_t ObjectWebglViewer::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectWebglViewer::_destroy()
{
}

ObjectSptr ObjectWebglViewer::_object_create()
{
    ObjectSptr object;
    _OBJECT_WEBGL_VIEWER(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectWebglViewer::_getType,
    .type_perl      = ObjectWebglViewer::_getTypePerl,
    .init           = ObjectWebglViewer::_init,
    .destroy        = ObjectWebglViewer::_destroy,
    .object_create  = ObjectWebglViewer::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

