#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/trash/main.hpp"

ObjectTrash::ObjectTrash(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectTrash::~ObjectTrash()
{
};

void ObjectTrash::do_init(
    const Object::CreateFlags   &a_flags)
{
    setId("static.trash");
}

string ObjectTrash::getType()
{
    return ObjectTrash::_getType();
}

void ObjectTrash::do_init_props()
{
    Object::do_init_props();

//    ObjectIdSptr    prop_id;
//    PROP_ID(   prop_id,   "static.trash");
//    INIT_PROP(this, Id,   prop_id);
}

void ObjectTrash::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectTrash::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectTrash::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectTrash::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectTrash::_getType()
{
    return "core.object.trash";
}

string ObjectTrash::_getTypePerl()
{
    return "core::object::trash::main";
}

int32_t ObjectTrash::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectTrash::_destroy()
{
}

ObjectSptr ObjectTrash::_object_create()
{
    ObjectSptr object;
    OBJECT_TRASH(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectTrash::_getType,
    .type_perl      = ObjectTrash::_getTypePerl,
    .init           = ObjectTrash::_init,
    .destroy        = ObjectTrash::_destroy,
    .object_create  = ObjectTrash::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

