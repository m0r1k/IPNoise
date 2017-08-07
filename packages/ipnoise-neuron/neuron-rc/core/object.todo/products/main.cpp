#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/products/main.hpp"

ObjectProducts::ObjectProducts(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectProducts::~ObjectProducts()
{
};

void ObjectProducts::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectProducts::getType()
{
    return ObjectProducts::_getType();
}

void ObjectProducts::do_init_props()
{
    Object::do_init_props();
}

void ObjectProducts::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectProducts::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectProducts::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectProducts::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectProducts::_getType()
{
    return "core.object.products";
}

string ObjectProducts::_getTypePerl()
{
    return "core::object::products::main";
}

int32_t ObjectProducts::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectProducts::_destroy()
{
}

ObjectSptr ObjectProducts::_object_create()
{
    ObjectSptr object;
    OBJECT_PRODUCTS(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectProducts::_getType,
    .type_perl      = ObjectProducts::_getTypePerl,
    .init           = ObjectProducts::_init,
    .destroy        = ObjectProducts::_destroy,
    .object_create  = ObjectProducts::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

