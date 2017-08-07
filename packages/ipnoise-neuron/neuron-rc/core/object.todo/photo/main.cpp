#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/photo/main.hpp"

ObjectPhoto::ObjectPhoto(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectPhoto::~ObjectPhoto()
{
};

void ObjectPhoto::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectPhoto::getType()
{
    return ObjectPhoto::_getType();
}

void ObjectPhoto::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.photo");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectPhoto::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectPhoto::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectPhoto::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectPhoto::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectPhoto::_getType()
{
    return "core.object.photo";
}

string ObjectPhoto::_getTypePerl()
{
    return "core::object::photo::main";
}

int32_t ObjectPhoto::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectPhoto::_destroy()
{
}

ObjectSptr ObjectPhoto::_object_create()
{
    ObjectSptr object;
    OBJECT_PHOTO(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectPhoto::_getType,
    .type_perl      = ObjectPhoto::_getTypePerl,
    .init           = ObjectPhoto::_init,
    .destroy        = ObjectPhoto::_destroy,
    .object_create  = ObjectPhoto::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

