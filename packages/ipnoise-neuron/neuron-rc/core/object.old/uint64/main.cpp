#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/uint64/main.hpp"

ObjectUint64::ObjectUint64(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectUint64::ObjectUint64(
    const Object::CreateFlags   &a_flags,
    const uint64_t              &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val;
    setDirty(1);
}

ObjectUint64::~ObjectUint64()
{
}

void ObjectUint64::do_init(
    const Object::CreateFlags &a_flags)
{
    m_val = 0;
}

string ObjectUint64::getType()
{
    return ObjectUint64::_getType();
}

void ObjectUint64::do_init_props()
{
    Object::do_init_props();
}

void ObjectUint64::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectUint64::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    // TODO XXX FIXME
    // fucking mongo don't known about unsigned long long
    // 20141013 morik@
    a_builder.append(a_field_name, (long long)m_val);
}

void ObjectUint64::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    // TODO XXX FIXME
    // fucking mongo don't known about unsigned long long
    // 20141013 morik@
    a_builder.append((long long)m_val);
}

SV * ObjectUint64::toPerl()
{
    SV *ret = NULL;
    ret = newSViv(m_val);
    return ret;
}

string ObjectUint64::serialize(
    const string &)
{
    char buffer[512] = { 0x00 };

    snprintf(buffer, sizeof(buffer),
        "%lld",
        m_val
    );

    return buffer;
}

uint64_t ObjectUint64::getVal()
{
    return m_val;
}

ObjectUint64::operator uint64_t()
{
    return m_val;
}

// ---------------- module ----------------

string ObjectUint64::_getType()
{
    return "core.object.uint64";
}

string ObjectUint64::_getTypePerl()
{
    return "core::object::uint64::main";
}

int32_t ObjectUint64::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectUint64::_destroy()
{
}

ObjectSptr ObjectUint64::_object_create()
{
    ObjectSptr object;
    _OBJECT_UINT64(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectUint64::_getType,
    .type_perl      = ObjectUint64::_getTypePerl,
    .init           = ObjectUint64::_init,
    .destroy        = ObjectUint64::_destroy,
    .object_create  = ObjectUint64::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

