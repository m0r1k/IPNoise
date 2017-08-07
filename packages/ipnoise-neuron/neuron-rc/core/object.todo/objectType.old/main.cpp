#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/objectType/main.hpp"

ObjectType::ObjectType(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectType::ObjectType(
    const Object::CreateFlags   &a_flags,
    const char                  *a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    PROP_STRING(m_val, a_val);
    setDirty(1);
}

ObjectType::ObjectType(
    const Object::CreateFlags   &a_flags,
    const string                &a_val)
    :   Object(a_flags)
{
    PROP_STRING(m_val, a_val);
    setDirty(1);
}

ObjectType::~ObjectType()
{
}

void ObjectType::do_init(
    const Object::CreateFlags   &a_flags)
{
    PROP_STRING(m_val);
}

string ObjectType::getType()
{
    return ObjectType::_getType();
}

void ObjectType::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.type");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectType::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectType::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void ObjectType::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

SV * ObjectType::toPerl()
{
    return m_val->toPerl();
}

string ObjectType::toString()
{
    return m_val->toString();
}

string ObjectType::serialize(
    const string &a_delim)
{
    return m_val->serialize(a_delim);
}

ObjectType::operator string()
{
    return m_val->toString();
}

/*
string ObjectType::serializeInfo(
    const string &a_delim)
{
//    char                buffer[512] = { 0x00 };
    string              ret, data;

    ObjectMapSptr         info;
    ObjectMap::iterator   info_it;

    PROP_MAP(info);

    Object::getObjectsInfo(info);

    ret = info->serialize(a_delim);

    for (info_it = info->begin();
        info_it != info->end();
        info_it++)
    {
        if (data.size()){
            data += ", ";
        }
        snprintf(buffer, sizeof(buffer),
            "{ 'type_str'%s '%s' }",
            a_delim.c_str(),
            info_it->first->toString()->c_str()
        );
        data += buffer;
    }

    ret += "[";
    if (data.size()){
        ret += " ";
    }
    ret += data;
    if (data.size()){
        ret += " ";
    }
    ret += "]";

    return ret;
}
*/

// ---------------- module ----------------

string ObjectType::_getType()
{
    return "core.object.type";
}

string ObjectType::_getTypePerl()
{
    return "core::object::objectType::main";
}

int32_t ObjectType::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectType::_destroy()
{
}

ObjectSptr ObjectType::_object_create()
{
    ObjectSptr object;
    OBJECT_TYPE(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectType::_getType,
    .type_perl      = ObjectType::_getTypePerl,
    .init           = ObjectType::_init,
    .destroy        = ObjectType::_destroy,
    .object_create  = ObjectType::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

