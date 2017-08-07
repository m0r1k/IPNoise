#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/int64/main.hpp"

ObjectInt64::ObjectInt64(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectInt64::ObjectInt64(
    const Object::CreateFlags   &a_flags,
    const int64_t               &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val;
    setDirty(1);
}

ObjectInt64::~ObjectInt64()
{
}

void ObjectInt64::do_init(
    const Object::CreateFlags &a_flags)
{
    m_val = 0;
}

string ObjectInt64::getType()
{
    return ObjectInt64::_getType();
}

void ObjectInt64::do_init_props()
{
    Object::do_init_props();
}

void ObjectInt64::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectInt64::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    a_builder.append(a_field_name, m_val);
}

void ObjectInt64::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    a_builder.append(m_val);
}

SV * ObjectInt64::toPerl()
{
    SV *ret = NULL;
    ret = newSViv(m_val);
    return ret;
}

string ObjectInt64::serialize(
    const string &)
{
    char buffer[512] = { 0x00 };

    snprintf(buffer, sizeof(buffer),
        "%lld",
        m_val
    );

    return buffer;
}

int64_t ObjectInt64::getVal()
{
    return m_val;
}

ObjectInt64::operator int64_t()
{
    return m_val;
}

// ---------------- module ----------------

string ObjectInt64::_getType()
{
    return "core.object.int64";
}

string ObjectInt64::_getTypePerl()
{
    return "core::object::int64::main";
}

int32_t ObjectInt64::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectInt64::_destroy()
{
}

ObjectSptr ObjectInt64::_object_create()
{
    ObjectSptr object;
    _OBJECT_INT64(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectInt64::_getType,
    .type_perl      = ObjectInt64::_getTypePerl,
    .init           = ObjectInt64::_init,
    .destroy        = ObjectInt64::_destroy,
    .object_create  = ObjectInt64::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

