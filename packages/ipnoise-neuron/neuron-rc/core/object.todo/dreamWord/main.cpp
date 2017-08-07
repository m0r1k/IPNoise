#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/dreamWord/main.hpp"

ObjectDreamWord::ObjectDreamWord(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectDreamWord::~ObjectDreamWord()
{
};

void ObjectDreamWord::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectDreamWord::getType()
{
    return ObjectDreamWord::_getType();
}

void ObjectDreamWord::do_init_props()
{
    Object::do_init_props();
}

void ObjectDreamWord::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectDreamWord::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectDreamWord::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectDreamWord::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectDreamWord::_getType()
{
    return "core.object.dream.word";
}

string ObjectDreamWord::_getTypePerl()
{
    return "core::object::dreamWord::main";
}

int32_t ObjectDreamWord::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDreamWord::_destroy()
{
}

ObjectSptr ObjectDreamWord::_object_create()
{
    ObjectSptr object;
    OBJECT_DREAM_WORD(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDreamWord::_getType,
    .type_perl      = ObjectDreamWord::_getTypePerl,
    .init           = ObjectDreamWord::_init,
    .destroy        = ObjectDreamWord::_destroy,
    .object_create  = ObjectDreamWord::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

