#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/objectId/main.hpp"

ObjectId::ObjectId(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectId::ObjectId(
    const Object::CreateFlags   &a_flags,
    const string                &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    PROP_STRING(m_val, a_val);
    setDirty(1);
}

ObjectId::~ObjectId()
{
}

void ObjectId::do_init(
    const Object::CreateFlags   &a_flags)
{
    PROP_STRING(m_val);
}

string ObjectId::getType()
{
    return ObjectId::_getType();
}

void ObjectId::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.id");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectId::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectId::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void ObjectId::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

SV * ObjectId::toPerl()
{
    return m_val->toPerl();
}

string ObjectId::toString()
{
    return m_val->toString();
}

string ObjectId::serialize(
    const string &a_delim)
{
    return m_val->serialize(a_delim);
}

// ---------------- module ----------------

string ObjectId::_getType()
{
    return "core.object.id";
}

string ObjectId::_getTypePerl()
{
    return "core::object::objectId::main";
}

int32_t ObjectId::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectId::_destroy()
{
}

ObjectSptr ObjectId::_object_create()
{
    ObjectSptr object;
    OBJECT_ID(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectId::_getType,
    .type_perl      = ObjectId::_getTypePerl,
    .init           = ObjectId::_init,
    .destroy        = ObjectId::_destroy,
    .object_create  = ObjectId::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

