#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/double/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/time/main.hpp"

ObjectTime::ObjectTime(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectTime::ObjectTime(
    const Object::CreateFlags   &a_flags,
    const double                &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    PROP_DOUBLE(m_val, a_val);
    m_val->setDirty(1);
}

ObjectTime::~ObjectTime()
{
}
void ObjectTime::do_init(
    const Object::CreateFlags &a_flags)
{
    PROP_DOUBLE(m_val, 0.0f);
}

string ObjectTime::getType()
{
    return ObjectTime::_getType();
}

void ObjectTime::do_init_props()
{
    Object::do_init_props();
}

void ObjectTime::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectTime::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    a_builder.append(a_field_name, m_val->getVal());
}

void ObjectTime::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    a_builder.append(m_val->getVal());
}

SV * ObjectTime::toPerl()
{
    SV *ret = NULL;
    ret = newSVnv(m_val->getVal());
    return ret;
}

string ObjectTime::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%f",
        m_val->getVal()
    );
    return buffer;
}

/*
ObjectTimeSptr ObjectTime::now(
    const int32_t &a_is_prop)
{
    ObjectTimeSptr ret;
    PROP_TIME(ret, a_is_prop);
    ret->reset();
    return ret;
}
*/

void ObjectTime::reset()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    PROP_DOUBLE(
        m_val,
        tv.tv_sec + tv.tv_usec / 1e6
    );
    setDirty(1);
}

double ObjectTime::getVal()
{
    return m_val->getVal();
}

ObjectTime::operator double()
{
    return m_val->getVal();
}

ObjectTime::operator int32_t()
{
    return (int32_t)m_val->getVal();
}

ObjectTime::operator int64_t()
{
    return (int64_t)m_val->getVal();
}

bool ObjectTime::operator != (
    ObjectTime a_right)
{
    bool ret = false;
    ret = (m_val->getVal() != a_right.m_val->getVal());
    return ret;
}

// ---------------- module ----------------

string ObjectTime::_getType()
{
    return "core.object.time";
}

string ObjectTime::_getTypePerl()
{
    return "core::object::time::main";
}

int32_t ObjectTime::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectTime::_destroy()
{
}

ObjectSptr ObjectTime::_object_create()
{
    ObjectSptr object;
    _OBJECT_TIME(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectTime::_getType,
    .type_perl      = ObjectTime::_getTypePerl,
    .init           = ObjectTime::_init,
    .destroy        = ObjectTime::_destroy,
    .object_create  = ObjectTime::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

