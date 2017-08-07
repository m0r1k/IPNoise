#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/bookmarks/main.hpp"

ObjectBookmarks::ObjectBookmarks(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectBookmarks::~ObjectBookmarks()
{
};

void ObjectBookmarks::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectBookmarks::getType()
{
    return ObjectBookmarks::_getType();
}

void ObjectBookmarks::do_init_props()
{
    Object::do_init_props();
}

void ObjectBookmarks::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectBookmarks::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectBookmarks::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectBookmarks::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = -1;

    err = Object::parseBSON(a_obj);
    if (err){
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// ---------------- module ----------------

string ObjectBookmarks::_getType()
{
    return "core.object.bookmarks";
}

string ObjectBookmarks::_getTypePerl()
{
    return "core::object::bookmarks::main";
}

int32_t ObjectBookmarks::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectBookmarks::_destroy()
{
}

ObjectSptr ObjectBookmarks::_object_create()
{
    ObjectSptr object;
    OBJECT_BOOKMARKS(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectBookmarks::_getType,
    .type_perl      = ObjectBookmarks::_getTypePerl,
    .init           = ObjectBookmarks::_init,
    .destroy        = ObjectBookmarks::_destroy,
    .object_create  = ObjectBookmarks::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

