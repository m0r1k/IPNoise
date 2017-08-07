#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/uint32/main.hpp"

ObjectUint32::ObjectUint32(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectUint32::ObjectUint32(
    const Object::CreateFlags   &a_flags,
    const uint32_t              &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val;
    setDirty(1);
}

ObjectUint32::~ObjectUint32()
{
}

void ObjectUint32::do_init(
    const Object::CreateFlags &a_flags)
{
}

string ObjectUint32::getType()
{
    return ObjectUint32::_getType();
}

void ObjectUint32::do_init_props()
{
    Object::do_init_props();
}

void ObjectUint32::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectUint32::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    a_builder.append(a_field_name, m_val);
}

void ObjectUint32::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    a_builder.append(m_val);
}

SV * ObjectUint32::toPerl()
{
    SV *ret = NULL;
    ret = newSViv(m_val);
    return ret;
}

string ObjectUint32::serialize(
    const string &a_delim)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%d",
        m_val
    );
    return buffer;
}

uint32_t ObjectUint32::getVal()
{
    return m_val;
}

ObjectUint32::operator uint32_t()
{
    return m_val;
}

// ---------------- module ----------------

string ObjectUint32::_getType()
{
    return "core.object.uint32";
}

string ObjectUint32::_getTypePerl()
{
    return "core::object::uint32::main";
}

int32_t ObjectUint32::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectUint32::_destroy()
{
}

ObjectSptr ObjectUint32::_object_create()
{
    ObjectSptr object;
    _OBJECT_UINT32(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectUint32::_getType,
    .type_perl      = ObjectUint32::_getTypePerl,
    .init           = ObjectUint32::_init,
    .destroy        = ObjectUint32::_destroy,
    .object_create  = ObjectUint32::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

