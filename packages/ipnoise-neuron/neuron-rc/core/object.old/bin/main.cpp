#include <math.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/bin/main.hpp"

ObjectBin::ObjectBin(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectBin::ObjectBin(
    const Object::CreateFlags   &a_flags,
    const string                &a_val)
    :   Object(a_flags)
{
    do_init(a_flags, a_val);
    setDirty(1);
}

ObjectBin::~ObjectBin()
{
}

void ObjectBin::do_init(
    const Object::CreateFlags   &a_flags,
    const string                &a_val)
{
    PROP_STRING(m_val, a_val);
}

string ObjectBin::getType()
{
    return ObjectBin::_getType();
}

void ObjectBin::do_init_props()
{
    Object::do_init_props();
}

void ObjectBin::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectBin::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    a_builder.appendBinData(
        a_field_name,
        m_val->size(),
        mongo::BinDataGeneral,
        m_val->c_str()
    );
}

void ObjectBin::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    // TODO XXX FIXME
    PFATAL("FIXME\n");
    // a_builder.append(m_val);
}

SV * ObjectBin::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->toPerl();
}

ObjectStringSptr ObjectBin::getVal()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val;
}

string ObjectBin::toString()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->toString();
}

string ObjectBin::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->serialize(a_delim);
}

bool ObjectBin::empty()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->empty();
}

uint64_t ObjectBin::size()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->size();
}

const char * ObjectBin::c_str() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->c_str();
}

char ObjectBin::at(
    const int32_t   &a_pos)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->at(a_pos);
}

void ObjectBin::add(
    const char &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val->add(a_val);
    setDirty(1);
}

void ObjectBin::add(
    const char      *a_buff,
    const uint64_t  &a_len)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val->add(a_buff, a_len);
    setDirty(1);
}

void ObjectBin::add(
    ObjectStringSptr    a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val->add(a_val);
    setDirty(1);
}

string ObjectBin::hexdump()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->hexdump();
}

// ---------------- module ----------------

string ObjectBin::_getType()
{
    return "core.object.bin";
}

string ObjectBin::_getTypePerl()
{
    return "core::object::bin::main";
}

int32_t ObjectBin::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectBin::_destroy()
{
}

ObjectSptr ObjectBin::_object_create()
{
    ObjectSptr object;
    _OBJECT_BIN(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectBin::_getType,
    .type_perl      = ObjectBin::_getTypePerl,
    .init           = ObjectBin::_init,
    .destroy        = ObjectBin::_destroy,
    .object_create  = ObjectBin::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

