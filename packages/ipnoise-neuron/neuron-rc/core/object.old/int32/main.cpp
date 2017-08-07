#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/int32/main.hpp"

ObjectInt32::ObjectInt32(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectInt32::ObjectInt32(
    const Object::CreateFlags   &a_flags,
    const int32_t               &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val;
    setDirty(1);
}

void ObjectInt32::do_init(
    const Object::CreateFlags &a_flags)
{
    m_val = 0;
}

string ObjectInt32::getType()
{
    return ObjectInt32::_getType();
}

void ObjectInt32::do_init_props()
{
    Object::do_init_props();
}

void ObjectInt32::do_init_api()
{
    Object::do_init_api();

    // register actions
}

ObjectInt32::~ObjectInt32()
{
}

void ObjectInt32::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);
    a_builder.append(a_field_name, m_val);
}

void ObjectInt32::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);
    a_builder.append(m_val);
}

SV * ObjectInt32::toPerl()
{
    SV *ret = NULL;
    ret = newSViv(m_val);
    return ret;
}

string ObjectInt32::serialize(
    const string &a_prefix)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    char buffer[1024] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%d", m_val
    );
    return buffer;
}

int32_t ObjectInt32::getVal()
{
    lock_guard<recursive_mutex> guard(m_mutex);
    return m_val;
}

ObjectInt32::operator int()
{
    lock_guard<recursive_mutex> guard(m_mutex);
    return m_val;
}

// ---------------- module ----------------

string ObjectInt32::_getType()
{
    return "core.object.int32";
}

string ObjectInt32::_getTypePerl()
{
    return "core::object::int32::main";
}

int32_t ObjectInt32::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectInt32::_destroy()
{
}

ObjectSptr ObjectInt32::_object_create()
{
    ObjectSptr object;
    _OBJECT_INT32(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectInt32::_getType,
    .type_perl      = ObjectInt32::_getTypePerl,
    .init           = ObjectInt32::_init,
    .destroy        = ObjectInt32::_destroy,
    .object_create  = ObjectInt32::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

