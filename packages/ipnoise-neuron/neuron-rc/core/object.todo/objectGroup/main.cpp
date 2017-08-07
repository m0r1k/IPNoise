#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/objectGroup/main.hpp"

ObjectGroup::ObjectGroup(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectGroup::ObjectGroup(
    const Object::CreateFlags   &a_flags,
    ObjectGroup                 a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val.m_val;
    setDirty(1);
}

ObjectGroup::ObjectGroup(
    const Object::CreateFlags   &a_flags,
    const string                &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);

    PROP_STRING(m_val, a_val);
    m_val->toLower();

    setDirty(1);
}

ObjectGroup::~ObjectGroup()
{
}

void ObjectGroup::do_init(
    const Object::CreateFlags   &a_flags)
{
    PROP_STRING(m_val);
}

string ObjectGroup::getType()
{
    return ObjectGroup::_getType();
}

void ObjectGroup::do_init_props()
{
    Object::do_init_props();
}

void ObjectGroup::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectGroup::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void ObjectGroup::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

SV * ObjectGroup::toPerl()
{
    return m_val->toPerl();
}

string ObjectGroup::serialize(
    const string &a_delim)
{
    return m_val->serialize(a_delim);
}

// ---------------- module ----------------

string ObjectGroup::_getType()
{
    return "core.object.group";
}

string ObjectGroup::_getTypePerl()
{
    return "core::object::objectGroup::main";
}

int32_t ObjectGroup::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectGroup::_destroy()
{
}

ObjectSptr ObjectGroup::_object_create()
{
    ObjectSptr object;
    OBJECT_GROUP(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectGroup::_getType,
    .type_perl      = ObjectGroup::_getTypePerl,
    .init           = ObjectGroup::_init,
    .destroy        = ObjectGroup::_destroy,
    .object_create  = ObjectGroup::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

