#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/photos/main.hpp"

ObjectPhotos::ObjectPhotos(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectPhotos::~ObjectPhotos()
{
};

void ObjectPhotos::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectPhotos::getType()
{
    return ObjectPhotos::_getType();
}

void ObjectPhotos::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.photos");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectPhotos::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectPhotos::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectPhotos::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectPhotos::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectPhotos::_getType()
{
    return "core.object.photos";
}

string ObjectPhotos::_getTypePerl()
{
    return "core::object::photos::main";
}

int32_t ObjectPhotos::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectPhotos::_destroy()
{
}

ObjectSptr ObjectPhotos::_object_create()
{
    ObjectSptr object;
    OBJECT_PHOTOS(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectPhotos::_getType,
    .type_perl      = ObjectPhotos::_getTypePerl,
    .init           = ObjectPhotos::_init,
    .destroy        = ObjectPhotos::_destroy,
    .object_create  = ObjectPhotos::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

